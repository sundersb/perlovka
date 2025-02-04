/*
    Perlovka - grain reduction filter
    Copyright (C) 2025 Alexander Belkov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef PLUGIN_UI
#define PLUGIN_UI

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "plugin.h"
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

gboolean show_perlovka_dialog (PerlovkaPluginSettings *options);

#pragma GCC diagnostic pop

#endif