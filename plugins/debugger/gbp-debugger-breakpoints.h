/* gbp-debugger-breakpoints.h
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

#ifndef GBP_DEBUGGER_BREAKPOINTS_H
#define GBP_DEBUGGER_BREAKPOINTS_H

#include <ide.h>

G_BEGIN_DECLS

#define GBP_TYPE_DEBUGGER_BREAKPOINTS (gbp_debugger_breakpoints_get_type())

G_DECLARE_FINAL_TYPE (GbpDebuggerBreakpoints, gbp_debugger_breakpoints, GBP, DEBUGGER_BREAKPOINTS, IdeObject)

typedef enum
{
  GBP_DEBUGGER_BREAK_NONE       = 0,
  GBP_DEBUGGER_BREAK_BREAKPOINT = 1 << 0,
  GBP_DEBUGGER_BREAK_COUNTPOINT = 1 << 1,
  GBP_DEBUGGER_BREAK_WATCHPOINT = 1 << 2,
} GbpDebuggerBreakType;

GFile                *gbp_debugger_breakpoints_get_file (GbpDebuggerBreakpoints *self);
void                  gbp_debugger_breakpoints_add      (GbpDebuggerBreakpoints *self,
                                                         guint                   line,
                                                         GbpDebuggerBreakType    break_type);
void                  gbp_debugger_breakpoints_remove   (GbpDebuggerBreakpoints *self,
                                                         guint                   line);
GbpDebuggerBreakType  gbp_debugger_breakpoints_lookup   (GbpDebuggerBreakpoints *self,
                                                         guint                   line);

G_END_DECLS

#endif /* GBP_DEBUGGER_BREAKPOINTS_H */
