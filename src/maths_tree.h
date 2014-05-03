/*
 * maths_tree.h
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


#ifndef __MATHS_TREE_H__
#define __MATHS_TREE_H__


typedef gdouble ( maths_tree_exec_f )     ( gpointer data );
typedef gint    ( maths_tree_dump_xml_f ) ( FILE *output, gint index, gpointer data );
typedef gint    ( maths_tree_precalc_f )  ( gpointer data );
typedef void    ( maths_tree_free_f )     ( gpointer data );


/* Precalculation opportunities
   PRECALC_NOT:  element can not be precalculated
   PRECALC_TERM: element can be included in precalculation (useless to precalculate it alone)
   PRECALC_OK:   element can be precalculated */
enum { PRECALC_NOT, PRECALC_TERM, PRECALC_OK };


/* Element structure */
typedef struct maths_tree_el_t
{
  gpointer              *data;
  maths_tree_exec_f     *exec;
  maths_tree_dump_xml_f *dump_xml;
  maths_tree_precalc_f  *precalc;
  maths_tree_free_f     *free;
} MATHS_TREE_ELEMENT ;


#endif
