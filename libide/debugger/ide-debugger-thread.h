/* ide-debugger-thread.h
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

#ifndef IDE_DEBUGGER_THREAD_H
#define IDE_DEBUGGER_THREAD_H

#include <glib-object.h>

G_BEGIN_DECLS

#define IDE_TYPE_DEBUGGER_THREAD (ide_debugger_thread_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeDebuggerThread, ide_debugger_thread, IDE, DEBUGGER_THREAD, GObject)

struct _IdeDebuggerThreadClass
{
  GObjectClass parent;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

IdeDebuggerThread *ide_debugger_thread_new      (void);
const gchar       *ide_debugger_thread_get_id   (IdeDebuggerThread *self);
void               ide_debugger_thread_set_id   (IdeDebuggerThread *self,
                                                 const gchar       *id);
const gchar       *ide_debugger_thread_get_name (IdeDebuggerThread *self);
void               ide_debugger_thread_set_name (IdeDebuggerThread *self,
                                                 const gchar       *name);

G_END_DECLS

#endif /* IDE_DEBUGGER_THREAD_H */
