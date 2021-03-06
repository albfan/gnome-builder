/* gbp-flatpak-pipeline-addin.c
 *
 * Copyright © 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "gbp-flatpak-pipeline-addin"

#include <glib/gi18n.h>

#include "gbp-flatpak-configuration.h"
#include "gbp-flatpak-download-stage.h"
#include "gbp-flatpak-pipeline-addin.h"
#include "gbp-flatpak-runtime.h"
#include "gbp-flatpak-transfer.h"
#include "gbp-flatpak-util.h"

#define VERSION_CHECK(v,a,b,c)                                         \
  (((v)->major > (a)) ||                                               \
   (((v)->major == (a)) && ((v)->minor > (b))) ||                      \
   (((v)->major == (a)) && ((v)->minor == (b)) && ((v)->micro >= (c))))

struct _GbpFlatpakPipelineAddin
{
  IdeObject parent_instance;

  gchar *state_dir;

  struct {
    int major;
    int minor;
    int micro;
  } version;
};

G_DEFINE_QUARK (gb-flatpak-pipeline-error-quark, gb_flatpak_pipeline_error)

enum {
  PREPARE_MKDIRS,
  PREPARE_BUILD_INIT,
};

enum {
  EXPORT_BUILD_FINISH,
  EXPORT_BUILD_EXPORT,
  EXPORT_BUILD_BUNDLE,
};

static void
sniff_flatpak_builder_version (GbpFlatpakPipelineAddin *self)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeSubprocess) subprocess = NULL;
  g_autofree gchar *stdout_buf = NULL;
  int major = 0;
  int minor = 0;
  int micro = 0;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));

  launcher = ide_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE);
  ide_subprocess_launcher_set_run_on_host (launcher, TRUE);
  ide_subprocess_launcher_set_clear_env (launcher, FALSE);
  ide_subprocess_launcher_setenv (launcher, "LANG", "C", TRUE);
  ide_subprocess_launcher_push_argv (launcher, "flatpak-builder");
  ide_subprocess_launcher_push_argv (launcher, "--version");

  subprocess = ide_subprocess_launcher_spawn (launcher, NULL, NULL);
  if (subprocess == NULL)
    return;

  if (!ide_subprocess_communicate_utf8 (subprocess, NULL, NULL, &stdout_buf, NULL, NULL))
    return;

  if (sscanf (stdout_buf, "flatpak-builder %d.%d.%d", &major, &minor, &micro) != 3)
    return;

  self->version.major = major;
  self->version.minor = minor;
  self->version.micro = micro;
}

static void
always_run_query_handler (IdeBuildStage    *stage,
                          IdeBuildPipeline *pipeline)
{
  g_return_if_fail (IDE_IS_BUILD_STAGE (stage));
  g_return_if_fail (IDE_IS_BUILD_PIPELINE (pipeline));

  ide_build_stage_set_completed (stage, FALSE);
}

static IdeSubprocessLauncher *
create_subprocess_launcher (void)
{
  IdeSubprocessLauncher *launcher;

  launcher = ide_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                          G_SUBPROCESS_FLAGS_STDERR_PIPE);
  ide_subprocess_launcher_set_run_on_host (launcher, TRUE);
  ide_subprocess_launcher_set_clear_env (launcher, FALSE);

  return launcher;
}

static gboolean
register_mkdirs_stage (GbpFlatpakPipelineAddin  *self,
                       IdeBuildPipeline         *pipeline,
                       IdeContext               *context,
                       GError                  **error)
{
  g_autoptr(IdeBuildStage) mkdirs = NULL;
  IdeConfiguration *config;
  g_autofree gchar *repo_dir = NULL;
  g_autofree gchar *staging_dir = NULL;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  config = ide_build_pipeline_get_configuration (pipeline);

  mkdirs = ide_build_stage_mkdirs_new (context);
  ide_build_stage_set_name (mkdirs, _("Creating flatpak workspace"));

  repo_dir = gbp_flatpak_get_repo_dir (config);
  staging_dir = gbp_flatpak_get_staging_dir (config);

  ide_build_stage_mkdirs_add_path (IDE_BUILD_STAGE_MKDIRS (mkdirs), repo_dir, TRUE, 0750, FALSE);
  ide_build_stage_mkdirs_add_path (IDE_BUILD_STAGE_MKDIRS (mkdirs), staging_dir, TRUE, 0750, TRUE);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_PREPARE, PREPARE_MKDIRS, mkdirs);

  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static void
check_if_file_exists (IdeBuildStage    *stage,
                      IdeBuildPipeline *pipeline,
                      GCancellable     *cancellable,
                      const gchar      *file_path)
{
  gboolean exists;

  g_assert (IDE_IS_BUILD_STAGE (stage));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_assert (file_path != NULL);

  exists = g_file_test (file_path, G_FILE_TEST_IS_REGULAR);

  IDE_TRACE_MSG ("%s::query checking for %s: %s",
                 G_OBJECT_TYPE_NAME (stage),
                 file_path,
                 exists ? "yes" : "no");

  ide_build_stage_set_completed (stage, exists);
}

static gboolean
register_build_init_stage (GbpFlatpakPipelineAddin  *self,
                           IdeBuildPipeline         *pipeline,
                           IdeContext               *context,
                           GError                  **error)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeBuildStage) stage = NULL;
  g_autofree gchar *staging_dir = NULL;
  g_autofree gchar *sdk = NULL;
  g_autofree gchar *metadata_path = NULL;
  IdeConfiguration *config;
  IdeRuntime *runtime;
  const gchar *app_id;
  const gchar *platform;
  const gchar *branch;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  launcher = create_subprocess_launcher ();

  config = ide_build_pipeline_get_configuration (pipeline);
  runtime = ide_configuration_get_runtime (config);

  if (!GBP_IS_FLATPAK_RUNTIME (runtime))
    {
      g_set_error (error,
                   GB_FLATPAK_PIPELINE_ERROR,
                   GB_FLATPAK_PIPELINE_ERROR_WRONG_RUNTIME,
                   "Configuration changed to a non-flatpak runtime during pipeline initialization");
      return FALSE;
    }

  staging_dir = gbp_flatpak_get_staging_dir (config);
  app_id = ide_configuration_get_app_id (config);
  platform = gbp_flatpak_runtime_get_platform (GBP_FLATPAK_RUNTIME (runtime));
  sdk = gbp_flatpak_runtime_get_sdk_name (GBP_FLATPAK_RUNTIME (runtime));
  branch = gbp_flatpak_runtime_get_branch (GBP_FLATPAK_RUNTIME (runtime));

  /*
   * If we got here by using a non-flatpak configuration, then there is a
   * chance we don't have a valid app-id.
   */
  if (dzl_str_empty0 (app_id))
    app_id = "com.example.App";

  if (platform == NULL && sdk == NULL)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                   "Platform and SDK are both NULL");
      return FALSE;
    }

  if (platform == NULL)
    platform = sdk;

  if (sdk == NULL)
    sdk = g_strdup (platform);

  metadata_path = g_build_filename (staging_dir, "metadata", NULL);

  ide_subprocess_launcher_push_argv (launcher, "flatpak");
  ide_subprocess_launcher_push_argv (launcher, "build-init");
  ide_subprocess_launcher_push_argv (launcher, staging_dir);
  ide_subprocess_launcher_push_argv (launcher, app_id);
  ide_subprocess_launcher_push_argv (launcher, sdk);
  ide_subprocess_launcher_push_argv (launcher, platform);
  ide_subprocess_launcher_push_argv (launcher, branch);

  stage = g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                        "name", _("Preparing build directory"),
                        "context", context,
                        "launcher", launcher,
                        NULL);

  /*
   * We want to avoid calling build-init if it has already been called.
   * To check if this has happened, we just look for the manifest file
   * located in the directory that had build-init called.
   */
  g_signal_connect_data (stage,
                         "query",
                         G_CALLBACK (check_if_file_exists),
                         g_steal_pointer (&metadata_path),
                         (GClosureNotify)g_free,
                         0);

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_PREPARE,
                                         PREPARE_BUILD_INIT,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static gboolean
register_downloads_stage (GbpFlatpakPipelineAddin  *self,
                          IdeBuildPipeline         *pipeline,
                          IdeContext               *context,
                          GError                  **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = g_object_new (GBP_TYPE_FLATPAK_DOWNLOAD_STAGE,
                        "name", _("Downloading dependencies"),
                        "context", context,
                        "state-dir", self->state_dir,
                        NULL);
  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_DOWNLOADS, 0, stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static gboolean
register_dependencies_stage (GbpFlatpakPipelineAddin  *self,
                             IdeBuildPipeline         *pipeline,
                             IdeContext               *context,
                             GError                  **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autofree gchar *staging_dir = NULL;
  g_autofree gchar *stop_at_option = NULL;
  IdeConfiguration *config;
  g_autofree gchar *manifest_path = NULL;
  const gchar *primary_module;
  const gchar *src_dir;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  config = ide_build_pipeline_get_configuration (pipeline);

  /* If there is no manifest, then there are no dependencies
   * to build for this configuration.
   */
  if (!GBP_IS_FLATPAK_CONFIGURATION (config))
    return TRUE;

  primary_module = gbp_flatpak_configuration_get_primary_module (GBP_FLATPAK_CONFIGURATION (config));
  manifest_path = gbp_flatpak_configuration_get_manifest_path (GBP_FLATPAK_CONFIGURATION (config));

  staging_dir = gbp_flatpak_get_staging_dir (config);
  src_dir = ide_build_pipeline_get_srcdir (pipeline);

  launcher = create_subprocess_launcher ();

  ide_subprocess_launcher_set_cwd (launcher, src_dir);

  ide_subprocess_launcher_push_argv (launcher, "flatpak-builder");
  ide_subprocess_launcher_push_argv (launcher, "--ccache");
  ide_subprocess_launcher_push_argv (launcher, "--force-clean");
  ide_subprocess_launcher_push_argv (launcher, "--disable-updates");
  if (self->state_dir != NULL)
    {
      ide_subprocess_launcher_push_argv (launcher, "--state-dir");
      ide_subprocess_launcher_push_argv (launcher, self->state_dir);
    }
  stop_at_option = g_strdup_printf ("--stop-at=%s", primary_module);
  ide_subprocess_launcher_push_argv (launcher, stop_at_option);
  ide_subprocess_launcher_push_argv (launcher, staging_dir);
  ide_subprocess_launcher_push_argv (launcher, manifest_path);

  stage = g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                        "name", _("Building dependencies"),
                        "context", context,
                        "launcher", launcher,
                        NULL);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_DEPENDENCIES, 0, stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static gboolean
register_build_finish_stage (GbpFlatpakPipelineAddin  *self,
                             IdeBuildPipeline         *pipeline,
                             IdeContext               *context,
                             GError                  **error)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeBuildStage) stage = NULL;
  g_autofree gchar *staging_dir = NULL;
  const gchar *const *finish_args;
  IdeConfiguration *config;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  config = ide_build_pipeline_get_configuration (pipeline);
  if (!GBP_IS_FLATPAK_CONFIGURATION (config))
    return TRUE;

  finish_args = gbp_flatpak_configuration_get_finish_args (GBP_FLATPAK_CONFIGURATION (config));
  staging_dir = gbp_flatpak_get_staging_dir (config);

  launcher = create_subprocess_launcher ();

  ide_subprocess_launcher_push_argv (launcher, "flatpak");
  ide_subprocess_launcher_push_argv (launcher, "build-finish");
  ide_subprocess_launcher_push_args (launcher, finish_args);
  ide_subprocess_launcher_push_argv (launcher, staging_dir);

  stage = g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                        "name", _("Finalizing flatpak build"),
                        "context", context,
                        "launcher", launcher,
                        NULL);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_EXPORT, EXPORT_BUILD_FINISH, stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static gboolean
register_build_export_stage (GbpFlatpakPipelineAddin  *self,
                             IdeBuildPipeline         *pipeline,
                             IdeContext               *context,
                             GError                  **error)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeBuildStage) stage = NULL;
  g_autofree gchar *repo_dir = NULL;
  g_autofree gchar *staging_dir = NULL;
  IdeConfiguration *config;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  config = ide_build_pipeline_get_configuration (pipeline);
  if (!GBP_IS_FLATPAK_CONFIGURATION (config))
    return TRUE;

  staging_dir = gbp_flatpak_get_staging_dir (config);
  repo_dir = gbp_flatpak_get_repo_dir (config);

  launcher = create_subprocess_launcher ();

  ide_subprocess_launcher_push_argv (launcher, "flatpak");
  ide_subprocess_launcher_push_argv (launcher, "build-export");
  ide_subprocess_launcher_push_argv (launcher, repo_dir);
  ide_subprocess_launcher_push_argv (launcher, staging_dir);

  stage = g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                        "name", _("Exporting staging directory"),
                        "context", context,
                        "launcher", launcher,
                        NULL);

  g_signal_connect (stage,
                    "query",
                    G_CALLBACK (always_run_query_handler),
                    NULL);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_EXPORT, EXPORT_BUILD_EXPORT, stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static void
build_bundle_notify_completed (IdeBuildStage *stage,
                               GParamSpec    *pspec,
                               const gchar   *dest_path)
{
  g_assert (IDE_IS_BUILD_STAGE (stage));
  g_assert (dest_path != NULL);

  /*
   * If we successfully completed the build-bundle, show the file
   * to the user so they can copy/paste/share/etc.
   */

  if (ide_build_stage_get_completed (stage))
    {
      g_autoptr(GFile) file = g_file_new_for_path (dest_path);
      dzl_file_manager_show (file, NULL);
    }
}

static gboolean
register_build_bundle_stage (GbpFlatpakPipelineAddin  *self,
                             IdeBuildPipeline         *pipeline,
                             IdeContext               *context,
                             GError                  **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autofree gchar *staging_dir = NULL;
  g_autofree gchar *repo_dir = NULL;
  g_autofree gchar *export_path = NULL;
  g_autofree gchar *dest_path = NULL;
  g_autofree gchar *name = NULL;
  IdeConfiguration *config;
  const gchar *app_id;
  guint stage_id;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  config = ide_build_pipeline_get_configuration (pipeline);
  if (!GBP_IS_FLATPAK_CONFIGURATION (config))
    return TRUE;

  staging_dir = gbp_flatpak_get_staging_dir (config);
  repo_dir = gbp_flatpak_get_repo_dir (config);

  app_id = ide_configuration_get_app_id (config);
  name = g_strdup_printf ("%s.flatpak", app_id);
  dest_path = g_build_filename (staging_dir, name, NULL);

  launcher = create_subprocess_launcher ();

  ide_subprocess_launcher_push_argv (launcher, "flatpak");
  ide_subprocess_launcher_push_argv (launcher, "build-bundle");
  ide_subprocess_launcher_push_argv (launcher, repo_dir);
  ide_subprocess_launcher_push_argv (launcher, dest_path);
  ide_subprocess_launcher_push_argv (launcher, app_id);
  /* TODO:
   *
   * We probably need to provide UI/config opt to tweak the branch name
   * if (ide_configuration_get_is_release (config))
   */
  ide_subprocess_launcher_push_argv (launcher, "master");

  stage = g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                        "name", _("Creating flatpak bundle"),
                        "context", context,
                        "launcher", launcher,
                        NULL);

  g_signal_connect (stage,
                    "query",
                    G_CALLBACK (always_run_query_handler),
                    NULL);

  g_signal_connect_data (stage,
                         "notify::completed",
                         G_CALLBACK (build_bundle_notify_completed),
                         g_steal_pointer (&dest_path),
                         (GClosureNotify)g_free,
                         0);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_EXPORT, EXPORT_BUILD_BUNDLE, stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static void
gbp_flatpak_pipeline_addin_load (IdeBuildPipelineAddin *addin,
                                 IdeBuildPipeline      *pipeline)
{
  GbpFlatpakPipelineAddin *self = (GbpFlatpakPipelineAddin *)addin;
  g_autoptr(GError) error = NULL;
  IdeConfiguration *config;
  IdeContext *context;
  IdeRuntime *runtime;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  sniff_flatpak_builder_version (self);

  if (VERSION_CHECK (&self->version, 0, 10, 5))
    {
      /* Use a single flatpak-builder state directory that is
       * kept within .cache (or appropriate mapping directory)
       * instead of littering it within the project checkout.
       * It also allows us to share checkouts and clones between
       * projects instead lots of duplicates.
       *
       * It does have the side effect that it is harder to
       * prune existing data and we may want to address that
       * in the future (either upstream or in here).
       */
      self->state_dir = g_build_filename (g_get_user_cache_dir (),
                                          ide_get_program_name (),
                                          "flatpak-builder",
                                          NULL);
    }

  config = ide_build_pipeline_get_configuration (pipeline);

  /* TODO: Once we have GbpFlatpakConfiguration, we can check for
   *       that (and it should only allow for valid flatpak runtimes).
   */

  runtime = ide_configuration_get_runtime (config);

  if (!GBP_IS_FLATPAK_RUNTIME (runtime))
    {
      g_message ("Configuration is not using flatpak, ignoring pipeline");
      return;
    }

  /*
   * TODO: We should add the ability to mark a pipeline as broken, if we
   *       detect something that is alarming. That will prevent builds from
   *       occuring altogether and allow us to present issues within the UI.
   */

  context = ide_object_get_context (IDE_OBJECT (self));

  if (!register_mkdirs_stage (self, pipeline, context, &error) ||
      !register_build_init_stage (self, pipeline, context, &error) ||
      !register_downloads_stage (self, pipeline, context, &error) ||
      !register_dependencies_stage (self, pipeline, context, &error) ||
      !register_build_finish_stage (self, pipeline, context, &error) ||
      !register_build_export_stage (self, pipeline, context, &error) ||
      !register_build_bundle_stage (self, pipeline, context, &error))
    g_warning ("%s", error->message);
}

static void
gbp_flatpak_pipeline_addin_unload (IdeBuildPipelineAddin *addin,
                                   IdeBuildPipeline      *pipeline)
{
  GbpFlatpakPipelineAddin *self = (GbpFlatpakPipelineAddin *)addin;

  g_assert (GBP_IS_FLATPAK_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  g_clear_pointer (&self->state_dir, g_free);
}

/* GObject boilerplate */

static void
build_pipeline_addin_iface_init (IdeBuildPipelineAddinInterface *iface)
{
  iface->load = gbp_flatpak_pipeline_addin_load;
  iface->unload = gbp_flatpak_pipeline_addin_unload;
}

G_DEFINE_TYPE_WITH_CODE (GbpFlatpakPipelineAddin, gbp_flatpak_pipeline_addin, IDE_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_BUILD_PIPELINE_ADDIN,
                                                build_pipeline_addin_iface_init))

static void
gbp_flatpak_pipeline_addin_class_init (GbpFlatpakPipelineAddinClass *klass)
{
}

static void
gbp_flatpak_pipeline_addin_init (GbpFlatpakPipelineAddin *self)
{
}
