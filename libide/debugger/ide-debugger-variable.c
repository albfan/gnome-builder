/* ide-debugger-variable.c
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

#define G_LOG_DOMAIN "ide-debugger-variable"

#include "ide-debugger-variable.h"

G_DEFINE_ABSTRACT_TYPE (IdeDebuggerVariable, ide_debugger_variable, G_TYPE_OBJECT)

static void
ide_debugger_variable_class_init (IdeDebuggerVariableClass *klass)
{
}

static void
ide_debugger_variable_init (IdeDebuggerVariable *self)
{
}

/**
 * ide_debugger_variable_get_name:
 * @self: a #IdeDebuggerVariable
 *
 * Gets the name of the variable if available.
 *
 * Returns: (nullable) (transfer full): The variable name as a string.
 */
gchar *
ide_debugger_variable_get_name (IdeDebuggerVariable *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_VARIABLE (self), NULL);

  if (IDE_DEBUGGER_VARIABLE_GET_CLASS (self)->get_name)
    return IDE_DEBUGGER_VARIABLE_GET_CLASS (self)->get_name (self);

  return NULL;
}

/**
 * ide_debugger_variable_get_type_name:
 * @self: a #IdeDebuggerVariable
 *
 * Gets the type name of the variable as a string. This will be language
 * specific.
 *
 * Returns: (nullable) (transfer full): The type name as a string.
 */
gchar *
ide_debugger_variable_get_type_name (IdeDebuggerVariable *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_VARIABLE (self), NULL);

  if (IDE_DEBUGGER_VARIABLE_GET_CLASS (self)->get_type_name)
    return IDE_DEBUGGER_VARIABLE_GET_CLASS (self)->get_type_name (self);

  return NULL;
}

/**
 * ide_debugger_variable_get_value:
 * @self: a #IdeDebuggerVariable
 *
 * Gets the contents of the variable as a #GBytes.
 *
 * Returns: (nullable) (transfer full): A #GBytes or %NULL
 */
GBytes *
ide_debugger_variable_get_value (IdeDebuggerVariable *self)
{
  g_return_val_if_fail (IDE_IS_DEBUGGER_VARIABLE (self), NULL);

  if (IDE_DEBUGGER_VARIABLE_GET_CLASS (self)->get_value)
    return IDE_DEBUGGER_VARIABLE_GET_CLASS (self)->get_value (self);

  return NULL;
}
