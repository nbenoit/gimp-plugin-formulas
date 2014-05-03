/*
 * tokens_table.c
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
#include <glib.h>
#include "error.h"
#include "tokens_table.h"
#include "maths_op.h"
#include "maths_val.h"
#include "maths_func.h"
#include "plugin-intl.h"


/*
 * Hash function ( produced by gperf version 3.0.3 (using options -c -m 4096) )
 */
static guint
tokens_table_get_hash_code ( gconstpointer key )
{
  static guchar asso_values[] =
    {
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 49, 51, 51,
       0, 51, 48, 47, 51, 46, 51, 45, 12, 51,
      34, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 38, 51, 51,  0, 12,  0,
       6,  1, 51, 10, 14,  0, 37, 51,  7, 23,
       4,  7, 21, 15, 15,  2,  0,  9, 21,  3,
      18,  2, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
      51, 51, 51, 51, 51, 51
    };

  register int hval = strlen( (gchar *) key);

  switch (hval)
    {
      default:
        hval += asso_values[*( ((guchar *) key ) + 4)];
      /*FALLTHROUGH*/
      case 4:
      case 3:
        hval += asso_values[*( ((guchar *) key ) + 2)];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[*( ((guchar *) key ) + 1)];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[*( ((guchar *) key ) + 0)];
        break;
    }

  return hval;
}


/*
 * Creates the hash table.
 */
GHashTable *
tokens_table_build ( void )
{
  GHashTable *htable;
  gint i;

  if ((htable = g_hash_table_new(&(tokens_table_get_hash_code), &(g_str_equal))) == NULL)
    {
      error("exp_build_table", _("unable to build the table"));
      return NULL;
    }

  for (i=0; operators[i].name!= NULL; ++i)
    g_hash_table_insert(htable, operators[i].name, &operators[i] );

  for (i=0; values[i].name!= NULL; ++i)
    g_hash_table_insert(htable, values[i].name, &values[i] );

  for (i=0; functions[i].name!= NULL; ++i)
    g_hash_table_insert(htable, functions[i].name, &functions[i] );

  return htable;
}


/*
 * Destroys the hash table.
 */
void
tokens_table_destroy ( GHashTable *htable )
{
  g_hash_table_destroy(htable);
}
