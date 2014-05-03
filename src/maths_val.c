/*
 * maths_val.c
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <glib.h>
#include "error.h"
#include "maths_val.h"
#include "plugin-intl.h"


/* Allocation of a value */
MATHS_VALUE *
maths_val_alloc ( void )
{
  MATHS_VALUE *val;
  val = (MATHS_VALUE *) g_malloc ( sizeof(MATHS_VALUE) );
  val->name = NULL;
  val->desc = NULL;
  val->precalc_code = PRECALC_TERM;
  val->value = NULL;
  return val;
}


/* Execution of a value */
gdouble
maths_val_exec ( gpointer data )
{
  MATHS_VALUE *val = (MATHS_VALUE *) data;
  return ( *val->value );  
}


/* XML dump of a value */
gint
maths_val_dump_xml ( FILE *output,
                     gint index,
                     gpointer data )
{
  MATHS_VALUE *val = (MATHS_VALUE *) data;

  /* if the value has been allocated */
  if ( ( val->name == NULL ) && ( val->desc == NULL ) )
    {
      if ( val->value != NULL )
        fprintf ( output, "\t<node id=\"%d\">\n\t\t<text label=\"%.3f\"/>\n\t</node>\n", index, *val->value );
    }
  else
    fprintf ( output, "\t<node id=\"%d\">\n\t\t<text label=\"%s\"/>\n\t</node>\n", index, val->name );

  return ( 1 );
}


/* Precalculation **/
gint
maths_val_precalc ( gpointer data )
{
  MATHS_VALUE *val = (MATHS_VALUE *) data;
  return ( val->precalc_code );
}


/* Memory desallocation of a value */
void
maths_val_free ( gpointer data )
{
  MATHS_VALUE *val = (MATHS_VALUE *) data;

  /* if the value has been allocated */
  if ( ( val->name == NULL ) && ( val->desc == NULL ) )
    if ( val->value != NULL )
      g_free ( val->value );
}


static gdouble dbl_pi = G_PI;
static gdouble dbl_e = G_E;
static gdouble dbl_j = M_GOLD_NUMBER;
static gdouble dbl_w = 0.0f;
static gdouble dbl_h = 0.0f;
static gdouble dbl_x = 0.0f;
static gdouble dbl_y = 0.0f;
static gdouble dbl_r = 0.0f;
static gdouble dbl_t = 0.0f;
/*
static gdouble dbl_red = 0.0f;
static gdouble dbl_green = 0.0f;
static gdouble dbl_gray = 0.0f;
static gdouble dbl_blue = 0.0f;
static gdouble dbl_alpha = 0.0f;
*/

/*
 * Cartesian to polar conversion.
 */
void
coords_set_polar_from_cartesian ( const gdouble x,
                                  const gdouble y )
{
  if ( ( x == 0.0 ) && ( y == 0.0 ) )
    {
      dbl_r = 0.0;
      dbl_t = 0.0;
    }
  else
    {
      dbl_r = sqrt ( (x*x) + (y*y) );
      dbl_t = atan2 ( x, y );
    }
}


/*
 * Polar to cartesian conversion.
 */
void
coords_set_cartesian_from_polar ( const gdouble r,
                                  const gdouble t )
{
  dbl_x = ( r * cos ( t ) );
  dbl_y = ( r * sin ( t ) );
}


void
values_set_w ( const gdouble val )
{
  dbl_w = val;
}


void
values_set_h ( const gdouble val )
{
  dbl_h = val;
}


void
values_set_x ( const gdouble val )
{
  dbl_x = val;
}


void
values_set_y ( const gdouble val )
{
  dbl_y = val;
}

void
values_set_r ( const gdouble val )
{
  dbl_r = val;
}

void
values_set_t ( const gdouble val )
{
  dbl_t = val;
}
/*
void
values_set_red ( const gdouble val )
{
  dbl_red = val;
}

void
values_set_gray ( const gdouble val )
{
  dbl_gray = val;
}

void
values_set_green ( const gdouble val )
{
  dbl_green = val;
}

void
values_set_blue ( const gdouble val)
{
  dbl_blue = val;
}


void
values_set_alpha ( const gdouble val)
{
  dbl_alpha = val;
}
*/

MATHS_VALUE values[] = 
  { {"pi",    "Pi",          PRECALC_TERM, &dbl_pi},
    {"e",     "e",           PRECALC_TERM, &dbl_e},
    {"j",     "Gold Number", PRECALC_TERM, &dbl_j},
    {"w",     "w",           PRECALC_NOT,  &dbl_w},
    {"h",     "h",           PRECALC_NOT,  &dbl_h},
    {"x",     "x",           PRECALC_NOT,  &dbl_x},
    {"y",     "y",           PRECALC_NOT,  &dbl_y},
    {"r",     "r",           PRECALC_NOT,  &dbl_r},
    {"t",     "t",           PRECALC_NOT,  &dbl_t},
    /*
    {"red",   "red",         PRECALC_NOT,  &dbl_red},
    {"gray",  "gray",        PRECALC_NOT,  &dbl_gray},
    {"green", "green",       PRECALC_NOT,  &dbl_green},
    {"blue",  "blue",        PRECALC_NOT,  &dbl_blue},
    {"alpha", "alpha",       PRECALC_NOT,  &dbl_alpha},
    */
    {NULL, NULL,             PRECALC_NOT,   NULL}
  };
