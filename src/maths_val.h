/*
 * maths_val.h
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


#ifndef __MATHS_VAL_H__
#define __MATHS_VAL_H__

#ifndef __MATHS_TREE_H__
#include "maths_tree.h"
#endif


#ifndef M_GOLD_NUMBER
#define M_GOLD_NUMBER 1.618033989
#endif


/* Types */
typedef struct maths_value_t
{
  gchar    *name;
  gchar    *desc;
  gint      precalc_code;
  gdouble  *value;
} MATHS_VALUE ;


/* Values prototypes */
MATHS_VALUE *maths_val_alloc    ( void );
gdouble      maths_val_exec     ( gpointer data );
gint         maths_val_dump_xml ( FILE *output, gint index, gpointer data );
gint         maths_val_precalc  ( gpointer data );
void         maths_val_free     ( gpointer data );


/* Provided for convenience */
void coords_set_polar_from_cartesian ( const gdouble x, const gdouble y );
void coords_set_cartesian_from_polar ( const gdouble r, const gdouble t );


/* Those functions are useful for variables modification */
void values_set_w ( const gdouble val );
void values_set_h ( const gdouble val );
void values_set_x ( const gdouble val );
void values_set_y ( const gdouble val );
void values_set_r ( const gdouble val );
void values_set_t ( const gdouble val );
void values_set_red ( const gdouble val );
void values_set_green ( const gdouble val );
void values_set_gray ( const gdouble val );
void values_set_blue ( const gdouble val );
void values_set_alpha ( const gdouble val );


/* Defined values */
extern MATHS_VALUE values [];


#endif
