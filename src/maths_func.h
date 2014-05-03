/*
 * maths_func.h
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


#ifndef __MATHS_FUNC_H__
#define __MATHS_FUNC_H__


#ifndef __MATHS_TREE_H__
#include "maths_tree.h"
#endif


#ifndef __USE_ISOC99
double log2 ( double x );
double round ( double x );
#endif


/* Types */
typedef gdouble ( maths_func_f )      ( const gint argc, GPtrArray *argv );


/* Structures */
typedef struct maths_function_t
{
  gchar        *name;
  gchar        *desc;
  gint          precalc_code;
  gint          argc;
  GPtrArray    *argv;
  maths_func_f *function;
} MATHS_FUNCTION ;


/* Function's argc rules */
enum { MATHS_FUNC_NO_ARG, MATHS_FUNC_ONE_ARG, MATHS_FUNC_TWO_ARG, MATHS_FUNC_N_OR_NO_ARG, MATHS_FUNC_N_ARG };


/* Functions prototypes */
gdouble  maths_func_exec     ( gpointer data );
gint     maths_func_dump_xml ( FILE *output, gint index, gpointer data );
gint     maths_func_precalc  ( gpointer data );
void     maths_func_free     ( gpointer data );


/* Defined functions */
extern MATHS_FUNCTION functions [];


#endif
