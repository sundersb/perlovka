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
#ifndef BALANCE_H
#define BALANCE_H

#include <stdbool.h>

/**
 * Signum relation of two values
 */
typedef enum
{
  /**
   * Both values are zero
   */
  BALANCE_ZERO,

  /**
   * One value is positive, another is negative
   */
  BALANCE_DIFFERENT,

  /**
   * Both values are positive
   */
  BALANCE_POSITIVE,

  /**
   * Both values are negative
   */
  BALANCE_NEGATIVE,

  /**
   * One value is positive, another is zero
   */
  BALANCE_SOFT_POSITIVE,

  /**
   * One value is negative, another is zero
   */
  BALANCE_SOFT_NEGATIVE,
} SignBalance;

/**
 * Get signum balance of two values
 */
static inline SignBalance
balance_of (int a, int b)
{
  if (a < 0)
    {
      if (b < 0)
        return BALANCE_NEGATIVE;
      else if (b == 0)
        return BALANCE_SOFT_NEGATIVE;
      else
        return BALANCE_DIFFERENT;
    }
  else if (a > 0)
    {
      if (b < 0)
        return BALANCE_DIFFERENT;
      else if (b == 0)
        return BALANCE_SOFT_POSITIVE;
      else
        return BALANCE_POSITIVE;
    }
  else
    {
      if (b < 0)
        return BALANCE_SOFT_NEGATIVE;
      else if (b == 0)
        return BALANCE_ZERO;
      else
        return BALANCE_SOFT_POSITIVE;
    }
}

static inline bool
is_positive (SignBalance balance)
{
  return balance == BALANCE_POSITIVE || balance == BALANCE_SOFT_POSITIVE;
}

static inline bool
is_negative (SignBalance balance)
{
  return balance == BALANCE_NEGATIVE || balance == BALANCE_SOFT_NEGATIVE;
}

/**
 * Two pairs are softly complement (one positive, another negative, one 'soft'
 * sign allowed)
 */
static inline bool
are_soft_complement (SignBalance lhs, SignBalance rhs)
{
  return (lhs == BALANCE_NEGATIVE && is_positive (rhs))
         || (lhs == BALANCE_SOFT_NEGATIVE && rhs == BALANCE_POSITIVE)
         || (lhs == BALANCE_POSITIVE && is_negative (rhs))
         || (lhs == BALANCE_SOFT_POSITIVE && rhs == BALANCE_NEGATIVE);
}

/**
 * Two signum balances are strictly inverse: one positive, one negative. No
 * 'soft' signs allowed
 */
static inline bool
are_strict_complement (SignBalance lhs, SignBalance rhs)
{
  return (lhs == BALANCE_NEGATIVE && rhs == BALANCE_POSITIVE)
         || (lhs == BALANCE_POSITIVE && rhs == BALANCE_NEGATIVE);
}

#endif
