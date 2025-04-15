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
#ifndef PERLOVKA_H
#define PERLOVKA_H

#include "solver.h"

/**
 * Color channel to denoize along with additional data and settings
 */
typedef struct
{
  /**
   * Array to denoize
   */
  int *data;

  /**
   * Actual amount of iterations done
   */
  int iterations_made;

  /**
   * Amount of compensations performed
   */
  size_t resolved;

  /**
   * Image width
   */
  size_t width;

  /**
   * Image height
   */
  size_t height;

  /**
   * Supposed maximal radius of the grain
   */
  int radius;

  /**
   * Iterations limit
   */
  int iterations;

  /**
   * Grain detection grid
   */
  Grid grid;

  /**
   * Fields matching mode
   */
  MatchMode matching;

  /**
   * Grain compensation mode
   */
  ResolveMode resolver;

  /**
   * Compensate pixels around diagonals too
   */
  bool field_matching;
  
  /**
   * Progress callback called after each iteration
   */
  void (*progress) (void *context);

  /**
   * Progress context for the callback
   */
  void *context;
} PerlovkaOptions;

/**
 * Run Perlovka denoize on data presented by `options`
 * @options Data to denoize
 * @tick Callback to use after each iteration
 */
void perlovka_denoize (PerlovkaOptions *options);

#endif