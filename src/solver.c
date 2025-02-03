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

#include <stdlib.h>
#include <string.h>

#include "solver.h"

typedef struct ArmBox ArmBox;
typedef ArmBox *PBox;

typedef struct ArmBox
{
    ArmPosition first;
    ArmPosition second;
    PBox skip_to;
} ArmBox;

typedef struct
{
    bool (*match)(PCValue lhs, PCValue rhs);
    int (*get_delta)(PCValue lhs, PCValue rhs);
    int n_boxes;
    ArmBox boxes[];
} Solvers;

void clean_solver(PSolver solver)
{
    if (solver)
        free(solver);
}

void set_solver_modes(Solvers *solvers, MatchMode matching, ResolveMode resolver)
{
    solvers->match = matching == MATCHING_SOFT
                         ? match_soft
                         : match_strict;

    if (resolver == RESOLVER_MINIMAL)
        solvers->get_delta = get_minimal_delta;
    else if (resolver == RESOLVER_LEAST_OF_MAX)
        solvers->get_delta = get_least_of_max_delta;
    else if (resolver == RESOLVER_LARGEST_OF_MIN)
        solvers->get_delta = get_largest_of_min_delta;
    else if (resolver == RESOLVER_MAXIMAL)
        solvers->get_delta = get_maximal_delta;
    else
        solvers->get_delta = get_minimal_delta;
}

PBox make_cross(PBox box, PBox next_grid, int width, int radius, bool odd)
{
    int row = radius * width;

    int row_top;
    int row_bottom = -row;
    int delta_left = -radius;
    int delta_right;

    if (odd)
    {
        // A |   | B
        //     x
        // C |   | D
        row_top = row;
        delta_right = radius;
    }
    else
    {
        // A | B(x)
        // C | D
        row_top = row - width;
        delta_right = radius - 1;
    }

    init_position(&box->first, row_top + delta_left, row_bottom + delta_right);
    init_position(&box->second, row_top + delta_right, row_bottom + delta_left);
    box->skip_to = next_grid;

    return ++box;
}

PBox make_box(PBox box, PBox next_grid, int width, int radius, bool odd)
{
    int row = radius * width;
    int other_row;

    int row_top;
    int row_bottom = -row;
    int delta_left = -radius;
    int delta_right;

    if (odd)
    {
        row_top = row;
        delta_right = radius;
    }
    else
    {
        row_top = row - width;
        delta_right = radius - 1;
    }

    init_position(&box->first, row_top + delta_left, row_bottom + delta_right);
    init_position(&box->second, row_top + delta_right, row_bottom + delta_left);

    box->skip_to = next_grid;
    ++box;

    for (int count = 1; count < radius; ++count)
    {
        // Pair by the rows
        init_position(&box->first, row_top + delta_left + count, row_bottom + delta_right - count);
        init_position(&box->second, row_top + delta_right - count, row_bottom + delta_left + count);
        box->skip_to = box + 1;
        ++box;

        other_row = width * count;

        // Pair by the columns
        init_position(&box->first, row_top - other_row + delta_left, row_bottom + other_row + delta_right);
        init_position(&box->second, row_top - other_row + delta_right, row_bottom + other_row + delta_left);
        box->skip_to = box + 1;
        ++box;
    }

    return box;
}

PBox make_boxes(PBox box, PBox next_grid, int width, int radius, bool field_matching, bool odd)
{
    int index;

    if (field_matching)
    {
        for (index = 0; index < radius; ++index)
            box = make_box(box, next_grid, width, index + 1, odd);
    }
    else
    {
        for (index = 0; index < radius; ++index)
            box = make_cross(box, next_grid, width, index + 1, odd);
    }

    return box;
}

PSolver
build_solver(int width, int radius,
             Grid grid,
             MatchMode matching,
             ResolveMode resolver,
             bool field_matching)
{
    Solvers *solvers;
    PBox next_grid;
    PBox box;
    int n_grid;
    int n_boxes;
    int size;

    n_grid = field_matching
                 ? radius * radius
                 : radius;

    n_boxes = grid == GRID_BOTH
                  ? n_grid * 2
                  : n_grid;

    size = sizeof(Solvers) + sizeof(ArmBox) * n_boxes;

    solvers = (Solvers *)malloc(size);
    memset(solvers, 0, size);

    solvers->n_boxes = n_boxes;

    set_solver_modes(solvers, matching, resolver);

    box = &solvers->boxes[0];

    next_grid = grid == GRID_BOTH
                    ? box + n_grid
                    : NULL;

    if (grid == GRID_ODD || grid == GRID_BOTH)
        box = make_boxes(box, next_grid, width, radius, field_matching, true);

    if (grid == GRID_EVEN || grid == GRID_BOTH)
        box = make_boxes(box, NULL, width, radius, field_matching, false);

    --box;

    box->skip_to = NULL;

    return solvers;
}

int apply_solver(PSolver solver, int *const data, int position)
{
    ArmPosition first;
    ArmPosition second;
    ArmValue first_value;
    ArmValue second_value;
    PBox box;
    PBox pend;
    Solvers *solvers;
    int delta;
    int result = 0;

    solvers = (Solvers *)solver;

    box = &solvers->boxes[0];
    pend = box + solvers->n_boxes;

    while (box && box < pend)
    {
        translated_position(&box->first, &first, position);
        translated_position(&box->second, &second, position);

        get_values(data, &first, &first_value);
        get_values(data, &second, &second_value);

        if (solvers->match(&first_value, &second_value))
        {
            delta = solvers->get_delta(&first_value, &second_value);

            apply_delta(data, &first, &first_value, delta);
            apply_delta(data, &second, &second_value, delta);

            ++box;
            ++result;
        }
        else
        {
            box = box->skip_to;
        }
    }

    return result;
}
