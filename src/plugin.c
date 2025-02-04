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
#ifndef PERLOVKA_USE_GEGL
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <libgimp/gimp.h>

#include "perlovka.h"
#include "plugin.h"
#include "ui.h"

static void query (void);

static void run (const gchar *name, gint nparams, const GimpParam *param,
                 gint *nreturn_vals, GimpParam **return_vals);

const Grid default_grid = GRID_ODD;
const MatchMode default_matching = MATCHING_SOFT;
const ResolveMode default_resolver = RESOLVER_LARGEST_OF_MIN;
const int default_iterations = 10;
const int default_radius = 7;

typedef enum
{
  PERLOVKA_PARAM_RUN_MODE = 0,
  PERLOVKA_PARAM_IMAGE,
  PERLOVKA_PARAM_DRAWABLE,
  PERLOVKA_PARAM_ITERATIONS,
  PERLOVKA_PARAM_RADIUS,
  PERLOVKA_PARAM_GRID,
  PERLOVKA_PARAM_MATCHING,
  PERLOVKA_PARAM_RESOLVER,
} perlovka_param_index;

struct PerlovkaData
{
  gint channels_count;
  int color_count;
  gint width;
  gint height;
  size_t size;
  int *channels[4];
};

GimpPlugInInfo PLUG_IN_INFO = {
  NULL,
  NULL,
  query,
  run,
};

static PerlovkaPluginSettings settings
    = { default_iterations, default_radius,   default_grid,
        default_matching,   default_resolver, FALSE };

struct
{
  /**
   * Plugin run mode
   */
  GimpRunMode run_mode;

  /**
   * Static value: show progress bar (for interactive plugin start)
   */
  gboolean show_progress;

  /**
   * Static value: current channel-iteration tick size
   */
  double progress_tick;

  /**
   * Static value: current progress [0.0 - 1.0]
   */
  double progress_count;
} conditions;

MAIN ()

static void
query (void)
{
  gchar *help_path;
  gchar *help_uri;

  static GimpParamDef args[] = {
    { GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE, "image", "Input image" },
    { GIMP_PDB_DRAWABLE, "drawable", "Input drawable" },
    { GIMP_PDB_INT32, "iterations", "Maximum allowed iterations" },
    { GIMP_PDB_INT32, "radius", "Maximal grain radius to compensate" },
    { GIMP_PDB_INT32, "grid",
      "Grain detection grid to use: odd, even or both" },
    { GIMP_PDB_INT32, "matching", "Fields matching: soft or strict" },
    { GIMP_PDB_INT32, "resolver",
      "Compensation mode: minimal, least-of-max, largest-of-min, maximal" },
  };

  gimp_install_procedure (
      PLUG_IN_PROC, N_ ("Perlovka"), "Reduces granularity in photo films",
      "Alexander Belkov", "Alexander Belkov", "2025",
      N_ ("Per_lovka Degranulation..."), "RGB*, GRAY*", GIMP_PLUGIN,
      G_N_ELEMENTS (args), 0, args, NULL);

  gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Enhance");
}

void
fix_options ()
{
  if (settings.iterations_limit < 1 || settings.iterations_limit > 100)
    settings.iterations_limit = default_iterations;

  if (settings.radius < 1 || settings.radius > 50)
    settings.radius = default_radius;

  if (settings.grid < GRID_ODD || settings.grid > GRID_BOTH)
    settings.grid = default_grid;

  if (settings.matching < MATCHING_SOFT || settings.matching > MATCHING_STRICT)
    settings.matching = default_matching;

  if (settings.resolver < RESOLVER_MINIMAL
      || settings.resolver > RESOLVER_MAXIMAL)
    settings.resolver = default_resolver;
}

/**
 * Initialize static plugin_options with relevant params
 */
GimpPDBStatusType
load_params (gint nparams, const GimpParam *param)
{
  conditions.run_mode = param[PERLOVKA_PARAM_RUN_MODE].data.d_int32;

  switch (conditions.run_mode)
    {
    case GIMP_RUN_INTERACTIVE:
      gimp_get_data (PLUG_IN_PROC, &settings);

      if (!show_perlovka_dialog (&settings))
        return GIMP_PDB_CANCEL;

      conditions.show_progress = TRUE;
      break;

    case GIMP_RUN_NONINTERACTIVE:
      if (nparams < 8)
        return GIMP_PDB_CALLING_ERROR;

      settings.iterations_limit
          = param[PERLOVKA_PARAM_ITERATIONS].data.d_int32;
      settings.radius = param[PERLOVKA_PARAM_RADIUS].data.d_int32;
      settings.grid = param[PERLOVKA_PARAM_GRID].data.d_int32;
      settings.matching = param[PERLOVKA_PARAM_MATCHING].data.d_int32;
      settings.resolver = param[PERLOVKA_PARAM_RESOLVER].data.d_int32;
      conditions.show_progress = FALSE;

      break;

    case GIMP_RUN_WITH_LAST_VALS:
      gimp_get_data (PLUG_IN_PROC, &settings);
      conditions.show_progress = TRUE;
      break;
    }

  fix_options ();

  return GIMP_PDB_SUCCESS;
}

/**
 * Initializes PerlovkaData with the GimpDrawable
 */
GimpPDBStatusType
load_data (struct PerlovkaData *data, gint32 drawable_id)
{
#ifdef PERLOVKA_USE_GEGL
  GeglBuffer *buffer;
  const Babl *format;
#else
  GimpDrawable *drawable;
  GimpPixelRgn region;
#endif
  guchar *buf;
  guchar *ptr;
  int *cptr[4];
  int cindex;
  size_t index;
  gint channels;
  gint width;
  gint height;

  memset (data, 0, sizeof (data));

  channels = gimp_drawable_bpp (drawable_id);
  width = gimp_drawable_width (drawable_id);
  height = gimp_drawable_height (drawable_id);

  if (channels <= 0 || width <= 0 || height <= 0)
    return GIMP_PDB_EXECUTION_ERROR;

  data->channels_count = channels;
  data->color_count = channels <= 2 ? 1 : 3;

#ifdef PERLOVKA_USE_GEGL
  channels = data->color_count;
#endif

  data->width = width;
  data->height = height;
  data->size = width * height;

#ifdef PERLOVKA_USE_GEGL
  buffer = gimp_drawable_get_buffer (drawable_id);
  if (buffer == NULL)
    return GIMP_PDB_EXECUTION_ERROR;
#else
  drawable = gimp_drawable_get (drawable_id);
  if (drawable == NULL)
    return GIMP_PDB_EXECUTION_ERROR;
#endif

  buf = g_new (guchar, channels * data->size);
  if (buf == NULL)
    return GIMP_PDB_EXECUTION_ERROR;

  for (cindex = 0; cindex < channels; ++cindex)
    {
      cptr[cindex] = data->channels[cindex] = g_new (int, data->size);
    }

#ifdef PERLOVKA_USE_GEGL
  format = data->color_count == 1 ? babl_format ("Y' u8")
                                  : babl_format ("R'G'B' u8");

  gegl_buffer_get (buffer, GEGL_RECTANGLE (0, 0, data->width, data->height),
                   1.0, format, buf, channels * data->width, GEGL_ABYSS_CLAMP);
#else
  gimp_pixel_rgn_init (&region, drawable, 0, 0, width, height, FALSE, FALSE);

  gimp_pixel_rgn_get_rect (&region, buf, 0, 0, width, height);
#endif

  ptr = buf;
  for (index = 0; index < data->size; ++index)
    {
      for (cindex = 0; cindex < channels; ++cindex)
        {
          *cptr[cindex] = (int)*ptr;
          ++cptr[cindex];
          ++ptr;
        }
    }

  g_free (buf);

#ifdef PERLOVKA_USE_GEGL
  g_object_unref (buffer);
#else
  gimp_drawable_detach (drawable);
#endif

  return GIMP_PDB_SUCCESS;
}

/**
 * Free and null the PerlovkaData channels
 */
void
clean_data (struct PerlovkaData *data)
{
  int index;

  for (index = 0; index < data->channels_count; ++index)
    {
      if (data->channels[index])
        g_free (data->channels[index]);
    }

  memset (data->channels, 0, sizeof (data->channels));
}

/**
 * Bring the data items to unsigned byte diapasone
 */
void
normalize (int *data, size_t size)
{
  int amplitude;
  double q;

  int min = INT_MAX;
  int max = INT_MIN;
  int *end = data + size;
  int *ptr = data;

  while (ptr < end)
    {
      min = MIN (min, *ptr);
      max = MAX (max, *ptr);
      ++ptr;
    }

  amplitude = max - min;

  /*
      Can't do linear transformation in one go:
      if negative values have been discovered but the amplitude is
      well inside byte limits,  we may get channel's hystogram
      widening which is outside the filter's responsibility zone.
      We compensate data once from below and then (only if needed) from above.
  */

  if (min < 0)
    {
      ptr = data;
      while (ptr < end)
        {
          *ptr -= min;
          ++ptr;
        }
    }

  if (amplitude > UCHAR_MAX)
    {
      q = (double)UCHAR_MAX / (double)amplitude;

      ptr = data;
      while (ptr < end)
        {
          *ptr = (int)((double)*ptr * q);
          ++ptr;
        }
    }
}

/**
 * Tick progress in interactive mode
 */
void
do_progress ()
{
  conditions.progress_count += conditions.progress_tick;
  gimp_progress_update (conditions.progress_count);
}

/**
 * Run Perlovka for each channel in the PerlovkaData
 */
GimpPDBStatusType
denoize (struct PerlovkaData *data)
{
  PerlovkaOptions run_options;
  int index;
  void (*progress) () = NULL;

  run_options.width = data->width;
  run_options.height = data->height;
  run_options.radius = settings.radius;
  run_options.iterations = settings.iterations_limit;
  run_options.grid = settings.grid;
  run_options.matching = settings.matching;
  run_options.resolver = settings.resolver;
  run_options.field_matching = settings.field_matching;

  if (conditions.show_progress)
    {
      gimp_progress_init (_ ("Perlovka working..."));
      progress = do_progress;
      conditions.progress_tick
          = 1.0 / settings.iterations_limit / data->color_count;
      conditions.progress_count = 0.0;
    }

  for (index = 0; index < data->color_count; ++index)
    {
      run_options.data = data->channels[index];
      perlovka_denoize (&run_options, progress);
      normalize (data->channels[index], data->size);
    }

  gimp_progress_update (1.0);

  return GIMP_PDB_SUCCESS;
}

/**
 * Add layer to the image and populate it with denoized PerlovkaData
 */
GimpPDBStatusType
paste_result (gint32 image_id, struct PerlovkaData *data)
{
#ifdef PERLOVKA_USE_GEGL
  GeglBuffer *buffer;
  const Babl *format;
#else
  GimpDrawable *layer;
  GimpPixelRgn region;
#endif
  GimpImageType image_type;
  guchar *buf;
  guchar *ptr;
  gint32 layer_id;
  size_t index;
  int cindex, channels;
  int *cptr[4];

  buf = g_new (guchar, data->color_count * data->size);

  channels = data->color_count;
  for (cindex = 0; cindex < channels; ++cindex)
    cptr[cindex] = data->channels[cindex];

  ptr = buf;
  for (index = 0; index < data->size; ++index)
    {
      for (cindex = 0; cindex < channels; ++cindex)
        {
          *ptr = (guchar)*cptr[cindex];
          ++cptr[cindex];
          ++ptr;
        }
    }

  image_type = data->color_count == 1 ? GIMP_GRAY_IMAGE : GIMP_RGB_IMAGE;

  layer_id = gimp_layer_new (image_id, "Perlovka", data->width, data->height,
                             image_type, 100.0, GIMP_LAYER_MODE_NORMAL);

  gimp_image_insert_layer (image_id, layer_id, 0, 0);

#ifdef PERLOVKA_USE_GEGL
  buffer = gimp_drawable_get_shadow_buffer (layer_id);

  format = data->color_count == 1 ? babl_format ("Y' u8")
                                  : babl_format ("R'G'B' u8");

  gegl_buffer_set (buffer, GEGL_RECTANGLE (0, 0, data->width, data->height), 0,
                   format, buf, channels * data->width);
#else
  gimp_pixel_rgn_init (&region, layer, 0, 0, data->width, data->height, TRUE,
                       TRUE);

  gimp_pixel_rgn_set_rect (&region, buf, 0, 0, data->width, data->height);
#endif

  g_free (buf);

#ifdef PERLOVKA_USE_GEGL
  g_object_unref (buffer);
#else
  gimp_drawable_detach (layer);
#endif

  gimp_drawable_merge_shadow (layer_id, FALSE);

  // New layer's thumbnail would be black withoud this:
  gimp_drawable_update (layer_id, 0, 0, data->width, data->height);

  return GIMP_PDB_SUCCESS;
}

static void
run (const gchar *name, gint nparams, const GimpParam *param,
     gint *nreturn_vals, GimpParam **return_vals)
{
  static GimpParam values[1];
  struct PerlovkaData data;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;
  gint32 image_id;
  gint32 drawable_id;

#ifdef PERLOVKA_USE_GEGL
  gegl_init (NULL, NULL);
#endif

  values[0].type = GIMP_PDB_STATUS;

  *nreturn_vals = 1;
  *return_vals = values;

  status = load_params (nparams, param);
  if (status != GIMP_PDB_SUCCESS)
    {
      values[0].data.d_status = status;
      return;
    }

  status = load_data (&data, param[PERLOVKA_PARAM_DRAWABLE].data.d_drawable);
  if (status != GIMP_PDB_SUCCESS)
    {
      values[0].data.d_status = status;
      return;
    }

  image_id = param[PERLOVKA_PARAM_IMAGE].data.d_image;

  gimp_context_push ();
  gimp_image_undo_group_start (image_id);

  status = denoize (&data);
  if (status == GIMP_PDB_SUCCESS)
    {
      status = paste_result (image_id, &data);
    }

  gimp_image_undo_group_end (image_id);
  gimp_context_pop ();

  clean_data (&data);

  if (conditions.run_mode != GIMP_RUN_NONINTERACTIVE)
    gimp_displays_flush ();

  if (conditions.run_mode == GIMP_RUN_INTERACTIVE)
    gimp_set_data (PLUG_IN_PROC, &settings, sizeof (PerlovkaPluginSettings));

  values[0].data.d_status = status;
}

#ifndef PERLOVKA_USE_GEGL
#pragma GCC diagnostic pop
#endif
