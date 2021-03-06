/* gbp-flatpak-download-stage.c
 *
 * Copyright © 2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "gbp-flatpak-download-stage"

#include <glib/gi18n.h>

#include "gbp-flatpak-configuration.h"
#include "gbp-flatpak-download-stage.h"
#include "gbp-flatpak-util.h"

struct _GbpFlatpakDownloadStage
{
  IdeBuildStageLauncher parent_instance;

  gchar *state_dir;

  guint invalid : 1;
  guint force_update : 1;
};

G_DEFINE_TYPE (GbpFlatpakDownloadStage, gbp_flatpak_download_stage, IDE_TYPE_BUILD_STAGE_LAUNCHER)

enum {
  PROP_0,
  PROP_STATE_DIR,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
gbp_flatpak_download_stage_query (IdeBuildStage    *stage,
                                  IdeBuildPipeline *pipeline,
                                  GCancellable     *cancellable)
{
  GbpFlatpakDownloadStage *self = (GbpFlatpakDownloadStage *)stage;
  IdeConfiguration *config;
  GNetworkMonitor *monitor;
  g_autofree gchar *staging_dir = NULL;
  g_autofree gchar *manifest_path = NULL;
  g_autofree gchar *stop_at_option = NULL;
  const gchar *src_dir;
  const gchar *primary_module;

  g_assert (GBP_IS_FLATPAK_DOWNLOAD_STAGE (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  config = ide_build_pipeline_get_configuration (pipeline);
  if (!GBP_IS_FLATPAK_CONFIGURATION (config))
    {
      ide_build_stage_set_completed (stage, TRUE);
      return;
    }

  if (self->invalid)
    {
      g_autoptr(IdeSubprocessLauncher) launcher = NULL;

      primary_module = gbp_flatpak_configuration_get_primary_module (GBP_FLATPAK_CONFIGURATION (config));
      manifest_path = gbp_flatpak_configuration_get_manifest_path (GBP_FLATPAK_CONFIGURATION (config));
      staging_dir = gbp_flatpak_get_staging_dir (config);
      src_dir = ide_build_pipeline_get_srcdir (pipeline);

      launcher = ide_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                              G_SUBPROCESS_FLAGS_STDERR_PIPE);

      ide_subprocess_launcher_set_run_on_host (launcher, TRUE);
      ide_subprocess_launcher_set_clear_env (launcher, FALSE);
      ide_subprocess_launcher_set_cwd (launcher, src_dir);

      ide_subprocess_launcher_push_argv (launcher, "flatpak-builder");
      ide_subprocess_launcher_push_argv (launcher, "--ccache");
      ide_subprocess_launcher_push_argv (launcher, "--force-clean");
      if (!dzl_str_empty0 (self->state_dir))
        {
          ide_subprocess_launcher_push_argv (launcher, "--state-dir");
          ide_subprocess_launcher_push_argv (launcher, self->state_dir);
        }
      ide_subprocess_launcher_push_argv (launcher, "--download-only");
      if (!self->force_update)
        ide_subprocess_launcher_push_argv (launcher, "--disable-updates");
      stop_at_option = g_strdup_printf ("--stop-at=%s", primary_module);
      ide_subprocess_launcher_push_argv (launcher, stop_at_option);
      ide_subprocess_launcher_push_argv (launcher, staging_dir);
      ide_subprocess_launcher_push_argv (launcher, manifest_path);

      ide_build_stage_launcher_set_launcher (IDE_BUILD_STAGE_LAUNCHER (self), launcher);
      ide_build_stage_set_completed (stage, FALSE);

      self->invalid = FALSE;
      self->force_update = FALSE;
    }

  /* Ignore downloads if there is no connection */
  monitor = g_network_monitor_get_default ();
  if (!g_network_monitor_get_network_available (monitor))
    {
      ide_build_stage_log (stage,
                           IDE_BUILD_LOG_STDOUT,
                           _("Network is not available, skipping downloads"),
                           -1);
      ide_build_stage_set_completed (stage, TRUE);
    }
}

static void
gbp_flatpak_download_stage_finalize (GObject *object)
{
  GbpFlatpakDownloadStage *self = (GbpFlatpakDownloadStage *)object;

  g_assert (GBP_IS_FLATPAK_DOWNLOAD_STAGE (self));

  g_clear_pointer (&self->state_dir, g_free);

  G_OBJECT_CLASS (gbp_flatpak_download_stage_parent_class)->finalize (object);
}

static void
gbp_flatpak_download_stage_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  GbpFlatpakDownloadStage *self = GBP_FLATPAK_DOWNLOAD_STAGE (object);

  switch (prop_id)
    {
    case PROP_STATE_DIR:
      self->state_dir = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_flatpak_download_stage_class_init (GbpFlatpakDownloadStageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  IdeBuildStageClass *stage_class = IDE_BUILD_STAGE_CLASS (klass);

  object_class->finalize = gbp_flatpak_download_stage_finalize;
  object_class->set_property = gbp_flatpak_download_stage_set_property;

  stage_class->query = gbp_flatpak_download_stage_query;

  properties [PROP_STATE_DIR] =
    g_param_spec_string ("state-dir", NULL, NULL, NULL,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gbp_flatpak_download_stage_init (GbpFlatpakDownloadStage *self)
{
  self->invalid = TRUE;
}

void
gbp_flatpak_download_stage_force_update (GbpFlatpakDownloadStage *self)
{
  g_return_if_fail (GBP_IS_FLATPAK_DOWNLOAD_STAGE (self));

  self->force_update = TRUE;
  self->invalid = TRUE;
}
