/* ide-debugger-view.c
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

#define G_LOG_DOMAIN "ide-debugger-view"

#include "ide-debug.h"

#include "debugger/ide-debugger-breakpoints.h"
#include "debugger/ide-debugger-gutter-renderer.h"
#include "debugger/ide-debugger-view.h"

struct _IdeDebuggerView
{
  IdeLayoutView              parent_instance;

  IdeSourceView             *source_view;
  IdeDebuggerBreakpoints    *breakpoints;
  IdeDebuggerGutterRenderer *breakpoints_gutter;
};

enum {
  PROP_0,
  PROP_BUFFER,
  PROP_VIEW,
  N_PROPS
};

G_DEFINE_TYPE (IdeDebuggerView, ide_debugger_view, IDE_TYPE_LAYOUT_VIEW)

static GParamSpec *properties [N_PROPS];

static gchar *
ide_debugger_view_get_title (IdeLayoutView *view)
{
  IdeDebuggerView *self = (IdeDebuggerView *)view;
  const gchar *title;
  IdeBuffer *buffer;

  g_assert (IDE_IS_DEBUGGER_VIEW (self));

  buffer = IDE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->source_view)));
  title = ide_buffer_get_title (buffer);

  return g_strdup (title);
}

static void
ide_debugger_view_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  IdeDebuggerView *self = IDE_DEBUGGER_VIEW (object);

  switch (prop_id)
    {
    case PROP_BUFFER:
      g_value_set_object (value, ide_debugger_view_get_buffer (self));
      break;
    case PROP_VIEW:
      g_value_set_object (value, ide_debugger_view_get_view (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_debugger_view_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  IdeDebuggerView *self = IDE_DEBUGGER_VIEW (object);

  switch (prop_id)
    {
    case PROP_BUFFER:
      ide_debugger_view_set_buffer (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_debugger_view_class_init (IdeDebuggerViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  IdeLayoutViewClass *view_class = IDE_LAYOUT_VIEW_CLASS (klass);

  object_class->get_property = ide_debugger_view_get_property;
  object_class->set_property = ide_debugger_view_set_property;

  view_class->get_title = ide_debugger_view_get_title;

  properties [PROP_BUFFER] =
    g_param_spec_object ("buffer",
                         "Buffer",
                         "The buffer for the view",
                         IDE_TYPE_BUFFER,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/builder/ui/ide-debugger-view.ui");
  gtk_widget_class_bind_template_child (widget_class, IdeDebuggerView, source_view);
}

static void
ide_debugger_view_init (IdeDebuggerView *self)
{
  GtkSourceGutter *gutter;

  gtk_widget_init_template (GTK_WIDGET (self));

  gutter = gtk_source_view_get_gutter (GTK_SOURCE_VIEW (self->source_view),
                                       GTK_TEXT_WINDOW_LEFT);
  self->breakpoints_gutter = g_object_new (IDE_TYPE_DEBUGGER_GUTTER_RENDERER,
                                           "visible", TRUE,
                                           NULL);
  gtk_source_gutter_insert (gutter, GTK_SOURCE_GUTTER_RENDERER (self->breakpoints_gutter), -100);
}

GtkWidget *
ide_debugger_view_new (void)
{
  return g_object_new (IDE_TYPE_DEBUGGER_VIEW, NULL);
}

/**
 * ide_debugger_view_get_view:
 * @self: a #IdeDebuggerView
 *
 * Gets the source view for the view.
 *
 * Returns: (transfer none): A #IdeSourceView
 */
IdeSourceView *
ide_debugger_view_get_view (IdeDebuggerView *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_VIEW (self), NULL);

  return self->source_view;
}

/**
 * ide_debugger_view_get_buffer:
 * @self: a #IdeDebuggerView
 *
 * Gets the buffer for the view.
 *
 * Returns: (transfer none): A #GtkSourceBuffer
 */
IdeBuffer *
ide_debugger_view_get_buffer (IdeDebuggerView *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_VIEW (self), NULL);

  return IDE_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->source_view)));
}

void
ide_debugger_view_set_buffer (IdeDebuggerView *self,
                              IdeBuffer       *buffer)
{
  g_return_if_fail (IDE_IS_DEBUGGER_VIEW (self));
  g_return_if_fail (GTK_SOURCE_IS_BUFFER (buffer));

  if (buffer != ide_debugger_view_get_buffer (self))
    {
      GtkSourceStyleScheme *scheme;
      GtkSourceLanguage *language;

      scheme = gtk_source_style_scheme_manager_get_scheme (gtk_source_style_scheme_manager_get_default (), "builder-dark");
      gtk_source_buffer_set_style_scheme (GTK_SOURCE_BUFFER (buffer), scheme);

      language = gtk_source_language_manager_get_language (gtk_source_language_manager_get_default (), "c");
      gtk_source_buffer_set_language (GTK_SOURCE_BUFFER (buffer), language);

      gtk_source_view_set_background_pattern (GTK_SOURCE_VIEW (self->source_view), GTK_SOURCE_BACKGROUND_PATTERN_TYPE_GRID);

      gtk_text_view_set_buffer (GTK_TEXT_VIEW (self->source_view), GTK_TEXT_BUFFER (buffer));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_BUFFER]);
    }
}
