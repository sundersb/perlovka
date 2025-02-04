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
#include "diff.h"

void
diff_horizontal (int *const data, size_t size)
{
  int *pt = data + 1;
  int *pend = data + size;
  int value = *data;
  int next;

  while (pt < pend)
    {
      next = *pt;
      *pt = next - value;
      value = next;
      ++pt;
    }
}

void
undiff_horizontal (int *const data, size_t size)
{
  int *pt = data + 1;
  int *pend = data + size;
  int *ps = data;

  while (pt < pend)
    {
      *pt += *ps;
      ++pt;
      ++ps;
    }
}

void
diff_vertical (int *const data, size_t size, size_t width)
{
  int *pt = data;
  int *ps = data + width;
  int *pend = data + size;

  while (ps < pend)
    {
      *pt -= *ps;
      ++ps;
      ++pt;
    }
}

void
undiff_vertical (int *const data, size_t size, size_t width)
{
  int *ps = data + size - 1;
  int *pt = ps - width;
  int *pend = data;

  while (pt >= pend)
    {
      *pt += *ps;
      --ps;
      --pt;
    }
}
