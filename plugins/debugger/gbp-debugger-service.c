/* gbp-debugger-service.c
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

#define G_LOG_DOMAIN "gbp-debugger-service"

#include "gbp-debugger-service.h"

struct _GbpDebuggerService
{
  IdeObject parent_instance;
};

enum {
  PROP_0,
  N_PROPS
};

enum {
  BREAKPOINT_ADDED,
  BREAKPOINT_REMOVED,
  N_SIGNALS
};

static void
service_iface_init (IdeServiceInterface *iface)
{
}

G_DEFINE_TYPE_EXTENDED (GbpDebuggerService, gbp_debugger_service, IDE_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (IDE_TYPE_SERVICE, service_iface_init))

static GParamSpec *properties [N_PROPS];

static void
gbp_debugger_service_finalize (GObject *object)
{
  G_OBJECT_CLASS (gbp_debugger_service_parent_class)->finalize (object);
}

static void
gbp_debugger_service_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  GbpDebuggerService *self = GBP_DEBUGGER_SERVICE (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_debugger_service_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  GbpDebuggerService *self = GBP_DEBUGGER_SERVICE (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_debugger_service_class_init (GbpDebuggerServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gbp_debugger_service_finalize;
  object_class->get_property = gbp_debugger_service_get_property;
  object_class->set_property = gbp_debugger_service_set_property;
}

static void
gbp_debugger_service_init (GbpDebuggerService *self)
{
}

/**
 * gbp_debugger_service_get_breakpoints_for_file:
 *
 * Gets a #GbpDebuggerBreakpoints for a given #GFile.
 *
 * This is useful for gutter renderers that need very fast access to whether or
 * not a line has a breakpoint on it.
 *
 * Returns: (transfer full): A #GbpDebuggerBreakpoints
 */
GbpDebuggerBreakpoints *
gbp_debugger_service_get_breakpoints_for_file (GbpDebuggerService *self,
                                               GFile              *file)
{
  GbpDebuggerBreakpoints *ret;
  IdeContext *context;

  g_return_val_if_fail (GBP_IS_DEBUGGER_SERVICE (self), NULL);
  g_return_val_if_fail (G_IS_FILE (file), NULL);

  context = ide_object_get_context (IDE_OBJECT (self));

  ret = g_object_new (GBP_TYPE_DEBUGGER_BREAKPOINTS,
                      "context", context,
                      "file", file,
                      NULL);

  /* TODO: Load breakpoints from current IDebugger */

  return ret;
}

void
gbp_debugger_service_begin (GbpDebuggerService *self,
                            IdeRunner          *runner)
{
  g_return_if_fail (GBP_IS_DEBUGGER_SERVICE (self));
  g_return_if_fail (IDE_IS_RUNNER (runner));

  g_warning ("TODO: hook in GDB into runner execution");

}

void
gbp_debugger_service_toggle_breakpoint (GbpDebuggerService *self,
                                        GFile              *file,
                                        guint               line)
{
  g_return_if_fail (GBP_IS_DEBUGGER_SERVICE (self));
  g_return_if_fail (G_IS_FILE (file));

  g_warning ("TODO: toggle breakpoint for debugger instance");

}
