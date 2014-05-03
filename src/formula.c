/*
 * formula.c
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "error.h"
#include "tokens_table.h"
#include "formula.h"
#include "maths_val.h"
#include "maths_op.h"
#include "maths_func.h"
#include "char_masks.h"
#include "plugin-intl.h"


static GHashTable  *htab = NULL;
static gchar       *key = NULL;
static guint        running_formulas = 0;
static gboolean     report_errors = TRUE;


static MATHS_TREE_ELEMENT * mstr_eval ( gchar        *mstr,
                                        const gulong  len );


/*
 * Returns 1 if we have a presentation character, else it returns 0.
 */
static gint
is_presentation ( gchar l )
{
  if ( ( l == '\n' ) || ( l == '\t' ) || ( l == ' ' ) )
    return 1;
  else
    return 0;
}


/*
 * Returns 1 if we have a letter, else it returns 0.
 */
static gint
is_letter ( gchar l )
{
  if ( ( l >= 'a' ) && ( l <= 'z' ) )
    return 1;
  else
    return 0;
}


/*
 * Returns 1 if we have an operator, else it returns 0.
 */
static gint
is_op ( gchar op )
{
  if ( mask_operators[(guchar) op] == MASK_NOT_AN_OP )
    return 0;

  return 1;
}


/*
 * Returns 1 if we have a prioritary operator, else it returns 0.
 */
static gint
is_op_prio ( gchar op )
{
  if ( mask_operators[(guchar) op] != MASK_PRIO_OP )
    return 0;

  return 1;
}


/*
 * Returns 1 if we have a prioritary operator, else it returns 0.
 */
static gint
is_op_non_prio ( gchar op )
{
  if ( mask_operators[(guchar) op] != MASK_NON_PRIO_OP )
    return 0;

  return 1;
}


/*
 * Returns a pointer to the next non prioritary operator.
 */
static gchar *
get_next_non_prio_op ( gchar        *mstr,
                       const gulong  len )
{
  gchar *ptr;
  gint k = 0;

  if ( is_op_non_prio(*mstr) )
    return mstr;
  else if ( *mstr == '(' )
    k = 1;

  for ( ptr=mstr+1; ptr<(mstr+len); ++ptr )
    {
      if ( *ptr == '(' )
        {
          ++k;
        }
      else if ( *ptr == ')' )
        --k;
      else if ( k == 0 )
        {
          if ( *ptr == '+' )
            return ptr;
          else if ( ( *ptr == '-' ) && ( !is_op(*(ptr-1)) ) )
            return ptr;
        }
    }

  return NULL;
}


/*
 * Returns a pointer to the next next non prioritary operator.
 */
static gchar *
get_next_next_non_prio_op ( gchar        *mstr,
                            const gulong  len )
{
  gchar *ptr;
  gint k = 0;

  for ( ptr=mstr+1; ptr<(mstr+len); ++ptr )
    {
      if ( *ptr == '(' )
        ++k;
      else if ( *ptr == ')' )
        --k;
      else if ( k == 0 )
        {
          if ( *ptr == '+' )
            return ptr;
          else if ( ( *ptr == '-' ) && ( !is_op(*(ptr-1)) ) )
            return ptr;
        }
    }

  return NULL;
}


/*
 * Returns a pointer to the prioritary operator.
 */
static gchar *
get_next_prio_op ( gchar        *mstr,
                   const gulong  len )
{
  gchar *ptr;
  gint k = 0;

  for ( ptr=mstr; ptr<(mstr+len); ++ptr )
    {
      if ( *ptr == '(' )
        ++k;
      else if ( *ptr == ')' )
        --k;
      if ( ( is_op_prio(*ptr) ) && ( k == 0 ) )
        return ptr;
    }

  return NULL;
}


/*
 * Returns the index of the closing parenthese.
 */
static gchar *
get_next_opening_p ( gchar        *mstr,
                     const gulong  len )
{
  gchar *ptr;

  for ( ptr=mstr; ptr<(mstr+len); ++ptr )
    if ( *ptr == '(' )
      return ptr;

  return NULL;
}


/*
 * Returns the index of the next arg separator.
 */
static gchar *
get_next_arg_delim ( gchar        *mstr,
                     const gulong  len )
{
  gchar *ptr;
  gint k = 0;

  for ( ptr=mstr; ptr<(mstr+len); ++ptr )
    if ( *ptr == '(' )
      ++k;
    else if ( *ptr == ')' )
      --k;
    else if ( k == 0 )
      if ( *ptr == ',' )
        return ptr;

  return NULL;
}


/*
 * Returns the index of the closing parenthese.
 */
static gchar *
get_next_closing_p ( gchar        *mstr,
                     const gulong  len )
{
  gchar *ptr;
  gint k = 0;

  for ( ptr=mstr, k=0; ptr<=(mstr+len); ++ptr )
    {
      if ( *ptr == '(' )
        ++k;
      else if ( *ptr == ')' )
        {
          if ( k == 0 )
            return ptr;

          --k;
        }
    }

  return ( mstr+len-1 );  /* we allow unclosed parentheses */
}


/*
 * Cleans up an expression (removes presentation chars)
 */
static gchar *
mstr_clean ( gchar        *mstr,
             const gulong  len )
{
  gchar *new_mstr;
  gchar *ptr, *new_ptr;

  new_mstr = ( gchar * ) g_malloc ( sizeof(gchar) * len );

  for ( ptr=mstr, new_ptr=new_mstr; ptr<(mstr+len); ++ptr )
    {
      if ( !is_presentation(*ptr) )
        {
          *new_ptr = *ptr;
          ++new_ptr;
        }
    }

  if ( (new_ptr-new_mstr) < len )
    new_mstr = ( gchar * ) g_realloc ( new_mstr, (new_ptr-new_mstr) * sizeof(gchar) );

  return ( new_mstr );
}


/*
 * Builds a new element.
 */
static MATHS_TREE_ELEMENT *
build_new_elem ( void )
{
  MATHS_TREE_ELEMENT *el;
  el = (MATHS_TREE_ELEMENT *) g_malloc ( sizeof(MATHS_TREE_ELEMENT) );
  el->data = NULL;
  el->exec = NULL;
  el->dump_xml = NULL;
  el->precalc = NULL;
  el->free = NULL;
  return ( el );
}


/*
 * Builds a new operator from a reference.
 */
static MATHS_OPERATOR *
build_new_op_from_ref ( MATHS_OPERATOR *ref )
{
  MATHS_OPERATOR *op;

  if ( ref == NULL )
    return NULL;

  op = ( MATHS_OPERATOR * ) g_malloc ( sizeof(MATHS_OPERATOR) );
  memcpy ( op, ref, sizeof(MATHS_OPERATOR) );

  return ( op );
}


/*
 * Builds a new function from a reference.
 */
static MATHS_FUNCTION *
build_new_func_from_ref ( MATHS_FUNCTION *ref )
{
  MATHS_FUNCTION *func;

  if ( ref == NULL )
    return NULL;

  func = ( MATHS_FUNCTION * ) g_malloc ( sizeof(MATHS_FUNCTION) );
  memcpy ( func, ref, sizeof(MATHS_FUNCTION) );

  return ( func );
}


/*
 * Builds a new value from a reference.
 */
static MATHS_VALUE *
build_new_val_from_ref ( MATHS_VALUE *ref )
{
  MATHS_VALUE *val;

  if ( ref == NULL )
    return NULL;

  val = ( MATHS_VALUE * ) g_malloc ( sizeof(MATHS_VALUE) );
  memcpy ( val, ref, sizeof(MATHS_VALUE) );

  return ( val );
}


/*
 * Builds a list of arguments (used for functions).
 */
static gint
build_new_argv_list ( GPtrArray *argv, gchar *mstr, gulong len )
{
  MATHS_TREE_ELEMENT *mtree;
  gchar *next_delim = NULL;
  gchar *last_delim = mstr;
  gint   argc = 0;

  if ( argv == NULL )
    return -1;

  if ( len == 1 ) /* if there aren't any arg */
    return argc;

  while ( 1 )
    {
      if ( len == 0 )
        return argc;

      next_delim = get_next_arg_delim ( ( last_delim+1), (len-1) );

      if ( next_delim == NULL )
        next_delim = last_delim + len;

      if ( ( mtree = mstr_eval((last_delim+1), (next_delim-(last_delim+1))) ) == NULL )
        {
#ifdef VERBOSE
          error ( "mstr_eval", _("function\'s argument \'%s\' evaluation has failed"), last_delim+1 );
#endif
          return -1;
        }

      g_ptr_array_add ( argv, mtree );
      ++argc;
      len -= (next_delim-last_delim);
      last_delim = next_delim;
    }

  return argc;
}


/*
 * Builds the father operation for substractions and negative numbers.
 */
static MATHS_TREE_ELEMENT *
build_father ( MATHS_TREE_ELEMENT   *mtree,
               gchar                *mstr,
               gchar                *next_op,
               gchar                *next_next_op,
               const gulong          len )
{
  MATHS_TREE_ELEMENT *father_el;
  MATHS_OPERATOR *father_op;

  /* we finish building the mtree */
  if ((((MATHS_OPERATOR *) mtree->data)->r = mstr_eval(next_op+1, next_next_op-next_op-1)) == NULL)
    {
#ifdef VERBOSE
      error("build_father", _("right subtree creation failed"));
#endif
      return NULL;
    }

  /* we build the daddy */
  father_el = build_new_elem ( );

  if ( is_op_non_prio(*next_next_op) )
    {
      if ( ( father_op = build_new_op_from_ref(g_hash_table_lookup(htab, "+")) ) == NULL )
        {
          error(NULL, _("internal: addition operator not found"));
          return NULL;
        }
    }
  else
    {
      g_strlcpy ( key, next_next_op, 1+MIN(1, MAX_KEY_LENGTH) );

      if ( ( father_op = build_new_op_from_ref(g_hash_table_lookup(htab, key) ) ) == NULL )
        {
          if ( report_errors )
            error ( NULL,  _("unknown operator: \'%c\'"), *next_next_op );

          return NULL;
        }
    }

  father_el->data = ( gpointer ) father_op;
  father_el->exec = maths_op_exec;
  father_el->dump_xml = maths_op_dump_xml;
  father_el->precalc = maths_op_precalc;
  father_el->free = maths_op_free;

  father_op->l = mtree;

  if ( *next_next_op == '-' )
    {
      /* here, we send the '-' operator too so the number is understood as a negative one */
      if ( ( father_op->r = mstr_eval(next_next_op, len-(next_next_op-mstr)) ) == NULL )
        {
#ifdef VERBOSE
          error ( "build_father", _("father\'s right subtree creation failed") );
#endif
          return NULL;
        }
    }
  else if ( is_op_prio ( *next_next_op ) )
    {
      gchar *next_next_next_prio_op = get_next_prio_op ( next_next_op+1, len-(next_next_op-mstr)-1 );

      if ( next_next_next_prio_op != NULL )
        {
          father_el = build_father ( father_el, next_next_op+1, next_next_op, next_next_next_prio_op, len-(next_next_op-mstr)-1 );
        }
      else
        {
          if ( ( father_op->r = mstr_eval(next_next_op+1, len-(next_next_op-mstr)-1) ) == NULL )
            {
#ifdef VERBOSE
              error ( "build_father", _("father\'s right subtree creation failed") );
#endif
              return NULL;
            }
        }
    }
  else
    {
      if ( ( father_op->r = mstr_eval(next_next_op+1, len-(next_next_op-mstr)-1) ) == NULL )
        {
#ifdef VERBOSE
          error ( "build_father", _("father\'s right subtree creation failed") );
#endif
          return NULL;
        }
    }

  return ( father_el );
}


/*
 * Evaluates a formule (recursive) and builds the math tree.
 */
static MATHS_TREE_ELEMENT *
mstr_eval ( gchar        *mstr,
            const gulong  len )
{
  MATHS_TREE_ELEMENT *mtree;
  gchar *next_non_prio_op;

#ifdef DEBUG
  g_fprintf ( stderr, _("Analyzing \"%s\" with len=%d\n"), mstr, len );
#endif

  if ( len == 0 )
    return NULL;

  mtree = build_new_elem ( );

  next_non_prio_op = get_next_non_prio_op ( mstr, len );

  /* if we don't have any non prioritary operator */
  if ( next_non_prio_op == NULL )
    {
      gchar *next_prio_op = get_next_prio_op ( mstr, len );

      /* if we don't have any prioritary operator */
      if ( next_prio_op == NULL )
        {
          /* we try to find an opening parenthese */
          gchar *next_opening_p = get_next_opening_p ( mstr, len );

          if ( next_opening_p == NULL )
            {
              /* we have a constant! */
              MATHS_VALUE *mtree_val;

              if ( is_letter ( *mstr ) )
                {
                  g_strlcpy ( key, mstr, 1+MIN(len, MAX_KEY_LENGTH) );

                  if ( ( mtree_val = build_new_val_from_ref(g_hash_table_lookup(htab, key) ) ) == NULL )
                    {
                      if (report_errors)
                        error(NULL, _("unknown constant: \'%s\'"), mstr);

                      return NULL;
                    }
                }
              else
                {
                  mtree_val = maths_val_alloc ( );
                  mtree_val->value = (gdouble *) g_malloc ( sizeof(gdouble) );
                  (*mtree_val->value) = atof(mstr);
                }

              mtree->data = ( gpointer ) mtree_val;
              mtree->exec = maths_val_exec;
              mtree->dump_xml = maths_val_dump_xml;
              mtree->precalc = maths_val_precalc;
              mtree->free = maths_val_free;
            }
          else
            {
              gchar *closing_p = get_next_closing_p ( next_opening_p+1, (len-(next_opening_p-mstr)) );

              /* if we are not at the beginning of the math string then, it's a function */
              if ( next_opening_p != mstr )
                {
                  MATHS_TREE_ELEMENT *dad_mtree;
                  MATHS_FUNCTION *dad_mtree_func;
                  gint ref_argc;

                  dad_mtree = build_new_elem ( );

                  g_strlcpy ( key, mstr, 1+MIN((1+(next_opening_p-mstr)),MAX_KEY_LENGTH) ); /* we add 1, because we want the '(' too */

                  if ( ( dad_mtree_func = build_new_func_from_ref(g_hash_table_lookup(htab, key)) ) == NULL )
                    {
                      if ( report_errors )
                        error ( NULL, _("unknown function: \'%s)\'"), key );

                      return NULL;
                    }

                  ref_argc = dad_mtree_func->argc;

                  /* args parsing */
                  dad_mtree_func->argv = g_ptr_array_new ( );

                  if ( ( dad_mtree_func->argc = build_new_argv_list(dad_mtree_func->argv,next_opening_p,(closing_p-next_opening_p))) == -1 )
                    {
                      if ( report_errors )
                        error ( NULL, _("unable to build args list for function \'%s)\'"), dad_mtree_func->name );

                      return NULL;
                    }

                  /* we check argc */
                  if ( ref_argc == MATHS_FUNC_NO_ARG )
                    {
                      if ( dad_mtree_func->argc != 0 )
                        {
                          if ( report_errors )
                            error ( NULL, _("function \'%s)\' should not have any argument"), dad_mtree_func->name );

                          return NULL;
                        }
                    }
                  else if ( ref_argc == MATHS_FUNC_ONE_ARG )
                    {
                      if ( dad_mtree_func->argc != 1 )
                        {
                          if ( report_errors )
                            error ( NULL, _("function \'%s)\' should have only one argument"), dad_mtree_func->name );

                          return NULL;
                        }
                    }
                  else if ( ref_argc == MATHS_FUNC_TWO_ARG )
                    {
                      if ( dad_mtree_func->argc != 2 )
                        {
                          if ( report_errors )
                            error ( NULL, _("function \'%s)\' should have only two arguments"), dad_mtree_func->name );

                          return NULL;
                        }
                    }
                  else if ( ref_argc == MATHS_FUNC_N_ARG )
                    {
                      if ( dad_mtree_func->argc == 0 )
                        {
                          if ( report_errors )
                            error ( NULL, _("function \'%s)\' should have at least one argument"), dad_mtree_func->name );

                          return NULL;
                        }
                    }

                  dad_mtree->data = ( gpointer ) dad_mtree_func;
                  dad_mtree->exec = maths_func_exec;
                  dad_mtree->dump_xml = maths_func_dump_xml;
                  dad_mtree->precalc = maths_func_precalc;
                  dad_mtree->free = maths_func_free;

                  mtree = dad_mtree;
                }
              else /* it's not a function, we evaluate the block */
                {
                  if ( ( mtree = mstr_eval(next_opening_p+1, ((closing_p-next_opening_p)-1)) ) == NULL )
                    {
#ifdef VERBOSE
                      error ( "mstr_eval", _("inside parentheses evaluation has failed") );
#endif
                      return NULL;
                    }
                }
            }
        }
      else if ( next_prio_op == mstr )
        {
          /* if the operator is at the beginning of the math string */
          if ( report_errors )
            error ( NULL, _("got the operator \'%c\' without any lvalue"), *next_prio_op );

          return NULL;
        }
      else
        {
          MATHS_OPERATOR *mtree_op;
          
          /* if we do have a prioritary operator */
          gchar *next_next_prio_op = get_next_prio_op ( next_prio_op+1, len-(next_prio_op-mstr) );
          g_strlcpy ( key, next_prio_op, 1+MIN(1, MAX_KEY_LENGTH) );

          if ( ( mtree_op = build_new_op_from_ref(g_hash_table_lookup(htab, key)) ) == NULL )
            {
              if ( report_errors )
                error ( NULL, _("unknown operator: \'%c\'"), *next_prio_op );

              return NULL;
            }

          mtree->data = ( gpointer ) mtree_op;
          mtree->exec = maths_op_exec;
          mtree->dump_xml = maths_op_dump_xml;
          mtree->precalc = maths_op_precalc;
          mtree->free = maths_op_free;

          if ( ( mtree_op->l = mstr_eval(mstr, (next_prio_op-mstr)) ) == NULL )
            {
#ifdef VERBOSE
              error ( "mstr_eval", _("left subtree creation failed") );
#endif
              return NULL;
            }

          if ( next_next_prio_op == NULL )
            {
              if ( ( mtree_op->r = mstr_eval(next_prio_op+1, (len-1-(next_prio_op-mstr))) ) == NULL )
                {
#ifdef VERBOSE
                  error ( "mstr_eval", _("right subtree creation failed") );
#endif
                  return NULL;
                }
            }
          else
            {
              mtree = build_father ( mtree, mstr, next_prio_op, next_next_prio_op, len );
            }
        }
    }
  else if ( (next_non_prio_op == mstr) && (*next_non_prio_op != '-') )
    {
      /* if the operator is at the beginning of the math string and isn't a '-' */
      if ( report_errors )
        error ( NULL, _("got the operator \'%c\' without any lvalue"), *next_non_prio_op );

      return NULL;
    }
  else
    {
      MATHS_OPERATOR *mtree_op;

      /* if we do have a non-prioritary operator */
      g_strlcpy ( key, next_non_prio_op, 1+MIN(1, MAX_KEY_LENGTH) );

      if ( ( mtree_op = build_new_op_from_ref(g_hash_table_lookup(htab, key)) ) == NULL )
        {
          if ( report_errors )
            error ( NULL, _("unknown operator: \'%c\'.\n"), *next_non_prio_op );

          return NULL;
        }

      mtree->data = ( gpointer ) mtree_op;
      mtree->exec = maths_op_exec;
      mtree->dump_xml = maths_op_dump_xml;
      mtree->precalc = maths_op_precalc;
      mtree->free = maths_op_free;

      if ( next_non_prio_op != mstr ) /* if we don't have something like a "-x" */
        {
          if ( ( mtree_op->l = mstr_eval(mstr, (next_non_prio_op-mstr)) ) == NULL )
            {
#ifdef VERBOSE
              error ( "mstr_eval", _("left subtree creation failed") );
#endif
              return NULL;
            }
        }
      else
        {
          MATHS_TREE_ELEMENT *elem;
          MATHS_VALUE *mtree_val;
          elem = build_new_elem ( );

          
          mtree_val = maths_val_alloc ( );
          mtree_val->value = (gdouble *) g_malloc ( sizeof(gdouble) );
          (*mtree_val->value) = 0.0;

          elem->data = ( gpointer ) mtree_val;
          elem->exec = maths_val_exec;
          elem->dump_xml = maths_val_dump_xml;
          elem->precalc = maths_val_precalc;
          elem->free = maths_val_free;

          mtree_op->l = elem;
        }

      if ( *next_non_prio_op == '+' )
        {
          if ( ( mtree_op->r = mstr_eval(next_non_prio_op+1, len-(next_non_prio_op-mstr)-1) ) == NULL )
            {
#ifdef VERBOSE
              error ( "mstr_eval", _("right subtree creation failed") );
#endif
              return NULL;
            }
        }
      else
        {
          /* we must use get_next_next_non_prio_op here, because we support --x syntax */
          gchar *next_next_non_prio_op = get_next_next_non_prio_op ( next_non_prio_op, len-(next_non_prio_op-mstr) );

          if ( next_next_non_prio_op == NULL ) /* if we don't have any non-prio operator, it's easy */
            {
              if ( ( mtree_op->r = mstr_eval(next_non_prio_op+1, len-(next_non_prio_op-mstr)-1) ) == NULL )
                {
#ifdef VERBOSE
                  error ( "mstr_eval", _("right subtree creation failed") );
#endif
                  return NULL;
                }
            }
          else
            {
              mtree = build_father ( mtree, mstr, next_non_prio_op, next_next_non_prio_op, len );
            }
        }
    }

  return ( mtree );
}


/*
 * Frees the memory previously allocated to a formula.
 */
static void
formula_free_mem ( FORMULA *f )
{
  if ( f == NULL )
    return;

  if ( running_formulas == 0 )
  {
    if ( htab != NULL )
      {
        tokens_table_destroy ( htab );
        htab = NULL;
      }

    if ( key != NULL )
      {
        g_free ( key );
        key = NULL;
      }
  }

  if ( f->str != NULL )
    g_free ( f->str );

  if ( f->head != NULL )
    {
      f->head->free ( f->head->data );
      g_free ( f->head );
    }

  g_free ( f );
}


/*
 * Creates a tree according to the expression.
 */
FORMULA *
formula_new ( gchar    *str,
              gboolean  want_report )
{
  FORMULA *f;
  report_errors = want_report;

  /* we allocate memory for the formula structure */
  f = (FORMULA *) g_malloc ( sizeof(FORMULA) );
  f->str = NULL;
  f->head = NULL;

  /* we create what we need if we have to */
  if ( running_formulas == 0 )
    {
      htab = tokens_table_build ( );
      key = (gchar *) g_malloc(sizeof(gchar) * (MAX_KEY_LENGTH+1));
    }

  /* we check everything's all right */
  if ( htab == NULL )
    {
#ifdef VERBOSE
      error("formula_new", _("the expression table is invalid"));
#endif
      formula_free_mem ( f );
      return NULL;
    }

  if ( key == NULL )
    {
#ifdef VERBOSE
      error("formula_new", _("the key storage is invalid"));
#endif
      formula_free_mem(f);
      return NULL;
    }

  /* we prepare the formula */
  if ( ( f->str = mstr_clean(str, (1+strlen(str))) ) == NULL )
    {
#ifdef VERBOSE
      error("formula_new", _("unable to clean the formula"));
#endif
      formula_free_mem ( f );
      return NULL;
    }

  if ( ( f->head = mstr_eval(((FORMULA *) f)->str, strlen(((FORMULA *) f)->str)) ) == NULL )
    {
#ifdef VERBOSE
      error("formula_new", _("unable to build the tree"));
#endif
      formula_free_mem ( f );
      return NULL;
    }

  ++running_formulas;
  return ( f );
}


/*
 * Executes a formula tree.
 */
gdouble
formula_execute ( FORMULA *f )
{
  if ( f == NULL )
    return 0.0;

  if ( f->head == NULL )
    return 0.0;

  if ( f->head->data == NULL )
    return 0.0;

  return f->head->exec ( f->head->data );
}


/*
 * Dumps an XML description of a formula tree.
 */
void
formula_dump_xml_tree ( FORMULA *f,
                        FILE    *output )
{
  if ( f == NULL )
    return;

  if ( f->head == NULL )
    return;

  fprintf ( output, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<graph name=\"%s\">\n", f->str );
  f->head->dump_xml ( output, 0, f->head->data );
  fputs ( "</graph>\n", output );
  fflush ( output );
}


/*
 * Precalculates/Optimizes a formula tree.
 */
void
formula_precalc ( FORMULA *f )
{
  gint precalc_code;
  MATHS_VALUE *val;
  MATHS_TREE_ELEMENT *el;

  if ( f == NULL )
    return;

  if ( f->head == NULL )
    return;

  precalc_code = f->head->precalc ( f->head->data );

  if ( precalc_code == PRECALC_OK )
    {
      val = maths_val_alloc ( );
      val->value = (gdouble *) g_malloc ( sizeof(gdouble) );
      (*val->value) = f->head->exec ( f->head->data );

      el = (MATHS_TREE_ELEMENT *) g_malloc ( sizeof(MATHS_TREE_ELEMENT) );
      el->data = ( gpointer ) val;
      el->exec = maths_val_exec;
      el->dump_xml = maths_val_dump_xml;
      el->precalc = maths_val_precalc;
      el->free = maths_val_free;
      f->head->free ( f->head->data );
      g_free ( f->head );
      f->head = el;
    }
}


/*
 * Destroys a formula.
 */
void
formula_destroy ( FORMULA *f )
{
  if (f == NULL)
    return;

  --running_formulas;
  formula_free_mem ( f );
  return;
}
