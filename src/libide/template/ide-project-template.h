/* ide-project-template.h
 *
 * Copyright © 2015 Christian Hergert <chergert@redhat.com>
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

#include <gtk/gtk.h>

#include "ide-version-macros.h"

G_BEGIN_DECLS

#define IDE_TYPE_PROJECT_TEMPLATE (ide_project_template_get_type())

G_DECLARE_INTERFACE (IdeProjectTemplate, ide_project_template, IDE, PROJECT_TEMPLATE, GObject)

struct _IdeProjectTemplateInterface
{
  GTypeInterface parent;

  gchar      *(*get_id)          (IdeProjectTemplate   *self);
  gchar      *(*get_name)        (IdeProjectTemplate   *self);
  gchar      *(*get_description) (IdeProjectTemplate   *self);
  GtkWidget  *(*get_widget)      (IdeProjectTemplate   *self);
  gchar     **(*get_languages)   (IdeProjectTemplate   *self);
  gchar      *(*get_icon_name)   (IdeProjectTemplate   *self);
  void        (*expand_async)    (IdeProjectTemplate   *self,
                                  GHashTable           *params,
                                  GCancellable         *cancellable,
                                  GAsyncReadyCallback   callback,
                                  gpointer              user_data);
  gboolean    (*expand_finish)   (IdeProjectTemplate   *self,
                                  GAsyncResult         *result,
                                  GError              **error);
};

IDE_AVAILABLE_IN_ALL
gchar      *ide_project_template_get_id          (IdeProjectTemplate  *self);
IDE_AVAILABLE_IN_ALL
gchar      *ide_project_template_get_name        (IdeProjectTemplate  *self);
IDE_AVAILABLE_IN_ALL
gchar      *ide_project_template_get_description (IdeProjectTemplate  *self);
IDE_AVAILABLE_IN_ALL
GtkWidget  *ide_project_template_get_widget      (IdeProjectTemplate  *self);
IDE_AVAILABLE_IN_ALL
gchar     **ide_project_template_get_languages   (IdeProjectTemplate  *self);
IDE_AVAILABLE_IN_ALL
gchar      *ide_project_template_get_icon_name   (IdeProjectTemplate  *self);
IDE_AVAILABLE_IN_ALL
void        ide_project_template_expand_async    (IdeProjectTemplate   *self,
                                                  GHashTable           *params,
                                                  GCancellable         *cancellable,
                                                  GAsyncReadyCallback   callback,
                                                  gpointer              user_data);
IDE_AVAILABLE_IN_ALL
gboolean    ide_project_template_expand_finish   (IdeProjectTemplate   *self,
                                                  GAsyncResult         *result,
                                                  GError              **error);

G_END_DECLS
