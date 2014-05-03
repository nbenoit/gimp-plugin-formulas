/*
 * error.c
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


#include "config.h"
#include <libgimp/gimp.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "error.h"
#include "plugin-intl.h"


/*
 * Prints A Warning
 */
void
warning ( const gchar *prod,
          const gchar *format,
          ... )
{
  GimpMessageHandlerType t;
  gchar *message, *str;
  va_list ap;

  if ( format == NULL )
    return;

  str = ( gchar * ) g_malloc(512);
  message = ( gchar * ) g_malloc(512);

  t = gimp_message_get_handler ( );
  gimp_message_set_handler ( GIMP_ERROR_CONSOLE );

  va_start ( ap, format );
  g_vsnprintf ( message, 512, format, ap );
  va_end ( ap );

  if ( prod != NULL )
    g_snprintf ( str, 512, _("warning in %s(): %s.\n"), prod, message );
  else
    g_snprintf ( str, 512, _("warning: %s.\n"), message );

  gimp_message ( str );
  gimp_message_set_handler ( t );
  g_free ( message );
  g_free ( str );
}


/*
 * Prints An Error Message
 */
void
error ( const gchar *prod,
        const gchar *format,
        ... )
{
  GimpMessageHandlerType t;
  gchar *message, *str;
  va_list ap;

   if ( format == NULL )
    return;

  str = ( gchar * ) g_malloc ( 512 );
  message = (gchar *) g_malloc ( 512 );

  t = gimp_message_get_handler ( );
  gimp_message_set_handler ( GIMP_ERROR_CONSOLE );

  va_start ( ap, format );
  g_vsnprintf ( message, 512, format, ap );
  va_end ( ap );

  if ( prod != NULL )
    g_snprintf ( str, 512, _("error in %s(): %s.\n"), prod, message );
  else
    g_snprintf ( str, 512, _("error: %s.\n"), message );

  gimp_message ( str );
  gimp_message_set_handler ( t );
  g_free ( message );
  g_free ( str );
}
