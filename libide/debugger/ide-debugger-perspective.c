/* ide-debugger-perspective.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "ide-debugger-perspective"

#include <dazzle.h>
#include <glib/gi18n.h>

#include "ide-debug.h"

#include "buffers/ide-buffer.h"
#include "files/ide-file.h"
#include "debugger/ide-debugger.h"
#include "debugger/ide-debugger-perspective.h"
#include "debugger/ide-debugger-view.h"
#include "layout/ide-layout-grid.h"
#include "workbench/ide-perspective.h"
#include <gdk/gdk.h>
#include "ide-internal.h"

#define IDE_DEBUGGER_STOPPED_LINE_TAG_NAME "ide-debugger-stopped-line-tag-name"

struct _IdeDebuggerPerspective
{
  DzlDockBin      parent_instance;

  IdeDebugger     *debugger;
  DzlSignalGroup  *debugger_signals;
  GSettings       *terminal_settings;
  GtkCssProvider  *log_css;

  GtkTextBuffer   *log_buffer;
  GtkTextView     *log_text_view;
  IdeLayoutGrid   *layout_grid;
  IdeDebuggerView *view;
};

enum {
  PROP_0,
  PROP_DEBUGGER,
  N_PROPS
};

typedef struct {
   IdeBreakpoint          *breakpoint;
   IdeDebuggerPerspective *perspective;
} IdeDebuggerInfo;

static gchar *
ide_debugger_perspective_get_title (IdePerspective *perspective)
{
  return g_strdup (_("Debugger"));
}

static gchar *
ide_debugger_perspective_get_id (IdePerspective *perspective)
{
  return g_strdup ("debugger");
}

static gchar *
ide_debugger_perspective_get_icon_name (IdePerspective *perspective)
{
  return g_strdup ("builder-debugger-symbolic");
}

static gchar *
ide_debugger_perspective_get_accelerator (IdePerspective *perspective)
{
  return g_strdup ("<Alt>2");
}

static void
perspective_iface_init (IdePerspectiveInterface *iface)
{
  iface->get_accelerator = ide_debugger_perspective_get_accelerator;
  iface->get_icon_name = ide_debugger_perspective_get_icon_name;
  iface->get_id = ide_debugger_perspective_get_id;
  iface->get_title = ide_debugger_perspective_get_title;
}

G_DEFINE_TYPE_EXTENDED (IdeDebuggerPerspective, ide_debugger_perspective, DZL_TYPE_DOCK_BIN, 0,
                        G_IMPLEMENT_INTERFACE (IDE_TYPE_PERSPECTIVE, perspective_iface_init))

static GParamSpec *properties [N_PROPS];

static void
on_debugger_log (IdeDebuggerPerspective *self,
                 const gchar            *message,
                 IdeDebugger            *debugger)
{
  GtkTextIter iter;

  g_assert (IDE_IS_DEBUGGER_PERSPECTIVE (self));
  g_assert (IDE_IS_DEBUGGER (debugger));

  gtk_text_buffer_get_end_iter (self->log_buffer, &iter);
  gtk_text_buffer_insert (self->log_buffer, &iter, message, -1);
  gtk_text_buffer_select_range (self->log_buffer, &iter, &iter);
  gtk_text_view_scroll_to_iter (self->log_text_view, &iter, 0.0, FALSE, 1.0, 1.0);
}

void
ide_debugger_perspective_set_debugger (IdeDebuggerPerspective *self,
                                       IdeDebugger            *debugger)
{
  IDE_ENTRY;

  g_return_if_fail (IDE_IS_DEBUGGER_PERSPECTIVE (self));
  g_return_if_fail (!debugger || IDE_IS_DEBUGGER (debugger));

  if (g_set_object (&self->debugger, debugger))
    {
      dzl_signal_group_set_target (self->debugger_signals, debugger);
      gtk_text_buffer_set_text (self->log_buffer, "", 0);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_DEBUGGER]);
    }

  IDE_EXIT;
}

static void
log_panel_changed_font_name (IdeDebuggerPerspective *self,
                             const gchar            *key,
                             GSettings              *settings)
{
  gchar *font_name;
  PangoFontDescription *font_desc;

  g_assert (IDE_IS_DEBUGGER_PERSPECTIVE (self));
  g_assert (g_strcmp0 (key, "font-name") == 0);
  g_assert (G_IS_SETTINGS (settings));

  font_name = g_settings_get_string (settings, key);
  font_desc = pango_font_description_from_string (font_name);

  if (font_desc != NULL)
    {
      gchar *fragment;
      gchar *css;

      fragment = dzl_pango_font_description_to_css (font_desc);
      css = g_strdup_printf ("textview { %s }", fragment);

      gtk_css_provider_load_from_data (self->log_css, css, -1, NULL);

      pango_font_description_free (font_desc);
      g_free (fragment);
      g_free (css);
    }

  g_free (font_name);
}

static void
on_debugger_stopped (IdeDebuggerPerspective *self,
                     IdeDebuggerStopReason   reason,
                     IdeBreakpoint          *breakpoint,
                     IdeDebugger            *debugger)
{
  IDE_ENTRY;

  g_assert (IDE_IS_DEBUGGER_PERSPECTIVE (self));
  g_assert (!breakpoint || IDE_IS_BREAKPOINT (breakpoint));
  g_assert (IDE_IS_DEBUGGER (debugger));

  if (breakpoint != NULL)
    ide_debugger_perspective_navigate_to_breakpoint (self, breakpoint);

  IDE_EXIT;
}

static void
ide_debugger_perspective_finalize (GObject *object)
{
  IdeDebuggerPerspective *self = (IdeDebuggerPerspective *)object;

  g_clear_object (&self->debugger);
  g_clear_object (&self->debugger_signals);
  g_clear_object (&self->terminal_settings);
  g_clear_object (&self->log_css);

  G_OBJECT_CLASS (ide_debugger_perspective_parent_class)->finalize (object);
}

static void
ide_debugger_perspective_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  IdeDebuggerPerspective *self = IDE_DEBUGGER_PERSPECTIVE (object);

  switch (prop_id)
    {
    case PROP_DEBUGGER:
      g_value_set_object (value, self->debugger);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_debugger_perspective_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  IdeDebuggerPerspective *self = IDE_DEBUGGER_PERSPECTIVE (object);

  switch (prop_id)
    {
    case PROP_DEBUGGER:
      ide_debugger_perspective_set_debugger (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_debugger_perspective_class_init (IdeDebuggerPerspectiveClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = ide_debugger_perspective_finalize;
  object_class->get_property = ide_debugger_perspective_get_property;
  object_class->set_property = ide_debugger_perspective_set_property;

  properties [PROP_DEBUGGER] =
    g_param_spec_object ("debugger",
                         "Debugger",
                         "The current debugger instance",
                         IDE_TYPE_DEBUGGER,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/builder/ui/ide-debugger-perspective.ui");
  gtk_widget_class_bind_template_child (widget_class, IdeDebuggerPerspective, layout_grid);
  gtk_widget_class_bind_template_child (widget_class, IdeDebuggerPerspective, log_text_view);
  gtk_widget_class_bind_template_child (widget_class, IdeDebuggerPerspective, log_buffer);
}

static void
ide_debugger_perspective_init (IdeDebuggerPerspective *self)
{
  GtkStyleContext *context;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->debugger_signals = dzl_signal_group_new (IDE_TYPE_DEBUGGER);

  dzl_signal_group_connect_object (self->debugger_signals,
                                   "log",
                                   G_CALLBACK (on_debugger_log),
                                   self,
                                   G_CONNECT_SWAPPED);

  dzl_signal_group_connect_object (self->debugger_signals,
                                   "stopped",
                                   G_CALLBACK (on_debugger_stopped),
                                   self,
                                   G_CONNECT_SWAPPED);

  self->log_css = gtk_css_provider_new ();
  context = gtk_widget_get_style_context (GTK_WIDGET (self->log_text_view));
  gtk_style_context_add_provider (context,
                                  GTK_STYLE_PROVIDER (self->log_css),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  self->terminal_settings = g_settings_new ("org.gnome.builder.terminal");
  g_signal_connect_object (self->terminal_settings,
                           "changed::font-name",
                           G_CALLBACK (log_panel_changed_font_name),
                           self,
                           G_CONNECT_SWAPPED);
  log_panel_changed_font_name (self, "font-name", self->terminal_settings);
}

static gboolean
mark_done (gpointer data)
{
  gboolean *done = data;
  *done = TRUE;
  return G_SOURCE_REMOVE;
}

static void
ide_debugger_perspective_load_source_cb (GObject      *object,
                                         GAsyncResult *result,
                                         gpointer      user_data)
{
  IdeDebugger *debugger = (IdeDebugger *)object;
  IdeDebuggerInfo *ide_debugger_info = user_data;
  g_autoptr(IdeDebuggerPerspective) self = ide_debugger_info->perspective;
  g_autoptr(IdeBreakpoint) breakpoint = ide_debugger_info->breakpoint;
  g_autoptr(IdeBuffer) buffer = NULL;
  g_autoptr(GError) error = NULL;
  GtkWidget *stack;
  GtkTextIter iter_bkp;
  GtkTextIter iter_bkp_end;
  GtkTextIter iter_start;
  GtkTextIter iter_end;
  guint line;
  guint line_offset;
  GdkRGBA stopped_line_rgba;
  GtkSourceFile *source_file;
  gboolean done;

  IDE_ENTRY;

  g_assert (IDE_IS_DEBUGGER (debugger));
  g_assert (G_IS_ASYNC_RESULT (result));
  g_assert (IDE_IS_DEBUGGER_PERSPECTIVE (self));

  line = ide_breakpoint_get_line (breakpoint);
  line_offset = ide_breakpoint_get_line_offset (breakpoint);

  buffer = ide_debugger_load_source_finish (debugger, result, &error);

  if (buffer == NULL)
    {
      g_warning ("%s", error->message);
      IDE_GOTO (failure);
    }

  gdk_rgba_parse(&stopped_line_rgba, "rgba(235,202,210,.4)");
  gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer),
                              IDE_DEBUGGER_STOPPED_LINE_TAG_NAME,
                              "background-rgba", &stopped_line_rgba,
                              NULL);

  self->view = g_object_new (IDE_TYPE_DEBUGGER_VIEW,
                             "buffer", buffer,
                             "visible", TRUE,
                             NULL);

  //stack = ide_layout_grid_get_last_focus (self->layout_grid);
  //gtk_container_add (GTK_CONTAINER (stack), GTK_WIDGET (self->view));

  done = FALSE;

  /*
   * timeout value of 100 was found experimentally.  this is utter crap, but I
   * don't have a clear event to key off with signals. we are sort of just
   * waiting for the textview to complete processing events.
   */

  g_timeout_add (100, mark_done, &done);

  for (;;)
    {
      gtk_main_iteration_do (TRUE);
      if (done)
        break;
    }

  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &iter_start);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &iter_end);

  gtk_text_buffer_get_iter_at_line_offset (GTK_TEXT_BUFFER (buffer),
                                           &iter_bkp,
                                           line - 1,
                                           line_offset);
  gtk_text_buffer_get_iter_at_line_offset (GTK_TEXT_BUFFER (buffer),
                                           &iter_bkp_end,
                                           line - 1,
                                           line_offset);
  gtk_text_iter_forward_to_line_end (&iter_bkp_end);
  IDE_TRACE_MSG ("breakpoint navigate %d:%d", line, line_offset);
  gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (ide_debugger_view_get_view(self->view)), &iter_bkp, 0.1, TRUE, 0.5, 0.0);
  gtk_text_buffer_remove_tag_by_name (GTK_TEXT_BUFFER (buffer), IDE_DEBUGGER_STOPPED_LINE_TAG_NAME, &iter_start, &iter_end) ;
  gtk_text_buffer_apply_tag_by_name (GTK_TEXT_BUFFER (buffer), IDE_DEBUGGER_STOPPED_LINE_TAG_NAME, &iter_bkp, &iter_bkp_end);

failure:
  IDE_EXIT;
}

void
ide_debugger_perspective_navigate_to_breakpoint (IdeDebuggerPerspective *self,
                                                 IdeBreakpoint          *breakpoint)
{
  IdeDebuggerInfo *debuggerInfo;
  IdeBuffer *buffer;
  GtkTextIter iter_start;
  GtkTextIter iter_end;
  GtkTextIter iter_bkp;
  GtkTextIter iter_bkp_end;
  guint line;
  guint line_offset;
  gboolean load_buffer;
  IdeFile *orig_file;
  GtkSourceFile *source_file;
  GFile *file;
  GFile *file_bkp;

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_DEBUGGER_PERSPECTIVE (self));
  g_return_if_fail (IDE_IS_BREAKPOINT (breakpoint));
  g_return_if_fail (IDE_IS_DEBUGGER (self->debugger));

  /*
   * To display the source for the breakpoint, first we need to discover what
   * file contains the source. If there is no file, then we need to ask the
   * IdeDebugger to retrieve the disassembly for us so that we can show
   * something "useful" to the developer.
   *
   * If we also fail to get the disassembly for the current breakpoint, we
   * need to load some dummy text into a buffer to denote to the developer
   * that technically they can click forward, but the behavior is rather
   * undefined.
   *
   * If the file on disk is out of date (due to changes behind the scenes) we
   * will likely catch that with a CRC check. We will show the file, but the
   * user will have an infobar displayed that denotes that the file is not
   * longer in sync with the debugged executable.
   */

  load_buffer = (self->view == NULL);
  if (!load_buffer)
    {
      buffer = ide_debugger_view_get_buffer (self->view);
      if (NULL != (orig_file = ide_buffer_get_file (buffer)))
        {
          file = ide_file_get_file(orig_file);
          file_bkp = ide_breakpoint_get_file (breakpoint);
          if (!g_file_equal(file, file_bkp))
            {
              /* files are different */
              load_buffer = TRUE;
            }
          else
            {
              if (NULL != (source_file = _ide_file_get_source_file (orig_file)))
                {
                  if (gtk_source_file_is_local (source_file))
                    {
                      gtk_source_file_check_file_on_disk (source_file);
                      if (gtk_source_file_is_externally_modified (source_file))
                        {
                          /* file changed on disk */
                          load_buffer = TRUE;
                        }
                    }
                }
              else
                {
                  load_buffer = TRUE;
                }
            }
        }
      else
        {
          load_buffer = TRUE;
        }
    }

  if (load_buffer)
    {
      debuggerInfo = g_slice_new0 (IdeDebuggerInfo);
      debuggerInfo->breakpoint = g_object_ref (breakpoint);
      debuggerInfo->perspective = g_object_ref (self);

      ide_debugger_load_source_async (self->debugger,
                                      breakpoint,
                                      NULL,
                                      ide_debugger_perspective_load_source_cb,
                                      debuggerInfo);
    }
  else
    {
      line = ide_breakpoint_get_line (breakpoint);
      line_offset = ide_breakpoint_get_line_offset (breakpoint);
      buffer = ide_debugger_view_get_buffer (self->view);
      gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (buffer), &iter_start);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (buffer), &iter_end);
      gtk_text_buffer_get_iter_at_line_offset (GTK_TEXT_BUFFER (buffer),
                                               &iter_bkp,
                                               line - 1,
                                               line_offset);
      gtk_text_buffer_get_iter_at_line_offset (GTK_TEXT_BUFFER (buffer),
                                               &iter_bkp_end,
                                               line - 1,
                                               line_offset);
      gtk_text_iter_forward_to_line_end(&iter_bkp_end);
      IDE_TRACE_MSG ("breakpoint navigate: %d:%d", line, line_offset);
      gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (ide_debugger_view_get_view(self->view)), &iter_bkp, 0.1, TRUE, 0.5, 0.0);
      gtk_text_buffer_remove_tag_by_name (GTK_TEXT_BUFFER (buffer), IDE_DEBUGGER_STOPPED_LINE_TAG_NAME, &iter_start, &iter_end) ;
      gtk_text_buffer_apply_tag_by_name (GTK_TEXT_BUFFER (buffer), IDE_DEBUGGER_STOPPED_LINE_TAG_NAME, &iter_bkp, &iter_bkp_end);
    }

  IDE_EXIT;
}
