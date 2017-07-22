/* ide-debugger-frame.h
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

#ifndef IDE_DEBUGGER_FRAME_H
#define IDE_DEBUGGER_FRAME_H

#include <gio/gio.h>

#include "debugger/ide-debugger-variable.h"

G_BEGIN_DECLS

#define IDE_TYPE_DEBUGGER_FRAME (ide_debugger_frame_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeDebuggerFrame, ide_debugger_frame, IDE, DEBUGGER_FRAME, GObject)

struct _IdeDebuggerFrameClass
{
  GObjectClass parent_class;

  guint      (*get_index)     (IdeDebuggerFrame *self);
  gchar     *(*get_function)  (IdeDebuggerFrame *self);
  GPtrArray *(*get_arguments) (IdeDebuggerFrame *self);
  gchar     *(*get_location)  (IdeDebuggerFrame *self);
  gchar     *(*get_address)   (IdeDebuggerFrame *self);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

guint      ide_debugger_frame_get_index     (IdeDebuggerFrame *self);
gchar     *ide_debugger_frame_get_address   (IdeDebuggerFrame *self);
gchar     *ide_debugger_frame_get_location  (IdeDebuggerFrame *self);
gchar     *ide_debugger_frame_get_function  (IdeDebuggerFrame *self);
GPtrArray *ide_debugger_frame_get_arguments (IdeDebuggerFrame *self);

G_END_DECLS

#endif /* IDE_DEBUGGER_FRAME_H */
