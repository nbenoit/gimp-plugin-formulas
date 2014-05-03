/*
 * formula.h
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


#ifndef __FORMULA_H__
#define __FORMULA_H__

#ifndef __MATHS_TREE_H__
#include "maths_tree.h"
#endif


/* Formula Structure */
typedef struct formula_t
{
  MATHS_TREE_ELEMENT  *head;
  gchar               *str;
} FORMULA ;


/* Creates a new formula based on the provided string */
FORMULA *formula_new ( gchar    *str, gboolean  want_report );

/* Evaluates a formula after it has been created with formula_new() */
gdouble formula_execute ( FORMULA *f );

/* Dumps the xml description of the formula into an xml file */
void formula_dump_xml_tree (  FORMULA *f, FILE *output );

/* Pprecalculates/Optimizes a formula */
void formula_precalc ( FORMULA *f );

/* Destroys a formula */
void formula_destroy ( FORMULA *f );


#endif
