/* ide-debugger-variable.h
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

#ifndef IDE_DEBUGGER_VARIABLE_H
#define IDE_DEBUGGER_VARIABLE_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define IDE_TYPE_DEBUGGER_VARIABLE (ide_debugger_variable_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeDebuggerVariable, ide_debugger_variable, IDE, DEBUGGER_VARIABLE, GObject)

struct _IdeDebuggerVariableClass
{
  GObjectClass parent_class;

  gchar  *(*get_name)      (IdeDebuggerVariable *self);
  gchar  *(*get_type_name) (IdeDebuggerVariable *self);
  GBytes *(*get_value)     (IdeDebuggerVariable *self);

  /* TODO: How do we want to do complex field types? */

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

gchar  *ide_debugger_variable_get_name      (IdeDebuggerVariable *self);
gchar  *ide_debugger_variable_get_type_name (IdeDebuggerVariable *self);
GBytes *ide_debugger_variable_get_value     (IdeDebuggerVariable *self);

G_END_DECLS

#endif /* IDE_DEBUGGER_VARIABLE_H */
