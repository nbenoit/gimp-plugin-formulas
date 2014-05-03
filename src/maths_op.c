/*
 * maths_op.c
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
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <glib.h>
#include "error.h"
#include "maths_op.h"
#include "maths_val.h"
#include "plugin-intl.h"


/* Execution of an operation */
gdouble
maths_op_exec ( gpointer data )
{
  MATHS_OPERATOR *op =  (MATHS_OPERATOR *) data;
  return ( op->operation ( op->l->exec(op->l->data), op->r->exec(op->r->data) ) );
}


/* XML Dump of an operation */
gint
maths_op_dump_xml ( FILE *output,
                    gint index,
                    gpointer data )
{
  MATHS_OPERATOR *op = (MATHS_OPERATOR *) data;
  gint old_index = index;
  gint tmp;

  fprintf ( output, "\t<node id=\"%d\">\n\t\t<text label=\"%s\"/>\n\t</node>\n", old_index, op->name );
  ++index;

  if ( op->l != NULL )
    {
      tmp = index;
      index += op->l->dump_xml ( output, index, op->l->data );
      fprintf ( output, "\t<edge src=\"%d\" dest=\"%d\"/>\n", old_index, tmp );
    }

  if ( op->r != NULL )
    {
      tmp = index;
      index += op->r->dump_xml ( output, tmp, op->r->data );
      fprintf ( output, "\t<edge src=\"%d\" dest=\"%d\"/>\n", old_index, tmp );
    }

  return ( index - old_index );
}

/* Precalculation */
gint
maths_op_precalc ( gpointer data )
{
  MATHS_OPERATOR *op = (MATHS_OPERATOR *) data;
  MATHS_TREE_ELEMENT *el;
  MATHS_VALUE *val;
  gint l_precalc_code, r_precalc_code;

  l_precalc_code = op->l->precalc ( op->l->data );
  r_precalc_code = op->r->precalc ( op->r->data );

  if ( ( l_precalc_code != PRECALC_NOT ) &&
       ( r_precalc_code != PRECALC_NOT ) &&
       ( op->precalc_code == PRECALC_OK ) )
    return PRECALC_OK;

  if ( l_precalc_code == PRECALC_OK )
    {
      val = maths_val_alloc ( );
      val->value = (gdouble *) g_malloc ( sizeof(gdouble) );
      (*val->value) = op->l->exec ( op->l->data );

      el = (MATHS_TREE_ELEMENT *) g_malloc ( sizeof(MATHS_TREE_ELEMENT) );
      el->data = ( gpointer ) val;
      el->exec = maths_val_exec;
      el->dump_xml = maths_val_dump_xml;
      el->precalc = maths_val_precalc;
      el->free = maths_val_free;
      op->l->free ( op->l->data );
      g_free ( op->l );
      op->l = el;
    }

  if ( r_precalc_code == PRECALC_OK )
    {
      val = maths_val_alloc ( );
      val->value = (gdouble *) g_malloc ( sizeof(gdouble) );
      (*val->value) = op->r->exec ( op->r->data );

      el = (MATHS_TREE_ELEMENT *) g_malloc ( sizeof(MATHS_TREE_ELEMENT) );
      el->data = ( gpointer ) val;
      el->exec = maths_val_exec;
      el->dump_xml = maths_val_dump_xml;
      el->precalc = maths_val_precalc;
      el->free = maths_val_free;
      op->r->free ( op->r->data );
      g_free ( op->r );
      op->r = el;
    }

  return PRECALC_NOT;
}


/* Memory desallocation of an operation */
void
maths_op_free ( gpointer data )
{
  MATHS_OPERATOR *op = (MATHS_OPERATOR *) data;

  if ( op->l != NULL )
    {
      op->l->free ( op->l->data );
      g_free ( op->l );
    }

  if ( op->r != NULL )
    {
      op->r->free ( op->r->data );
      g_free ( op->r );
    }
}


static gdouble
add ( const gdouble a,
      const gdouble b )
{
  return (a + b);
}

static gdouble
sub ( const gdouble a,
      const gdouble b )
{
  return (a - b);
}

static gdouble
mul ( const gdouble a,
      const gdouble b )
{
  return (a * b);
}

static gdouble
division ( const gdouble a,
           const gdouble b )
{
  return (a / b);
}

static gdouble
modulo ( const gdouble a,
         const gdouble b )
{
  return (gdouble) ((gint) a % (gint) b);
}


MATHS_OPERATOR operators[] = 
  { {"+", "Addition",       PRECALC_OK,  NULL, NULL, &add},
    {"-", "Substraction",   PRECALC_OK,  NULL, NULL, &sub},
    {"*", "Multiplication", PRECALC_OK,  NULL, NULL, &mul},
    {"/", "Division",       PRECALC_OK,  NULL, NULL, &division},
    {"^", "Power",          PRECALC_OK,  NULL, NULL, &pow},
    {"%", "Modulo",         PRECALC_OK,  NULL, NULL, &modulo},
    {NULL, NULL,            PRECALC_NOT, NULL, NULL, NULL}
  };
