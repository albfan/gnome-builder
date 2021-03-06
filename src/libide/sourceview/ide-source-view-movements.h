/* ide-source-view-movements.h
 *
 * Copyright © 2015 Christian Hergert <christian@hergert.me>
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

#pragma once

#include "sourceview/ide-source-view.h"

G_BEGIN_DECLS

void _ide_source_view_apply_movement (IdeSourceView         *source_view,
                                      IdeSourceViewMovement  movement,
                                      gboolean               extend_selection,
                                      gboolean               exclusive,
                                      gint                   count,
                                      GString               *command_str,
                                      gunichar               command,
                                      gunichar               modifier,
                                      gunichar               search_char,
                                      guint                 *target_column);

void _ide_source_view_select_inner   (IdeSourceView *self,
                                      gunichar       inner_left,
                                      gunichar       inner_right,
                                      gint           count,
                                      gboolean       exclusive,
                                      gboolean       string_mode);

void _ide_source_view_select_tag     (IdeSourceView *self,
                                      gint           count,
                                      gboolean       exclusive);

G_END_DECLS
