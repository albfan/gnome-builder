/* ide-transfer-button.h
 *
 * Copyright © 2017 Christian Hergert <chergert@redhat.com>
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

#include <dazzle.h>

#include "ide-version-macros.h"

#include "transfers/ide-transfer.h"

G_BEGIN_DECLS

#define IDE_TYPE_TRANSFER_BUTTON (ide_transfer_button_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeTransferButton, ide_transfer_button, IDE, TRANSFER_BUTTON, DzlProgressButton)

struct _IdeTransferButtonClass
{
  DzlProgressButtonClass parent_class;

  /*< private >*/
  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

IDE_AVAILABLE_IN_ALL
GtkWidget *ide_transfer_button_new (IdeTransfer *transfer);

G_END_DECLS
