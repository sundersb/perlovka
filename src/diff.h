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
#ifndef DIFF_H
#define DIFF_H

#include <stddef.h>

/**
 * Build horizontal diffs in place: diff value is current pixel minus the one
 * on the left
 */
void diff_horizontal (int *const data, size_t size);

/**
 * Restore original image from the horizontal diffs
 */
void undiff_horizontal (int *const data, size_t size);

/**
 * Differentiate image vertically
 */
void diff_vertical (int *const data, size_t size, size_t width);

/**
 * Reverse vertical differetiation
 */
void undiff_vertical (int *const data, size_t size, size_t width);

#endif