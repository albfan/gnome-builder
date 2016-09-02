/* gbp-debugger-workbench-addin.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "gbp-debugger-workbench-addin"

#include <glib/gi18n.h>

#include "gbp-debugger-perspective.h"
#include "gbp-debugger-workbench-addin.h"

struct _GbpDebuggerWorkbenchAddin
{
  GObject                 parent_instance;

  IdeWorkbench           *workbench;
  GbpDebuggerPerspective *perspective;
  GSimpleActionGroup     *actions;

  GtkBox                 *exec_controls;
  GtkBox                 *step_controls;
};

static void
gbp_debugger_workbench_addin_runner_exited (GbpDebuggerWorkbenchAddin *self,
                                            IdeRunner                 *runner)
{
  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));
  g_assert (IDE_IS_RUNNER (runner));

  gtk_widget_hide (GTK_WIDGET (self->exec_controls));
  gtk_widget_hide (GTK_WIDGET (self->step_controls));

  ide_workbench_set_visible_perspective_name (self->workbench, "editor");
}

static void
debugger_run_handler (IdeRunManager *run_manager,
                      IdeRunner     *runner,
                      gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  IDE_ENTRY;

  g_assert (IDE_IS_RUN_MANAGER (run_manager));
  g_assert (IDE_IS_RUNNER (runner));
  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

  ide_workbench_set_visible_perspective_name (self->workbench, "debugger");

  g_signal_connect_object (runner,
                           "exited",
                           G_CALLBACK (gbp_debugger_workbench_addin_runner_exited),
                           self,
                           G_CONNECT_SWAPPED);

  gtk_widget_show (GTK_WIDGET (self->exec_controls));
  gtk_widget_show (GTK_WIDGET (self->step_controls));

#if 0
  ide_runner_prepend_argv (runner, "--args");
  ide_runner_prepend_argv (runner, "run");
  ide_runner_prepend_argv (runner, "-ex");
  ide_runner_prepend_argv (runner, "gdb");
#endif

  IDE_EXIT;
}

static void
gbp_debugger_workbench_addin_load (IdeWorkbenchAddin *addin,
                                   IdeWorkbench      *workbench)
{
  GbpDebuggerWorkbenchAddin *self = (GbpDebuggerWorkbenchAddin *)addin;
  IdeWorkbenchHeaderBar *headerbar;
  IdeRunManager *run_manager;
  IdeContext *context;
  GtkBox *box;

  g_assert (IDE_IS_WORKBENCH_ADDIN (addin));
  g_assert (IDE_IS_WORKBENCH (workbench));

  self->workbench = workbench;

  gtk_widget_insert_action_group (GTK_WIDGET (workbench),
                                  "debugger",
                                  G_ACTION_GROUP (self->actions));

  headerbar = ide_workbench_get_headerbar (workbench);

  context = ide_workbench_get_context (workbench);

  run_manager = ide_context_get_run_manager (context);
  ide_run_manager_add_handler (run_manager,
                               "debugger",
                               _("Run with Debugger"),
                               "nemiver-symbolic",
                               "F5",
                               debugger_run_handler,
                               g_object_ref (self),
                               g_object_unref);

  self->perspective = g_object_new (GBP_TYPE_DEBUGGER_PERSPECTIVE,
                                    "visible", TRUE,
                                    NULL);
  ide_workbench_add_perspective (workbench, IDE_PERSPECTIVE (self->perspective));

  self->exec_controls = g_object_new (GTK_TYPE_BOX,
                                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                                      NULL);
  ide_widget_add_style_class (GTK_WIDGET (self->exec_controls), "linked");
  ide_workbench_header_bar_insert_left (headerbar, GTK_WIDGET (self->exec_controls), GTK_PACK_START, 100);

#define ADD_BUTTON(action_name, icon_name, tooltip_text) \
  G_STMT_START { \
    GtkButton *button; \
    button = g_object_new (GTK_TYPE_BUTTON, \
                           "action-name", action_name, \
                           "child", g_object_new (GTK_TYPE_IMAGE, \
                                                  "icon-name", icon_name, \
                                                  "visible", TRUE, \
                                                  NULL), \
                           "tooltip-text", tooltip_text, \
                           "visible", TRUE, \
                           NULL); \
    gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (button)); \
  } G_STMT_END

  box = self->exec_controls;
  ADD_BUTTON ("debugger.continue",            "debug-continue-symbolic",            _("Continue"));
  ADD_BUTTON ("debugger.execute-from-cursor", "debug-execute-to-cursor-symbolic",   _("Execute to cursor"));
  ADD_BUTTON ("debugger.execute-to-cursor",   "debug-execute-from-cursor-symbolic", _("Execute from cursor"));

  self->step_controls = g_object_new (GTK_TYPE_BOX,
                                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                                      NULL);
  ide_widget_add_style_class (GTK_WIDGET (self->step_controls), "linked");
  ide_workbench_header_bar_insert_left (headerbar, GTK_WIDGET (self->step_controls), GTK_PACK_START, 110);

  box = self->step_controls;
  ADD_BUTTON ("debugger.step-in",   "debug-step-in-symbolic",   _("Step in"));
  ADD_BUTTON ("debugger.step-out",  "debug-step-out-symbolic",  _("Step out"));
  ADD_BUTTON ("debugger.step-over", "debug-step-over-symbolic", _("Step over"));
}

static void
gbp_debugger_workbench_addin_unload (IdeWorkbenchAddin *addin,
                                     IdeWorkbench      *workbench)
{
  GbpDebuggerWorkbenchAddin *self = (GbpDebuggerWorkbenchAddin *)addin;
  IdeRunManager *run_manager;
  IdeContext *context;

  g_assert (IDE_IS_WORKBENCH_ADDIN (addin));
  g_assert (IDE_IS_WORKBENCH (workbench));

  context = ide_workbench_get_context (workbench);
  run_manager = ide_context_get_run_manager (context);
  ide_run_manager_remove_handler (run_manager, "debugger");

  gtk_widget_insert_action_group (GTK_WIDGET (workbench), "debugger", NULL);

  ide_workbench_remove_perspective (workbench, IDE_PERSPECTIVE (self->perspective));
  self->perspective = NULL;

  self->workbench = NULL;
}

static void
workbench_addin_iface_init (IdeWorkbenchAddinInterface *iface)
{
  iface->load = gbp_debugger_workbench_addin_load;
  iface->unload = gbp_debugger_workbench_addin_unload;
}

G_DEFINE_TYPE_EXTENDED (GbpDebuggerWorkbenchAddin, gbp_debugger_workbench_addin, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (IDE_TYPE_WORKBENCH_ADDIN, workbench_addin_iface_init))

static void
gbp_debugger_workbench_addin_finalize (GObject *object)
{
  GbpDebuggerWorkbenchAddin *self = (GbpDebuggerWorkbenchAddin *)object;

  g_clear_object (&self->actions);

  G_OBJECT_CLASS (gbp_debugger_workbench_addin_parent_class)->finalize (object);
}

static void
gbp_debugger_workbench_addin_class_init (GbpDebuggerWorkbenchAddinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gbp_debugger_workbench_addin_finalize;
}

static void
step_in_activate (GSimpleAction *action,
                  GVariant      *param,
                  gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

}

static void
step_out_activate (GSimpleAction *action,
                   GVariant      *param,
                   gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

}

static void
step_over_activate (GSimpleAction *action,
                    GVariant      *param,
                    gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

}

static void
continue_action (GSimpleAction *action,
                 GVariant      *param,
                 gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

}

static void
execute_to_cursor_action (GSimpleAction *action,
                          GVariant      *param,
                          gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

}

static void
execute_from_cursor_action (GSimpleAction *action,
                            GVariant      *param,
                            gpointer       user_data)
{
  GbpDebuggerWorkbenchAddin *self = user_data;

  g_assert (GBP_IS_DEBUGGER_WORKBENCH_ADDIN (self));

}

static GActionEntry action_entries[] = {
  { "continue", continue_action },
  { "step-in", step_in_activate },
  { "step-out", step_out_activate },
  { "step-over", step_over_activate },
  { "execute-from-cursor", execute_from_cursor_action },
  { "execute-to-cursor", execute_to_cursor_action },
};

static void
gbp_debugger_workbench_addin_init (GbpDebuggerWorkbenchAddin *self)
{
  self->actions = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (self->actions),
                                   action_entries,
                                   G_N_ELEMENTS (action_entries),
                                   self);
}
