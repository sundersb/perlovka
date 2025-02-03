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

#ifndef SOLVER_H
#define SOLVER_H

#include "position.h"

typedef enum
{
    GRID_ODD,
    GRID_EVEN,
    GRID_BOTH,
} Grid;

/**
 * Fields matching mode
 */
typedef enum
{
    /**
     * Soft: one pixel in the two pairs allowed to have zero difference
     */
    MATCHING_SOFT = 0,

    /**
     * Strict mode: both diffs in one pair must be negative while in the other one positive
     */
    MATCHING_STRICT
} MatchMode;

/**
 * Grain compensation mode
 */
typedef enum
{
    /**
     * Closest to zero diff counts for compensation
     */
    RESOLVER_MINIMAL,

    /**
     * Minimal amplitude of pair-max taken for compensation
     */
    RESOLVER_LEAST_OF_MAX,

    /**
     * Maximal value between pair-mins used
     */
    RESOLVER_LARGEST_OF_MIN,

    /**
     * Furtherst from zero diff is used
     */
    RESOLVER_MAXIMAL
} ResolveMode;

typedef void *PSolver;

PSolver build_solver(int width, int radius,
                      Grid grid,
                      MatchMode matching,
                      ResolveMode resolver,
                      bool field_matching);

void clean_solver(PSolver solver);

int apply_solver(PSolver solver, int *const data, int position);

#endif