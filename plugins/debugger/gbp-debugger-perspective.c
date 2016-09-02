/* gbp-debugger-perspective.c
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

#define G_LOG_DOMAIN "gbp-debugger-perspective"

#include <glib/gi18n.h>

#include "gbp-debugger-perspective.h"

struct _GbpDebuggerPerspective
{
  IdeLayout parent_instance;
};

enum {
  PROP_0,
  N_PROPS
};

static gchar *
gbp_debugger_perspective_get_title (IdePerspective *perspective)
{
  return g_strdup (_("Debugger"));
}

static gchar *
gbp_debugger_perspective_get_id (IdePerspective *perspective)
{
  return g_strdup ("debugger");
}

static gchar *
gbp_debugger_perspective_get_icon_name (IdePerspective *perspective)
{
  return g_strdup ("nemiver-symbolic");
}

static gchar *
gbp_debugger_perspective_get_accelerator (IdePerspective *perspective)
{
  return g_strdup ("<Alt>2");
}

static void
perspective_iface_init (IdePerspectiveInterface *iface)
{
  iface->get_accelerator = gbp_debugger_perspective_get_accelerator;
  iface->get_icon_name = gbp_debugger_perspective_get_icon_name;
  iface->get_id = gbp_debugger_perspective_get_id;
  iface->get_title = gbp_debugger_perspective_get_title;
}

G_DEFINE_TYPE_EXTENDED (GbpDebuggerPerspective, gbp_debugger_perspective, IDE_TYPE_LAYOUT, 0,
                        G_IMPLEMENT_INTERFACE (IDE_TYPE_PERSPECTIVE, perspective_iface_init))

static GParamSpec *properties [N_PROPS];

static void
gbp_debugger_perspective_finalize (GObject *object)
{
  G_OBJECT_CLASS (gbp_debugger_perspective_parent_class)->finalize (object);
}

static void
gbp_debugger_perspective_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  GbpDebuggerPerspective *self = GBP_DEBUGGER_PERSPECTIVE (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_debugger_perspective_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  GbpDebuggerPerspective *self = GBP_DEBUGGER_PERSPECTIVE (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_debugger_perspective_class_init (GbpDebuggerPerspectiveClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gbp_debugger_perspective_finalize;
  object_class->get_property = gbp_debugger_perspective_get_property;
  object_class->set_property = gbp_debugger_perspective_set_property;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/builder/plugins/debugger-plugin/gbp-debugger-perspective.ui");
}

static void
gbp_debugger_perspective_init (GbpDebuggerPerspective *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
