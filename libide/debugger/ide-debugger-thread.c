/* ide-debugger-thread.c
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

#include "ide-debugger-thread.h"

typedef struct
{
  gchar *id;
  gchar *name;
} IdeDebuggerThreadPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (IdeDebuggerThread, ide_debugger_thread, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_ID,
  PROP_NAME,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

IdeDebuggerThread *
ide_debugger_thread_new (void)
{
  return g_object_new (IDE_TYPE_DEBUGGER_THREAD, NULL);
}

static void
ide_debugger_thread_finalize (GObject *object)
{
  IdeDebuggerThread *self = (IdeDebuggerThread *)object;
  IdeDebuggerThreadPrivate *priv = ide_debugger_thread_get_instance_private (self);

  g_clear_pointer (&priv->id, g_free);
  g_clear_pointer (&priv->name, g_free);

  G_OBJECT_CLASS (ide_debugger_thread_parent_class)->finalize (object);
}

static void
ide_debugger_thread_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  IdeDebuggerThread *self = IDE_DEBUGGER_THREAD (object);

  switch (prop_id)
    {
    case PROP_ID:
      g_value_set_string (value, ide_debugger_thread_get_id (self));
      break;

    case PROP_NAME:
      g_value_set_string (value, ide_debugger_thread_get_name (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_debugger_thread_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  IdeDebuggerThread *self = IDE_DEBUGGER_THREAD (object);

  switch (prop_id)
    {
    case PROP_ID:
      ide_debugger_thread_set_id (self, g_value_get_string (value));
      break;

    case PROP_NAME:
      ide_debugger_thread_set_name (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_debugger_thread_class_init (IdeDebuggerThreadClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_debugger_thread_finalize;
  object_class->get_property = ide_debugger_thread_get_property;
  object_class->set_property = ide_debugger_thread_set_property;

  /**
   * IdeDebuggerThread:id:
   *
   * The "id" property contains the identifier of the thread within the debugger.
   * This may not match the id of the thread to the operating system, but simply
   * for internal access within the debugger.
   *
   * It generally starts from 1 and is monotonically increasing.
   */
  properties [PROP_ID] =
    g_param_spec_string ("id",
                         "Id",
                         "The id of the thread",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  /**
   * IdeDebuggerThread:name:
   *
   * If the process has specified a name for the thread and the debugger has access to it,
   * this is where that thread name should be stored.
   *
   * This allows the UI to display more than just "Thread 2" when referring to the thread.
   */
  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The name of the thread",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
ide_debugger_thread_init (IdeDebuggerThread *self)
{
}

const gchar *
ide_debugger_thread_get_id (IdeDebuggerThread *self)
{
  IdeDebuggerThreadPrivate *priv = ide_debugger_thread_get_instance_private (self);

  g_return_val_if_fail (IDE_IS_DEBUGGER_THREAD (self), NULL);

  return priv->id;
}

void
ide_debugger_thread_set_id (IdeDebuggerThread *self,
                            const gchar       *id)
{
  IdeDebuggerThreadPrivate *priv = ide_debugger_thread_get_instance_private (self);

  g_return_if_fail (IDE_IS_DEBUGGER_THREAD (self));

  if (g_strcmp0 (priv->id, id) != 0)
    {
      g_free (priv->id);
      priv->id = g_strdup (id);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ID]);
    }
}

const gchar *
ide_debugger_thread_get_name (IdeDebuggerThread *self)
{
  IdeDebuggerThreadPrivate *priv = ide_debugger_thread_get_instance_private (self);

  g_return_val_if_fail (IDE_IS_DEBUGGER_THREAD (self), NULL);

  return priv->name;
}

void
ide_debugger_thread_set_name (IdeDebuggerThread *self,
                              const gchar       *name)
{
  IdeDebuggerThreadPrivate *priv = ide_debugger_thread_get_instance_private (self);

  g_return_if_fail (IDE_IS_DEBUGGER_THREAD (self));

  if (g_strcmp0 (priv->name, name) != 0)
    {
      g_free (priv->name);
      priv->name = g_strdup (name);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_NAME]);
    }
}
