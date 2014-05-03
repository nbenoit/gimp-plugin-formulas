/*
 * render.c
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


#include "config.h"
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "main.h"
#include "error.h"
#include "formula.h"
#include "maths_val.h"
#include "render.h"
#include "plugin-intl.h"


/*
 * Frees memory.
 */
static void
destroy_formulas ( PlugInDrawableVals *dvals, 
                   FORMULA            *red,
                   FORMULA            *green,
                   FORMULA            *blue,
                   FORMULA            *gray,
                   FORMULA            *alpha )
{
  if ( dvals->is_rgb )
    {
      formula_destroy ( red );
      formula_destroy ( green );
      formula_destroy ( blue );
    }
  else
    formula_destroy ( gray );

  if ( dvals->has_alpha )
    formula_destroy ( alpha );
}


/*
 * Get the red, gray, green, blue, alpha channel value at (x,y) coords
 */
guchar *in_img_buf;
gint nb_chan;
gint width;
gint height;
gdouble aspect_ratio_w;
gdouble aspect_ratio_h;
gint row_stride;

#define RED   0
#define GRAY  0
#define GREEN 1
#define BLUE  2
#define ALPHA() (nb_chan-1)

gint current_chan;

/* assignment to the x coord */
#define ASSIGN_X(X)  {                                \
    x = (gint) (X / aspect_ratio_w);                  \
    if ( x < 0 )                                      \
      x = 0;                                          \
    else if ( x >= width )                            \
      x = width - 1; }

/* assignment to the y coord */
#define ASSIGN_Y(Y)   {                               \
    y = (gint) (Y / aspect_ratio_h);                  \
    if ( y < 0 )                                      \
      y = 0;                                          \
    else if ( y >= height )                           \
      y = height - 1; }

/* read a channel's value */
#define READ_CHAN(C) ((gdouble) *( in_img_buf + x*nb_chan + y*row_stride + C))

/* red */
gdouble
get_red_at ( gdouble xoff,
             gdouble yoff )
{
  gint x, y;

  ASSIGN_X ( xoff );
  ASSIGN_Y ( yoff );

  if ( nb_chan < 3 ) /* gray image */
    return READ_CHAN(GRAY);
  else
    return READ_CHAN(RED);
}

/* gray */
gdouble
get_gray_at ( gdouble xoff,
              gdouble yoff )
{
  gint x, y;

  ASSIGN_X ( xoff );
  ASSIGN_Y ( yoff );

  if ( nb_chan > 2 ) /* rgb image */
    return (READ_CHAN(RED) + READ_CHAN(GREEN) + READ_CHAN(BLUE)) / 3.0;
  else
    return READ_CHAN(GRAY);
}

/* green */
gdouble
get_green_at ( gdouble xoff,
               gdouble yoff )
{
  gint x, y;

  ASSIGN_X ( xoff );
  ASSIGN_Y ( yoff );

  if ( nb_chan < 3 ) /* gray image */
    return READ_CHAN(GRAY);
  else
    return READ_CHAN(GREEN);
}

/* blue */
gdouble
get_blue_at ( gdouble xoff,
              gdouble yoff )
{
  gint x, y;

  ASSIGN_X ( xoff );
  ASSIGN_Y ( yoff );

  if ( nb_chan < 3 ) /* gray image */
    return READ_CHAN(GRAY);
  else
    return READ_CHAN(BLUE);
}

/* alpha */
gdouble
get_alpha_at ( gdouble xoff,
               gdouble yoff )
{
  gint x, y;

  ASSIGN_X ( xoff );
  ASSIGN_Y ( yoff );

  if ( nb_chan & 1 ) /* no alpha if nb_chan == 1 or nb_chan == 3 */
    return 255.0;

  return READ_CHAN(ALPHA());
}

/* red, green or blue channel according to current channel */
gdouble
get_rgb_at ( gdouble xoff,
             gdouble yoff )
{
  gint x, y;

  ASSIGN_X ( xoff );
  ASSIGN_Y ( yoff );

  return READ_CHAN(current_chan);
}


/*
 * Renders the formulas.
 */
void 
render_to_gimpdrawable ( const gint32        image_ID,
                         PlugInDrawableVals *dvals,
                         PlugInVals         *vals )
{
  guchar *in_image, *in_ptr;
  guchar *out_image, *out_ptr;
  gint x, y;
  GimpPixelRgn in_pr;
  GimpPixelRgn out_pr;
  FORMULA *red_chan = NULL;
  FORMULA *green_chan = NULL;
  FORMULA *blue_chan = NULL;
  FORMULA *gray_chan = NULL;
  FORMULA *alpha_chan = NULL;

  in_image = g_new ( guchar, dvals->size );
  out_image = g_new ( guchar, dvals->size );
  in_ptr = in_image;
  in_img_buf = in_image;
  out_ptr = out_image;

  aspect_ratio_w = 1.0;
  aspect_ratio_h = 1.0;

  /* formulas building */
  if ( dvals->is_rgb )
    {
      if ((red_chan = formula_new(vals->str_red_chan, TRUE)) == NULL)
        {
          error ( NULL, _("Unable to evaluate the formula of the red channel.") );
          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }

      if ((green_chan = formula_new(vals->str_green_chan, TRUE)) == NULL)
        {
          error ( NULL, _("Unable to evaluate the formula of the green channel.") );
          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }

      if ((blue_chan = formula_new(vals->str_blue_chan, TRUE)) == NULL)
        {
          error ( NULL, _("Unable to evaluate the formula of the blue channel.") );
          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }
    }
  else
    {
      if ((gray_chan = formula_new(vals->str_gray_chan, TRUE)) == NULL)
        {
          error ( NULL, _("Unable to evaluate the formula of the gray channel.") );
          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }
    }

  if ( dvals->has_alpha )
    {
      if ((alpha_chan = formula_new(vals->str_alpha_chan, TRUE)) == NULL)
        {
          error ( NULL, _("Unable to evaluate the formula of the alpha channel.") );
          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }
    }

  gimp_progress_init ( _("Formulas' Rendering...") );
  gimp_pixel_rgn_init ( &in_pr, dvals->drawable, 0, 0, dvals->width, dvals->height, FALSE, FALSE );
  gimp_pixel_rgn_get_rect ( &in_pr, in_image, 0, 0, dvals->width, dvals->height );

  width = dvals->width;
  values_set_w ( (gdouble) dvals->width );

  height = dvals->height;
  values_set_h ( (gdouble) dvals->height );

  /* we separate the loops, so we don't waste time testing for alpha and RGB */
  if (dvals->is_rgb)
    {
      gdouble gray;

      /* formula optimisation */
      formula_precalc ( red_chan );
      formula_precalc ( green_chan );
      formula_precalc ( blue_chan );

      if ( dvals->has_alpha )
        {
          /* RGBA image */
          nb_chan = 4;
          row_stride = width * 4;

          formula_precalc (  alpha_chan );

          for ( y=0; y<dvals->height; ++y )
            {
              values_set_y((gdouble) y);

              for ( x=0; x<dvals->width; ++x )
                {
                  gray = 0.0;
                  values_set_x((gdouble) x);
                  coords_set_polar_from_cartesian ( (x-(dvals->width>>1)), (y-(dvals->height>>1)) );
                  current_chan = RED;
                  *out_ptr = (guchar) (formula_execute(red_chan)); ++out_ptr;
                  current_chan = GREEN;
                  *out_ptr = (guchar) (formula_execute(green_chan)); ++out_ptr;
                  current_chan = BLUE;
                  *out_ptr = (guchar) (formula_execute(blue_chan)); ++out_ptr;
                  current_chan = ALPHA();
                  *out_ptr = (guchar) (formula_execute(alpha_chan)); ++out_ptr;
                }

              if ( (y & 8) == 0 )
                gimp_progress_update((double) y / (double) dvals->height);
            }
        }
      else
        {
          /* RGB image */
          nb_chan = 3;
          row_stride = width * 3;

          for ( y=0; y<dvals->height; ++y )
            {
              values_set_y((double) y);

              for ( x=0; x<dvals->width; ++x )
                {
                  gray = 0.0;
                  values_set_x((gdouble) x);
                  coords_set_polar_from_cartesian ( (x-(dvals->width>>1)), (y-(dvals->height>>1)) );
                  current_chan = RED;
                  *out_ptr = (guchar) (formula_execute(red_chan)); ++out_ptr;
                  current_chan = GREEN;
                  *out_ptr = (guchar) (formula_execute(green_chan)); ++out_ptr;
                  current_chan = BLUE;
                  *out_ptr = (guchar) (formula_execute(blue_chan)); ++out_ptr;
                }

              if ( (y & 8) == 0 )
                gimp_progress_update((double) y / (double) dvals->height);
            }
        }
    }
  else
    {
      /* formula optimisation */
      formula_precalc ( gray_chan );

      if ( dvals->has_alpha )
        {
          /* GrayA image */
          nb_chan = 2;
          row_stride = width * 2;

          formula_precalc (  alpha_chan );

          for ( y=0; y<dvals->height; ++y )
            {
              values_set_y((double) y);

              for ( x=0; x<dvals->width; ++x )
                {
                  values_set_x((gdouble) x);
                  coords_set_polar_from_cartesian ( (x-(dvals->width>>1)), -(y-(dvals->height>>1)) );
                  current_chan = GRAY;
                  *out_ptr = (guchar) (formula_execute(gray_chan)); ++out_ptr;
                  current_chan = ALPHA();
                  *out_ptr = (guchar) (formula_execute(alpha_chan)); ++out_ptr;
                }

              if ( (y & 8) == 0 )
                gimp_progress_update((double) y / (double) dvals->height);
            }
        }
      else
        {
          /* Gray image */
          nb_chan = 1;
          row_stride = width;

          for( y=0; y<dvals->height; ++y )
            {
              values_set_y((double) y);

              for ( x=0; x<dvals->width; ++x )
                {
                  values_set_x ( (gdouble) x );
                  coords_set_polar_from_cartesian ( (x-(dvals->width>>1)), -(y-(dvals->height>>1)) );
                  current_chan = GRAY;
                  *out_ptr = (guchar) (formula_execute(gray_chan)); ++out_ptr;
                }

              if ( (y & 8) == 0 )
                gimp_progress_update((double) y / (double) dvals->height);
            }
        }
    }

  gimp_pixel_rgn_init ( &out_pr, dvals->drawable, 0, 0, dvals->width, dvals->height, TRUE, TRUE );
  gimp_pixel_rgn_set_rect ( &out_pr, out_image, 0, 0, dvals->width, dvals->height );
  gimp_drawable_flush ( dvals->drawable );
  gimp_drawable_merge_shadow ( dvals->drawable->drawable_id, TRUE );
  gimp_drawable_update ( dvals->drawable->drawable_id, 0, 0, dvals->width, dvals->height );
  gimp_progress_update ( 1.0 );
#if GIMP_MINOR_VERSION >= 4
  gimp_progress_end ( );
#endif

  /* cleaning ... */
  destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
  g_free ( out_image );
  g_free ( in_image );
}


/*
 * Renders the formulas (preview).
 */
void
render_to_pixbuf ( GdkPixbuf          *pixbuf,
                   GdkPixbuf          *original,
                   PlugInDrawableVals *dvals,
                   PlugInVals         *vals,
                   const gdouble      caspect_ratio_w,
                   const gdouble      caspect_ratio_h,
                   const gboolean     report_errors )
{
  FORMULA *red_chan = NULL;
  FORMULA *green_chan = NULL;
  FORMULA *blue_chan = NULL;
  FORMULA *gray_chan = NULL;
  FORMULA *alpha_chan = NULL;
  guchar  *pixbuf_pixels, *row_ptr, *ptr;
  gdouble  x, y;
  gint     img_width, img_height;

  pixbuf_pixels = gdk_pixbuf_get_pixels ( pixbuf );
  in_img_buf = gdk_pixbuf_get_pixels ( original );

  aspect_ratio_w = caspect_ratio_w;
  aspect_ratio_h = caspect_ratio_h;
  row_stride = gdk_pixbuf_get_rowstride ( pixbuf );

  /* formulas building */
  if ( dvals->is_rgb )
    {
      if ((red_chan = formula_new(vals->str_red_chan, report_errors)) == NULL)
        {
          if ( report_errors )
            error ( NULL, _("Unable to evaluate the formula of the red channel.") );

          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }

      if ((green_chan = formula_new(vals->str_green_chan, report_errors)) == NULL)
        {
          if ( report_errors )
            error ( NULL, _("Unable to evaluate the formula of the green channel.") );

          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }

      if ((blue_chan = formula_new(vals->str_blue_chan, report_errors)) == NULL)
        {
          if ( report_errors )
            error ( NULL, _("Unable to evaluate the formula of the blue channel.") );

          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }
    }
  else
    {
      if ((gray_chan = formula_new(vals->str_gray_chan, report_errors)) == NULL)
        {
          if ( report_errors )
            error ( NULL, _("Unable to evaluate the formula of the gray channel.") );

          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }
    }

  if ( dvals->has_alpha )
    {
      if ((alpha_chan = formula_new(vals->str_alpha_chan, report_errors)) == NULL)
        {
          if ( report_errors )
            error(NULL, _("Unable to evaluate the formula of the alpha channel."));

          destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
          return ;
        }
    }

  /* we set the 'w' and 'h' values according to the original picture */
  img_width = ( gint )( caspect_ratio_w * (gdouble) dvals->width );
  img_height = ( gint ) ( caspect_ratio_h * (gdouble) dvals->height );
  values_set_w ( img_width );
  values_set_h ( img_height );

  /* width and height are relative to the preview pixbuf */
  width = dvals->width;
  height = dvals->height;

  nb_chan = 3;

  /* rendering ... */
  if ( dvals->is_rgb )
    {
      const gint col_size = dvals->height*row_stride;
      const gint row_size = dvals->width*3*sizeof(guchar);

      for ( row_ptr=pixbuf_pixels, y=0.0;
            row_ptr<(pixbuf_pixels+col_size);
            row_ptr+=row_stride, y+=caspect_ratio_h )
        {
          values_set_y ( y );

          for ( ptr=row_ptr, x=0.0;
                ptr<(row_ptr+row_size);
                x+=caspect_ratio_w )
            {
              values_set_x ( x );
              coords_set_polar_from_cartesian ( (x-(img_width>>1)), -(y-(img_height>>1)) );
              current_chan = RED;
              *ptr = (guchar) (formula_execute(red_chan)); ++ptr;
              current_chan = GREEN;
              *ptr = (guchar) (formula_execute(green_chan)); ++ptr;
              current_chan = BLUE;
              *ptr = (guchar) (formula_execute(blue_chan)); ++ptr;
            }
        }
    }
  else
    {
      const gint col_size = dvals->height*row_stride;
      const gint row_size = dvals->width*3*sizeof(guchar);

      for ( row_ptr=pixbuf_pixels, y=0.0;
            row_ptr<(pixbuf_pixels+col_size);
            row_ptr+=row_stride, y+=caspect_ratio_h )
        {
          values_set_y ( y );

          for ( ptr=row_ptr, x=0.0;
                ptr<(row_ptr+row_size);
                x+=caspect_ratio_w )
            {
              values_set_x ( x );
              coords_set_polar_from_cartesian ( (x-(img_width>>1)), -(y-(img_height>>1)) );
              current_chan = GRAY;
              memset(ptr, (guchar) (formula_execute(gray_chan)), 3*sizeof(guchar));
              ptr += 3;
            }
        }
    }

  destroy_formulas ( dvals, red_chan, green_chan, blue_chan, gray_chan, alpha_chan );
}
