/* ide-debugger-stack.c
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

#define G_LOG_DOMAIN "ide-debugger-stack"

#include "ide-debugger-stack.h"

static void list_model_iface_init (GListModelInterface *iface);

G_DEFINE_ABSTRACT_TYPE_WITH_CODE (IdeDebuggerStack, ide_debugger_stack, G_TYPE_OBJECT,
                                  G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_model_iface_init))

static void
ide_debugger_stack_class_init (IdeDebuggerStackClass *klass)
{
}

static void
ide_debugger_stack_init (IdeDebuggerStack *self)
{
}

/**
 * ide_debugger_stack_get_frame:
 * @self: An #IdeDebuggerStack
 * @position: the position within the stack, starting from 0
 *
 * It is an error to call this function with a position that is outside the
 * range of the available frames in this stack.
 *
 * The index starts from 0.
 *
 * See ide_debugger_stack_get_n_frames() for the number of frames within
 * this stack.
 *
 * Returns: (transfer full): An #IdeDebuggerFrame.
 */
IdeDebuggerFrame *
ide_debugger_stack_get_frame (IdeDebuggerStack *self,
                              guint             position)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_STACK (self), NULL);

#ifdef IDE_ENABLE_TRACE
  g_assert_cmpint (position, <, ide_debugger_stack_get_n_frames (self));
#endif

  return IDE_DEBUGGER_STACK_GET_CLASS (self)->get_frame (self, position);
}

/**
 * ide_debugger_stack_get_n_frames:
 * @self: An #IdeDebuggerStack
 *
 * Gets the number of frames within this stack trace.
 *
 * Returns: The number of frames in the stack trace.
 */
guint
ide_debugger_stack_get_n_frames (IdeDebuggerStack *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_STACK (self), 0);

  return IDE_DEBUGGER_STACK_GET_CLASS (self)->get_n_frames (self);
}

static guint
ide_debugger_stack_get_n_items (GListModel *model)
{
  return ide_debugger_stack_get_n_frames (IDE_DEBUGGER_STACK (model));
}

static GType
ide_debugger_stack_get_item_type (GListModel *model)
{
  return IDE_TYPE_DEBUGGER_FRAME;
}

static gpointer
ide_debugger_stack_get_item (GListModel *model,
                             guint       position)
{
  return ide_debugger_stack_get_frame (IDE_DEBUGGER_STACK (model), position);
}

static void
list_model_iface_init (GListModelInterface *iface)
{
  iface->get_n_items = ide_debugger_stack_get_n_items;
  iface->get_item_type = ide_debugger_stack_get_item_type;
  iface->get_item = ide_debugger_stack_get_item;
}
