/* ide-debugger-frame.c
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

#define G_LOG_DOMAIN "ide-debugger-frame"

#include "ide-debugger-frame.h"

G_DEFINE_ABSTRACT_TYPE (IdeDebuggerFrame, ide_debugger_frame, G_TYPE_OBJECT)

static void
ide_debugger_frame_class_init (IdeDebuggerFrameClass *klass)
{
}

static void
ide_debugger_frame_init (IdeDebuggerFrame *self)
{
}

guint
ide_debugger_frame_get_index (IdeDebuggerFrame *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_FRAME (self), 0);

  if (IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_index)
    return IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_index (self);

  return 0;
}

gchar *
ide_debugger_frame_get_address (IdeDebuggerFrame *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_FRAME (self), 0);

  if (IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_address)
    return IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_address (self);

  return NULL;
}

gchar *
ide_debugger_frame_get_location (IdeDebuggerFrame *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_FRAME (self), 0);

  if (IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_location)
    return IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_location (self);

  return NULL;
}

gchar *
ide_debugger_frame_get_function (IdeDebuggerFrame *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_FRAME (self), 0);

  if (IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_function)
    return IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_function (self);

  return NULL;
}

/**
 * ide_debugger_frame_get_arguments:
 * @self: a #IdeDebuggerFrame
 *
 * Gets the arguments passed to the function as #IdeDebuggerVariable.
 *
 * Returns: (transfer container) (element-type Ide.DebuggerVariable): The
 *   arguments for the function call in this frame.
 */
GPtrArray *
ide_debugger_frame_get_arguments (IdeDebuggerFrame *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_FRAME (self), 0);

  if (IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_arguments)
    return IDE_DEBUGGER_FRAME_GET_CLASS (self)->get_arguments (self);

  return NULL;
}
