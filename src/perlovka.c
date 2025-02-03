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

#include <string.h>

#include "perlovka.h"
#include "diff.h"
#include "solver.h"

void perlovka_denoize(PerlovkaOptions *options, void (*tick)())
{
    PSolver solver;
    size_t size = options->width * options->height;

    int max_height = options->height - options->radius - 1;
    int max_width = options->width - options->radius - 1;

    size_t resolved = 0;
    int iteration = 0;
    int solved_in_one_go;
    int position;
    int x, y;
    int grid_index;
    int solver_index;

    diff_horizontal(options->data, size);
    diff_vertical(options->data, size, options->width);

    solver = build_solver(options->width,
                          options->radius,
                          options->grid,
                          options->matching,
                          options->resolver,
                          options->field_matching);

    do
    {
        solved_in_one_go = 0;

        for (y = options->radius; y < max_height; ++y)
        {
            position = y * options->width + options->radius;

            for (int x = options->radius; x < max_width; ++x)
            {
                ++position;
                solved_in_one_go += apply_solver(solver, options->data, position);
            }
        }
        resolved += solved_in_one_go;

        if (tick)
            tick();
    } while (++iteration < options->iterations && solved_in_one_go > 0);

    clean_solver(solver);

    undiff_vertical(options->data, size, options->width);
    undiff_horizontal(options->data, size);

    options->iterations_made = iteration;
    options->resolved = resolved;
}
