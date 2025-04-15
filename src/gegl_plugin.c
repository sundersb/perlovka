/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 Alexander Belkov
 */

// #include "config.h"
#define GETTEXT_PACKAGE "perlovka-1.0"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_perlovka_grid)
  enum_value (GEGL_PERLOVKA_GRID_ODD, "odd", N_("Odd"))
  enum_value (GEGL_PERLOVKA_GRID_EVEN, "even", N_("Even"))
  enum_value (GEGL_PERLOVKA_GRID_BOTH, "both", N_("Both"))
enum_end (GeglPerlovkaGrid)

enum_start (gegl_perlovka_matching)
  enum_value (GEGL_PERLOVKA_MATCHING_STRICT, "strict", N_("Strict"))
  enum_value (GEGL_PERLOVKA_MATCHING_SOFT, "soft", N_("Soft"))
enum_end (GeglPerlovkaMatching)

enum_start (gegl_perlovka_resolver)
  enum_value (GEGL_PERLOVKA_RESOLVER_MINIMAL, "minimal", N_("Minimal"))
  enum_value (GEGL_PERLOVKA_RESOLVER_LEAST, "leastOfMax", N_("LeastOfMax"))
  enum_value (GEGL_PERLOVKA_RESOLVER_LARGEST, "largestOfMin", N_("LargestOfMin"))
  enum_value (GEGL_PERLOVKA_RESOLVER_MAXIMAL, "maximal", N_("Maximal"))
enum_end (GeglPerlovkaResolver)

property_int (radius, _("Radius"), 5)
  description (_("Maximal grain radius to compensate"))
  value_range (1, 100)
  ui_range (1, 100)
  ui_meta ("unit", "pixel-distance")

property_int (iterations, _("Iterations"), 5)
  description (_("Maximum allowed iterations"))
  value_range (1, 100)
  ui_range (1, 100)

property_enum (grid, _("Grid"),
              GeglPerlovkaGrid, gegl_perlovka_grid,
              GEGL_PERLOVKA_GRID_ODD)
  description (_("Grain detection grid"))

property_enum (matching, _("Matching mode"),
              GeglPerlovkaMatching, gegl_perlovka_matching,
              GEGL_PERLOVKA_MATCHING_STRICT)
  description (_("Fields matching mode"))

property_enum (resolver, _("Resolve mode"),
              GeglPerlovkaResolver, gegl_perlovka_resolver,
              GEGL_PERLOVKA_RESOLVER_LEAST)
  description (_("Compensation greediness"))

property_boolean (field_matching, _("Field matching"), FALSE)
  description (_("Study all pixels for given radius"))

#else

#define GEGL_OP_AREA_FILTER
#define GEGL_OP_NAME perlovka
#define GEGL_OP_C_SOURCE src/gegl_plugin.c

#include "gegl-op.h"

#include "balance.h"
#include "diff.c"
#include "diff.h"
#include "perlovka.c"
#include "perlovka.h"
#include "position.c"
#include "position.h"
#include "solver.c"
#include "solver.h"
#include "value.c"
#include "value.h"

char *format_code = "CIE Lab u16";

typedef struct
{
  GeglOperation *operation;
  double tick_size;
  double position;
} PositionContext;

static void
prepare (GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space (operation, "input");
  const Babl *format = babl_format_with_space (format_code, space);

  GeglOperationAreaFilter *area = GEGL_OPERATION_AREA_FILTER (operation);

  area->left = area->right = area->top = area->bottom =
      GEGL_PROPERTIES (operation)->radius;

  gegl_operation_set_format (operation, "input", format);
  gegl_operation_set_format (operation, "output", format);
}

static void
clamp (int *data, gint size)
{
  int value;

  int *pend = data + size;
  while (data < pend)
    {
      value = *data;
      if (value < 1)
        *data = 0;
      else if (value > USHRT_MAX)
        *data = USHRT_MAX;
      ++data;
    }
}

void
read_options (GeglOperation *operation, PerlovkaOptions *options)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);

  options->radius = o->radius;
  options->iterations = o->iterations;
  options->field_matching = o->field_matching;
  
  switch (o->grid)
  {
    case GEGL_PERLOVKA_GRID_ODD: options->grid = GRID_ODD; break;
    case GEGL_PERLOVKA_GRID_EVEN: options->grid = GRID_EVEN; break;
    case GEGL_PERLOVKA_GRID_BOTH: options->grid = GRID_BOTH; break;
  }
  
  switch (o->matching)
  {
    case GEGL_PERLOVKA_MATCHING_STRICT: options->matching = MATCHING_STRICT; break;
    case GEGL_PERLOVKA_MATCHING_SOFT: options->matching = MATCHING_SOFT; break;
  }
  
  switch (o->resolver)
  {
    case GEGL_PERLOVKA_RESOLVER_MINIMAL: options->resolver = RESOLVER_MINIMAL; break;
    case GEGL_PERLOVKA_RESOLVER_LEAST: options->resolver = RESOLVER_LEAST_OF_MAX; break;
    case GEGL_PERLOVKA_RESOLVER_LARGEST: options->resolver = RESOLVER_LARGEST_OF_MIN; break;
    case GEGL_PERLOVKA_RESOLVER_MAXIMAL: options->resolver = RESOLVER_MAXIMAL; break;
  }
}

static void
move_and_shrink(guint16 *buffer, gint radius, gint components, gint width, gint height)
{
  gint source_stride = width * components;
  gint dest_stride = (width - radius * 2) * components;
  gint radius_stride = radius * components;

  guint16 *psource = buffer + radius * source_stride;
  guint16 *pend = buffer + (height - radius) * source_stride;
  guint16 *psource_end;

  while (psource < pend)
    {
      psource += radius_stride;
      psource_end = psource + dest_stride;
      while (psource < psource_end)
      {
        for (int i = components; i > 0; --i)
          {
            *buffer = *psource;
            ++buffer;
            ++psource;
          }
      }
      psource += radius_stride;
    }
}

static void
progress (void *pc)
{
  PositionContext *context = pc;
  context->position += context->tick_size;
  gegl_operation_progress (context->operation, context->position, _("Perlovka working..."));
}

static gboolean
process (GeglOperation *operation, GeglBuffer *input, GeglBuffer *output,
         const GeglRectangle *roi, gint level)
{

  PerlovkaOptions options;
  GeglRectangle compute = gegl_operation_get_required_for_output (operation, "input", roi);
  PositionContext position_context;
  const Babl *format = gegl_operation_get_format (operation, "input");

  guint16 *buffer, *pbuffer;
  int *perlovka_data, *pdata, *pend;

  gint components = babl_format_get_n_components (format);
  gint width = compute.width;
  gint height = compute.height;
  gint size = width * height;

  position_context.operation = operation;
  position_context.tick_size = 1 / components;
  position_context.position = -position_context.tick_size;

  options.width = width;
  options.height = height;
  options.progress = progress;
  options.context = &position_context;

  read_options (operation, &options);
  progress (&position_context);

  buffer = g_new (guint16, components * size);
  perlovka_data = g_new (int, size);
  pend = perlovka_data + size;

  gegl_buffer_get (input, &compute, 1.0, format, buffer, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  pbuffer = buffer;
  pdata = perlovka_data;
  while (pdata < pend)
    {
      *pdata = (int)*pbuffer;
      ++pdata;
      pbuffer += components;
    }

  options.data = perlovka_data;
  perlovka_denoize (&options);
  clamp (perlovka_data, size);

  pbuffer = buffer;
  pdata = perlovka_data;
  while (pdata < pend)
    {
      *pbuffer = (guint16)*pdata;
      ++pdata;
      pbuffer += components;
    }

  move_and_shrink(buffer, options.radius, components, width, height);

  gegl_buffer_set (output, roi, 0, format, buffer, GEGL_AUTO_ROWSTRIDE);

  g_free (perlovka_data);
  g_free (buffer);

  gegl_operation_progress (operation, 1.0, _("Perlovka working..."));

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;
  GeglOperationFilterClass *filter_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  filter_class = GEGL_OPERATION_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "title", _("Perlovka Degranulation"),
    "name", "sundersb:perlovka",
    "categories", "enhance:noise-reduction",
    "description", _("Reduces granularity in photo films"),
    "gimp:menu-path", "<Image>/Filters/Enhance",
    "gimp:menu-label", _("GEGL Perlovka"),
    NULL);
}

#endif