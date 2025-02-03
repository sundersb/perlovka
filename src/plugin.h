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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <libintl.h>

#define PLUG_IN_PROC "plug-in-perlovka"
#define PLUGIN_NAME "perlovka"

/**
 * PerlovkaPluginSettings:
 * Static options for the plugin
 */
typedef struct
{
    /**
     * Maximum amount of denoize iterations for each channel
     */
    gint32 iterations_limit;

    /**
     * Maximal grain radius to detect
     */
    gint32 radius;

    /**
     * Grain detection grid: odd, even or both
     */
    gint32 grid;

    /**
     * Grain detection mode: soft (0), strict (1)
     */
    gint32 matching;

    /**
     * Grain resolve aggression: from minimal (0) to maximal (3)
     */
    gint32 resolver;

    /**
     * Compensate pixels around diagonals too
     */
    gboolean field_matching;
} PerlovkaPluginSettings;

#define _(String) gettext(String)

#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif

#endif