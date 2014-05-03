/*
 * maths_op.h
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


#ifndef __MATHS_OP_H__
#define __MATHS_OP_H__


#ifndef __MATHS_TREE_H__
#include "maths_tree.h"
#endif


/* Types */
typedef gdouble ( maths_op_f )  ( const gdouble a, const gdouble b );


/* Structure */
typedef struct maths_operator_t
{
  gchar              *name;
  gchar              *desc;
  gint                precalc_code;
  MATHS_TREE_ELEMENT *l;
  MATHS_TREE_ELEMENT *r;
  maths_op_f         *operation;
} MATHS_OPERATOR ;


/* Operations prototypes */
gdouble maths_op_exec     ( gpointer data );
gint    maths_op_dump_xml ( FILE *output, gint index, gpointer data );
gint    maths_op_precalc  ( gpointer data );
void    maths_op_free     ( gpointer data );


extern MATHS_OPERATOR operators [];


#endif
