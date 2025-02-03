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

#ifndef POSITION_H
#define POSITION_H

#include "value.h"

/**
 * Pixel pair positions in the array
 */
typedef struct
{
    /**
     * First pixel position
     */
    int a;

    /**
     * Second pixel position
     */
    int b;
} ArmPosition;

typedef ArmPosition *const PPosition;
typedef ArmPosition const *const PCPosition;

/** 
 * Build positions
 */
void init_position(PPosition position, int a, int b);

/**
 * Build `target` as `source` translated to `delta`
 */
void translated_position(PCPosition source, PPosition target, int delta);

/**
 * Get pixel pairs according to the position
 */
void get_values(int const *const data, PCPosition position, PValue value);

/**
 * Modify diff values
 */
void apply_delta(int *const data, PCPosition position, PValue value, int delta);

#endif