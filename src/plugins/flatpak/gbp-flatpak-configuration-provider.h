/* gbp-flatpak-configuration-provider.h
 *
 * Copyright © 2016 Matthew Leeds <mleeds@redhat.com>
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

#include <ide.h>

G_BEGIN_DECLS

#define GBP_TYPE_FLATPAK_CONFIGURATION_PROVIDER (gbp_flatpak_configuration_provider_get_type())

G_DECLARE_FINAL_TYPE (GbpFlatpakConfigurationProvider, gbp_flatpak_configuration_provider, GBP, FLATPAK_CONFIGURATION_PROVIDER, GObject)

G_END_DECLS
