/* gbp-debugger-breakpoints.c
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

#define G_LOG_DOMAIN "gbp-debugger-breakpoints"

#include "gbp-debugger-breakpoints.h"
#include "gbp-debugger-service.h"

/**
 * SECTION:gbp-debugger-breakpoints
 * @title: GbpDebuggerBreakpoints
 *
 * This object is used to track breakpoints within a certain file.
 * This is necessary so that we can draw the gutter for breakpoints
 * very fast by reducing our breakpoint lookup cost to simply a line
 * number in a hashtable.
 *
 * To do this, the #GbpDebuggerBreakpoints object will track changes
 * to breakpoints (and watchpoints) in the GbpDebuggerService. If the
 * breakpoint is part to the current #GbpDebuggerBreakpoints:file, then
 * it will cache the value. The #GbpDebuggerGutterRenderer is assigned
 * a #GbpDebuggerBreakpoints which it will use to determine what icon
 * to render on the gutter.
 */

struct _GbpDebuggerBreakpoints
{
  IdeObject   parent_instance;
  GHashTable *cache;
  GFile      *file;
};

enum {
  PROP_0,
  PROP_FILE,
  N_PROPS
};

enum {
  CHANGED,
  N_SIGNALS
};

G_DEFINE_TYPE (GbpDebuggerBreakpoints, gbp_debugger_breakpoints, IDE_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static void
on_breakpoint_added (GbpDebuggerBreakpoints *self,
                     GFile                  *file,
                     guint                   line,
                     GbpDebuggerBreakType    break_type,
                     GbpDebuggerService     *service)
{
  GbpDebuggerBreakType type;
  gpointer key = GUINT_TO_POINTER (line);

  g_assert (GBP_IS_DEBUGGER_BREAKPOINTS (self));
  g_assert (G_IS_FILE (file));

  if (!g_file_equal (file, self->file))
    return;

  type = GPOINTER_TO_UINT (g_hash_table_lookup (self->cache, key)) | break_type;
  g_hash_table_insert (self->cache, key, GUINT_TO_POINTER (type));

  g_signal_emit (self, signals [CHANGED], 0);
}

static void
on_breakpoint_removed (GbpDebuggerBreakpoints *self,
                       GFile                  *file,
                       guint                   line,
                       GbpDebuggerBreakType    break_type,
                       GbpDebuggerService     *service)
{
  GbpDebuggerBreakType type;
  gpointer key = GUINT_TO_POINTER (line);

  g_assert (GBP_IS_DEBUGGER_BREAKPOINTS (self));
  g_assert (G_IS_FILE (file));

  if (!g_file_equal (file, self->file))
    return;

  type = GPOINTER_TO_UINT (g_hash_table_lookup (self->cache, key));
  type = type & ~break_type;

  if (type == 0)
    g_hash_table_remove (self->cache, key);
  else
    g_hash_table_replace (self->cache, key, GUINT_TO_POINTER (type));

  g_signal_emit (self, signals [CHANGED], 0);
}

static void
gbp_debugger_breakpoints_constructed (GObject *object)
{
  GbpDebuggerBreakpoints *self = (GbpDebuggerBreakpoints *)object;
  GbpDebuggerService *service;
  IdeContext *context;

  g_assert (GBP_IS_DEBUGGER_BREAKPOINTS (self));

  G_OBJECT_CLASS (gbp_debugger_breakpoints_parent_class)->constructed (object);

  context = ide_object_get_context (IDE_OBJECT (self));
  service = ide_context_get_service_typed (context, GBP_TYPE_DEBUGGER_SERVICE);

  g_signal_connect_object (service,
                           "breakpoint-added",
                           G_CALLBACK (on_breakpoint_added),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (service,
                           "breakpoint-removed",
                           G_CALLBACK (on_breakpoint_removed),
                           self,
                           G_CONNECT_SWAPPED);
}

static void
gbp_debugger_breakpoints_finalize (GObject *object)
{
  GbpDebuggerBreakpoints *self = (GbpDebuggerBreakpoints *)object;

  g_clear_object (&self->file);
  g_clear_pointer (&self->cache, g_hash_table_unref);

  G_OBJECT_CLASS (gbp_debugger_breakpoints_parent_class)->finalize (object);
}

static void
gbp_debugger_breakpoints_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  GbpDebuggerBreakpoints *self = GBP_DEBUGGER_BREAKPOINTS (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, self->file);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_debugger_breakpoints_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  GbpDebuggerBreakpoints *self = GBP_DEBUGGER_BREAKPOINTS (object);

  switch (prop_id)
    {
    case PROP_FILE:
      self->file = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_debugger_breakpoints_class_init (GbpDebuggerBreakpointsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gbp_debugger_breakpoints_constructed;
  object_class->finalize = gbp_debugger_breakpoints_finalize;
  object_class->get_property = gbp_debugger_breakpoints_get_property;
  object_class->set_property = gbp_debugger_breakpoints_set_property;

  properties [PROP_FILE] =
    g_param_spec_object ("file",
                         "File",
                         "File",
                         G_TYPE_FILE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  /**
   * GbpDebuggerBreakpoints::changed:
   *
   * This signal is emitted when a breakpoint has changed with in the file
   * indicated by GbpDebuggerBreakpoints:file.
   */
  signals [CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
gbp_debugger_breakpoints_init (GbpDebuggerBreakpoints *self)
{
  self->cache = g_hash_table_new (NULL, NULL);
}

/**
 * gbp_debugger_breakpoints_get_file:
 *
 * Returns: (transfer none): A #GFile.
 */
GFile *
gbp_debugger_breakpoints_get_file (GbpDebuggerBreakpoints *self)
{
  g_return_val_if_fail (GBP_IS_DEBUGGER_BREAKPOINTS (self), NULL);

  return self->file;
}

void
gbp_debugger_breakpoints_add (GbpDebuggerBreakpoints *self,
                              guint                   line,
                              GbpDebuggerBreakType    break_type)
{
  IDE_ENTRY;

  g_assert (GBP_IS_DEBUGGER_BREAKPOINTS (self));
  g_assert (line > 0);
  g_assert (break_type > GBP_DEBUGGER_BREAK_NONE);

  /*
   * TODO: Propagate request for breakpoint via debugger service.
   *       Instead, for testing, we will just add the breakpoint
   *       locally to our cache. However, a real implementation
   *       should possibly set a pending bit on the entry and the
   *       remove the pending bit once it has been added by the
   *       debugger (since that could involve a round trip over
   *       IPC and we don't want to show a full breakpoint before
   *       its actually available).
   */

  g_hash_table_insert (self->cache, GUINT_TO_POINTER (line), GUINT_TO_POINTER (break_type));
  g_signal_emit (self, signals [CHANGED], 0);

  IDE_EXIT;
}

void
gbp_debugger_breakpoints_remove (GbpDebuggerBreakpoints *self,
                                 guint                   line)
{
  IDE_ENTRY;

  g_assert (GBP_IS_DEBUGGER_BREAKPOINTS (self));
  g_assert (line > 0);

  /*
   * TODO: Propagate request to remove breakpoint via debugger service.
   *       For testing, we can just remove the debugger from cache.
   */

  g_hash_table_remove (self->cache, GUINT_TO_POINTER (line));
  g_signal_emit (self, signals [CHANGED], 0);

  IDE_EXIT;
}

GbpDebuggerBreakType
gbp_debugger_breakpoints_lookup (GbpDebuggerBreakpoints *self,
                                 guint                   line)
{
  return GPOINTER_TO_UINT (g_hash_table_lookup (self->cache, GUINT_TO_POINTER (line)));
}
