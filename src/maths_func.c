/*
 * maths_func.c
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
#include "maths_func.h"
#include "maths_val.h"
#include "plugin-intl.h"


/* Accessors to channel value according to coords */
extern gdouble get_red_at ( gdouble, gdouble );
extern gdouble get_gray_at ( gdouble, gdouble );
extern gdouble get_green_at ( gdouble, gdouble );
extern gdouble get_blue_at ( gdouble, gdouble );
extern gdouble get_alpha_at ( gdouble, gdouble );
extern gdouble get_rgb_at ( gdouble, gdouble );


/* Execution of a function */
gdouble
maths_func_exec ( gpointer data )
{
  MATHS_FUNCTION *func =  (MATHS_FUNCTION *) data;
  return ( func->function ( func->argc, func->argv ) );  
}


/* XML dump of a function */
gint
maths_func_dump_xml ( FILE *output,
                      gint index,
                      gpointer data )
{
  gint i, tmp;
  MATHS_FUNCTION *func =  (MATHS_FUNCTION *) data;
  MATHS_TREE_ELEMENT *arg;
  gint old_index = index;

  fprintf ( output, "\t<node id=\"%d\">\n\t\t<text label=\"%s)\"/>\n\t</node>\n", old_index, func->name );
  ++index;

  for (i=0; i<func->argc; ++i)
    {
      arg = g_ptr_array_index ( func->argv, i );

      if ( arg != NULL )
        {
          tmp = index;
          index += arg->dump_xml ( output, index, arg->data );
          fprintf ( output, "\t<edge src=\"%d\" dest=\"%d\"/>\n", old_index, tmp );
        }
    }

  return ( index - old_index );
}


/* Precalculation */
gint
maths_func_precalc ( gpointer data )
{
  gint i;
  gint *arg_precalc;
  gint global_precalc;
  GPtrArray *new_argv;
  MATHS_TREE_ELEMENT *arg;
  MATHS_TREE_ELEMENT *el;
  MATHS_VALUE *val;
  MATHS_FUNCTION *func = (MATHS_FUNCTION *) data;

  if ( func->argc == 0 )
    return func->precalc_code;

  arg_precalc = (gint *) g_malloc ( func->argc * sizeof(gint) );
  global_precalc = PRECALC_OK;

  for (i=0; i<func->argc; ++i)
    {
      arg = g_ptr_array_index ( func->argv, i );
      arg_precalc[i] = arg->precalc ( arg->data );

      if ( arg_precalc[i] == PRECALC_NOT )
        global_precalc = PRECALC_NOT;
    }

  if ( ( global_precalc == PRECALC_OK ) &&
       ( func->precalc_code == PRECALC_OK ) )
    {
      g_free ( arg_precalc );
      return PRECALC_OK;
    }

  new_argv = g_ptr_array_new ( );

  for (i=0; i<func->argc; ++i)
    {
      arg = g_ptr_array_index ( func->argv, i );

      if ( arg_precalc[i] == PRECALC_OK )
        {
          val = maths_val_alloc ( );
          val->value = (gdouble *) g_malloc ( sizeof(gdouble) );
          (*val->value) = arg->exec ( arg->data );

          el = (MATHS_TREE_ELEMENT *) g_malloc ( sizeof(MATHS_TREE_ELEMENT) );
          el->data = ( gpointer ) val;
          el->exec = maths_val_exec;
          el->dump_xml = maths_val_dump_xml;
          el->precalc = maths_val_precalc;
          el->free = maths_val_free;
          arg->free ( arg->data );
          g_free ( arg );
          arg = el;
        }
      
      g_ptr_array_add ( new_argv, arg );
    }

  g_ptr_array_free ( func->argv, FALSE );
  func->argv = new_argv;
  g_free ( arg_precalc );
  return PRECALC_NOT;
}


/* Memory desallocation of a function */
void
maths_func_free ( gpointer data )
{
  gint i;
  MATHS_TREE_ELEMENT *arg;
  MATHS_FUNCTION *func =  (MATHS_FUNCTION *) data;

  for (i=0; i<func->argc; ++i)
    {
      arg = g_ptr_array_index ( func->argv, i );

      if ( arg != NULL )
        {
          arg->free ( arg->data );
          g_free ( arg );
        }
    }

  if ( func->argv != NULL )
    g_ptr_array_free ( func->argv, FALSE );
}


static gdouble
dred ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *x, *y;

  x = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  y = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));

  return get_red_at ( x->exec(x->data), y->exec(y->data) );
}

static gdouble
dgray ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *x, *y;

  x = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  y = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));

  return get_gray_at ( x->exec(x->data), y->exec(y->data) );
}

static gdouble
dgreen ( const gint argc, GPtrArray *argv )
{
MATHS_TREE_ELEMENT *x, *y;

  x = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  y = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));

  return get_green_at ( x->exec(x->data), y->exec(y->data) );
}

static gdouble
dblue ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *x, *y;

  x = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  y = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));

  return get_blue_at ( x->exec(x->data), y->exec(y->data) );
}

static gdouble
dalpha ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *x, *y;

  x = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  y = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));

  return get_alpha_at ( x->exec(x->data), y->exec(y->data) );
}

static gdouble
drgb ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *x, *y;

  x = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  y = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));

  return get_rgb_at ( x->exec(x->data), y->exec(y->data) );
}

static gdouble
drand ( const gint argc, GPtrArray *argv )
{
  return g_random_double ( );
}

static gdouble
dabs ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return fabs ( arg->exec(arg->data) );
}

static gdouble
dsign ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  gdouble d = arg->exec ( arg->data );

  if ( d == 0.0 )
    return 0.0;
  else if ( d > 0.0 )
    return 1.0;
  else
    return -1.0;
}

static gdouble
dsin ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return sin ( arg->exec(arg->data) );
}

static gdouble
dsinh ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return sinh ( arg->exec(arg->data) );
}

static gdouble
dasin ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return asin ( arg->exec(arg->data) );
}

static gdouble
dasinh ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return asinh ( arg->exec(arg->data) );
}

static gdouble
dcos ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return cos ( arg->exec(arg->data) );
}

static gdouble
dcosh ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return cosh ( arg->exec(arg->data) );
}

static gdouble
dacos ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return acos ( arg->exec(arg->data) );
}

static gdouble
dacosh ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return acosh ( arg->exec(arg->data) );
}

static gdouble
dtan ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return tan ( arg->exec(arg->data) );
}

static gdouble
dtanh ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return tanh ( arg->exec(arg->data) );
}

static gdouble
datan ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return atan ( arg->exec(arg->data) );
}

static gdouble
datan2 ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg0 = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  MATHS_TREE_ELEMENT *arg1 = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 1 ) ));
  return atan2 ( arg0->exec(arg0->data), arg1->exec(arg1->data) );
}

static gdouble
datanh ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return atanh ( arg->exec(arg->data) );
}

static gdouble
drad ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return ( arg->exec(arg->data) * (G_PI/180.0) );
}

static gdouble
ddeg ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return ( arg->exec(arg->data) * (180.0/G_PI) );
}

static gdouble
dsqrt ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return sqrt ( arg->exec(arg->data) );
}

static gdouble
dcbrt ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return cbrt ( arg->exec(arg->data) );
}

static gdouble
dlog ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return log ( arg->exec(arg->data) );
}

static gdouble
dlog2 ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return log2 ( arg->exec(arg->data) );
}

static gdouble
dlog10 ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return log10 ( arg->exec(arg->data) );
}

static gdouble
dexp ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return exp ( arg->exec(arg->data) );
}

static gdouble
dceil ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return ceil ( arg->exec(arg->data) );
}

static gdouble
dround ( const gint argc, GPtrArray *argv )
{
  MATHS_TREE_ELEMENT *arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, 0 ) ));
  return ( round ( arg->exec ( arg->data ) ) );
}

static gdouble
dmin ( const gint argc, GPtrArray *argv )
{
  gint i;
  MATHS_TREE_ELEMENT *arg;
  gdouble min, tmp;

  min = G_MAXDOUBLE;

  for (i=0; i<argc; ++i)
    {
      arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, i ) ));
      tmp = arg->exec ( arg->data );

      if ( tmp < min )
        min = tmp;
    }

  return min;
}

static gdouble
dmax ( const gint argc, GPtrArray *argv )
{
  gint i;
  MATHS_TREE_ELEMENT *arg;
  gdouble max, tmp;

  max = G_MINDOUBLE;

  for (i=0; i<argc; ++i)
    {
      arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, i ) ));
      tmp = arg->exec ( arg->data );

      if ( tmp > max )
        max = tmp;
    }

  return max;
}

static gdouble
davg ( const gint argc, GPtrArray *argv )
{
  gint i;
  MATHS_TREE_ELEMENT *arg;
  gdouble avg;

  avg = 0.0;

  for (i=0; i<argc; ++i)
    {
      arg = ( (MATHS_TREE_ELEMENT *) ( g_ptr_array_index ( argv, i ) ));
      avg += arg->exec ( arg->data );
    }

  return ( avg / (gdouble) argc );
}


MATHS_FUNCTION functions[] = 
  {
    {"red(",   "Red channel value at x, y coordinates",                PRECALC_NOT, MATHS_FUNC_TWO_ARG, NULL, &dred},
    {"gray(",  "Gray channel value at x, y coordinates",               PRECALC_NOT, MATHS_FUNC_TWO_ARG, NULL, &dgray},
    {"green(", "Green channel value at x, y coordinates",              PRECALC_NOT, MATHS_FUNC_TWO_ARG, NULL, &dgreen},
    {"blue(",  "Blue channel value at x, y coordinates",               PRECALC_NOT, MATHS_FUNC_TWO_ARG, NULL, &dblue},
    {"alpha(", "Alpha channel value at x, y coordinates",              PRECALC_NOT, MATHS_FUNC_TWO_ARG, NULL, &dalpha},
    {"rgb(",   "Red, Green or Blue channel value at x, y coordinates", PRECALC_NOT, MATHS_FUNC_TWO_ARG, NULL, &drgb},
    {"rand(",  "Random value between 0.0 and 1.0",                     PRECALC_NOT, MATHS_FUNC_NO_ARG,  NULL, &drand},
    {"abs(",   "Absolute value",                                       PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dabs},
    {"sign(",  "Sign of the value",                                    PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dsign},
    {"sin(",   "Sine",                                                 PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dsin},
    {"sinh(",  "Hyperbolic sine",                                      PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dsinh},
    {"asin(",  "Arc sine",                                             PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dasin},
    {"asinh(", "Arc hyperbolic",                                       PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dasinh},
    {"cos(",   "Cosine",                                               PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dcos},
    {"cosh(",  "Hyperbolic cosine",                                    PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dcosh},
    {"acos(",  "Arc cosine",                                           PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dacos},
    {"acosh(", "Arc hyperbolic cosine",                                PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dacosh},
    {"tan(",   "Tangent",                                              PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dtan},
    {"tanh(",  "Hyperbolic tangent",                                   PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dtanh},
    {"atan(",  "Arc tangent",                                          PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &datan},
    {"atan2(", "Arc tangent with correct quadrant",                    PRECALC_OK,  MATHS_FUNC_TWO_ARG, NULL, &datan2},
    {"atanh(", "Arc hyperbolic tangent",                               PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &datanh},
    {"rad(",   "To radians conversion",                                PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &drad},
    {"deg(",   "To degrees conversion",                                PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &ddeg},
    {"sqrt(",  "Square root",                                          PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dsqrt},
    {"cbrt(",  "Cube root",                                            PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dcbrt},
    {"log(",   "Natural logarithmic",                                  PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dlog},
    {"log2(",  "Base-2 logarithmic",                                   PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dlog2},
    {"log10(", "Base-10 logarithmic",                                  PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dlog10},
    {"exp(",   "Base-e exponential",                                   PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dexp},
    {"ceil(",  "Smallest integral value not less than argument",       PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dceil},
    {"round(", "Round to nearest integer, away from zero",             PRECALC_OK,  MATHS_FUNC_ONE_ARG, NULL, &dround},
    {"min(",   "Minimal value",                                        PRECALC_OK,  MATHS_FUNC_N_ARG,   NULL, &dmin},
    {"max(",   "Maximal value",                                        PRECALC_OK,  MATHS_FUNC_N_ARG,   NULL, &dmax},
    {"avg(",   "Average value",                                        PRECALC_OK,  MATHS_FUNC_N_ARG,   NULL, &davg},
    {NULL,     NULL,                                                   PRECALC_NOT, MATHS_FUNC_NO_ARG,  NULL, NULL}
  };
