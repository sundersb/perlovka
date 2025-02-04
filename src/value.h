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
#ifndef VALUE_H
#define VALUE_H

#include "balance.h"
#include <stdbool.h>

/**
 * Pixel diff pairs
 */
typedef struct
{
  /**
   * Diffs signum in the pair
   */
  SignBalance balance;

  /**
   * First diff value
   */
  int a;

  /**
   * Second diff value
   */
  int b;
} ArmValue;

typedef ArmValue *const PValue;
typedef ArmValue const *const PCValue;

/**
 * Build non-balanced diff pair
 */
void init_value (PValue value, int a, int b);

/**
 * Build diff pair and calculate signum balance
 */
void init_balanced_value (PValue value, int a, int b);

/**
 * Get furtherst from zero value from the pair
 */
int get_value_maximum (PCValue value);

/**
 * Get closest to zero value from the pair
 */
int get_value_minimum (PCValue value);

/**
 * Get minimal applicable compensation between pairs
 * (resolve_mode.RESOLVER_MINIMAL)
 */
int get_minimal_delta (PCValue lhs, PCValue rhs);

/**
 * Get moderate compensation between pairs (resolve_mode.RESOLVER_LEAST_OF_MAX)
 */
int get_least_of_max_delta (PCValue lhs, PCValue rhs);

/**
 * Get moderate compensation between pairs
 * (resolve_mode.RESOLVER_LARGEST_OF_MIN)
 */
int get_largest_of_min_delta (PCValue lhs, PCValue rhs);

/**
 * Get maximal compensation between pairs (resolve_mode.RESOLVER_MAXIMAL)
 */
int get_maximal_delta (PCValue lhs, PCValue rhs);

/**
 * Match pairs according to match_mode.MATCHING_STRICT
 */
bool match_strict (PCValue lhs, PCValue rhs);

/**
 * Match pairs according to match_mode.MATCHING_SOFT
 */
bool match_soft (PCValue lhs, PCValue rhs);

/**
 * Apply delta for the diffs pair
 */
void fix_value (PValue value, int delta);

#endif