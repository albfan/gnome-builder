/* ide-project-files.h
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

#include "ide-version-macros.h"

#include "projects/ide-project-file.h"
#include "projects/ide-project-item.h"

G_BEGIN_DECLS

#define IDE_TYPE_PROJECT_FILES (ide_project_files_get_type())

G_DECLARE_FINAL_TYPE (IdeProjectFiles, ide_project_files, IDE, PROJECT_FILES,
                      IdeProjectItem)

struct _IdeProjectFiles
{
  IdeProjectItem parent_instance;
};

IDE_AVAILABLE_IN_ALL
IdeFile        *ide_project_files_get_file_for_path (IdeProjectFiles *self,
                                                     const gchar     *path);
IDE_AVAILABLE_IN_ALL
void            ide_project_files_add_file          (IdeProjectFiles *self,
                                                     IdeProjectFile  *file);
IDE_AVAILABLE_IN_ALL
IdeProjectItem *ide_project_files_find_file         (IdeProjectFiles *self,
                                                     GFile           *file);

G_END_DECLS
