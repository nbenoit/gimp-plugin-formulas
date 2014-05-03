/*
 * main.h
 *
 * This file is distributed as a part of the Formulas Rendering Plugin for the GIMP.
 * Copyright (c) 2005-2010 Nicolas BENOIT
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef __MAIN_H__
#define __MAIN_H__


#define FORMULA_STR_MAX_LEN 256


typedef struct
{
  gchar   str_red_chan   [FORMULA_STR_MAX_LEN+1];
  gchar   str_green_chan [FORMULA_STR_MAX_LEN+1];
  gchar   str_blue_chan  [FORMULA_STR_MAX_LEN+1];
  gchar   str_gray_chan  [FORMULA_STR_MAX_LEN+1];
  gchar   str_alpha_chan [FORMULA_STR_MAX_LEN+1];
  gboolean auto_update_preview;
} PlugInVals;


typedef struct
{
  GimpDrawable *drawable;
  gint32        drawable_id;
  gint32        width;
  gint32        height;
  gint32        size;
  gboolean      has_alpha;
  gboolean      is_rgb;
} PlugInDrawableVals;


/* default values */
extern const PlugInVals         default_vals;
extern const PlugInDrawableVals default_drawable_vals;


#endif
