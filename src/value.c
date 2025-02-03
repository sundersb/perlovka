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

#include "value.h"
#include "balance.h"

static inline int min(int a, int b)
{
    return a < b ? a : b;
}

static inline int max(int a, int b)
{
    return a > b ? a : b;
}

void init_value(PValue value, int a, int b)
{
    value->a = a;
    value->b = b;
}

void init_balanced_value(PValue value, int a, int b)
{
    value->a = a;
    value->b = b;
    value->balance = balance_of(a, b);
}

int get_value_maximum(PCValue value)
{
    if (is_positive(value->balance))
        return max(value->a, value->b);
    else
        return -min(value->a, value->b);
}

int get_value_minimum(PCValue value)
{
    if (value->balance == BALANCE_POSITIVE)
        return min(value->a, value->b);
    else if (value->balance == BALANCE_NEGATIVE)
        return -max(value->a, value->b);
    else if (value->balance == BALANCE_SOFT_NEGATIVE)
        return -(value->a == 0 ? value->b : value->a);
    else if (value->balance == BALANCE_SOFT_POSITIVE)
        return value->a == 0? value->b : value->a;
    else
        return 0;
}

int get_minimal_delta(PCValue lhs, PCValue rhs)
{
    return min(get_value_minimum(lhs), get_value_minimum(rhs));
}

int get_least_of_max_delta(PCValue lhs, PCValue rhs)
{
    return min(get_value_maximum(lhs), get_value_maximum(rhs));
}

int get_largest_of_min_delta(PCValue lhs, PCValue rhs)
{
    return max(get_value_minimum(lhs), get_value_minimum(rhs));
}

int get_maximal_delta(PCValue lhs, PCValue rhs)
{
    return max(get_value_maximum(lhs), get_value_maximum(rhs));
}

bool match_strict(PCValue lhs, PCValue rhs)
{
    return are_strict_complement(lhs->balance, rhs->balance);
}

bool match_soft(PCValue lhs, PCValue rhs)
{
    return are_soft_complement(lhs->balance, rhs->balance);
}

void fix_value(PValue value, int delta)
{
    if (is_positive(value->balance))
    {
        value->a -= delta;
        value->b -= delta;
    }
    else
    {
        value->a += delta;
        value->b += delta;
    }
}
