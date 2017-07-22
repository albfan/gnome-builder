/* ide-debugger-stack.h
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

#ifndef IDE_DEBUGGER_STACK_H
#define IDE_DEBUGGER_STACK_H

#include <gio/gio.h>

#include "debugger/ide-debugger-frame.h"

G_BEGIN_DECLS

#define IDE_TYPE_DEBUGGER_STACK (ide_debugger_stack_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeDebuggerStack, ide_debugger_stack, IDE, DEBUGGER_STACK, GObject)

struct _IdeDebuggerStackClass
{
  GObjectClass parent;

  guint             (*get_n_frames) (IdeDebuggerStack *self);
  IdeDebuggerFrame *(*get_frame)    (IdeDebuggerStack *self,
                                     guint             position);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

guint             ide_debugger_stack_get_n_frames (IdeDebuggerStack *self);
IdeDebuggerFrame *ide_debugger_stack_get_frame    (IdeDebuggerStack *self,
                                                   guint             position);

G_END_DECLS

#endif /* IDE_DEBUGGER_STACK_H */
