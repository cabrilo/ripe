// Copyright (C) 2008  Maksim Sipos <msipos@mailc.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "clib.h"

void array_init2(Array* arr, uint sz)
{
  arr->alloc_size = 2;
  arr->size = 0;
  arr->data = mem_malloc(2 * sz);
}

void array_deinit(Array* arr)
{
  mem_free(arr->data);
}

Array* array_new2(uint sz)
{
  Array* p = mem_new(Array);
  array_init2(p, sz);
  return p;
}

void array_delete(Array* arr)
{
  array_deinit(arr);
  mem_free(arr);
}

void array_prepend(Array* arr, uint sz, void* el)
{
  array_expand(arr, sz);
  memmove(arr->data + sz, arr->data, sz * arr->size);
  memcpy(arr->data, el, sz);
  arr->size++;
}

void array_expand(Array* arr, uint sz)
{
  if (arr->alloc_size > arr->size) return;
  arr->alloc_size *= 2;
  arr->data = mem_realloc(arr->data, sz * arr->alloc_size);
}

void array_clear(Array* arr)
{
  arr->size = 0;
}

