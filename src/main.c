/*
 * main.c
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
#include <string.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include "main.h"
#include "interface.h"
#include "render.h"
#include "plugin-intl.h"


#define PROCEDURE_NAME   "plug_in_formulas"
#define DATA_KEY_VALS    "plug_in_formulas"
#define RELEASE_DATE     "14 November 2010"
#define DESCRIPTION      "This simple plugin allows you to change the values of each color channel of each pixel of a layer by using mathematical expressions."


const PlugInVals default_vals =
{
  "red(x,y)", "green(x,y)", "blue(x,y)", "gray(x,y)", "alpha(x,y)", TRUE
};

const PlugInDrawableVals default_dvals =
{
  NULL, 0, 0, 0, 0, FALSE, FALSE
};

static PlugInVals         vals;
static PlugInDrawableVals dvals;


/*
 * GIMP plugin query function.
 */
static void
query ( void )
{
  gchar *help_path;
  gchar *help_uri;

  static GimpParamDef args[] =
    {
      { GIMP_PDB_INT32,    "run_mode",      "Interactive, non-interactive"   },
      { GIMP_PDB_IMAGE,    "image",         "Input image"                    },
      { GIMP_PDB_DRAWABLE, "drawable",      "Input drawable"                 },
      { GIMP_PDB_STRING,   "red_channel",   "Formula for the red channel"    },
      { GIMP_PDB_STRING,   "green_channel", "Formula for the green channel"  },
      { GIMP_PDB_STRING,   "blue_channel",  "Formula for the blue channel"   },
      { GIMP_PDB_STRING,   "gray_channel",  "Formula for the blue channel"   },
      { GIMP_PDB_STRING,   "alpha_channel", "Formula for the alpha channel"  }
    };

  gimp_plugin_domain_register ( PLUGIN_NAME, LOCALEDIR );

  help_path = g_build_filename ( DATADIR, "help", NULL );
  help_uri = g_filename_to_uri ( help_path, NULL, NULL );
  g_free ( help_path );

  gimp_plugin_help_register ( "http://developer.gimp.org/formulas/help", help_uri );

  gimp_install_procedure ( PROCEDURE_NAME,
                           DESCRIPTION,
                           DESCRIPTION,
                           "Nicolas BENOIT <nbenoit@tuxfamily.org>",
                           "2004-2010 Nicolas BENOIT", RELEASE_DATE,
                           N_("<Image>/Filters/Render/F_ormulas..."), "RGB*, GRAY*",
                           GIMP_PLUGIN, G_N_ELEMENTS(args), 0, args, NULL );
}


/*
 * GIMP plugin run function.
 */
static void
run ( const gchar      *name,
      gint              n_params,
      const GimpParam  *param,
      gint             *nreturn_vals,
      GimpParam       **return_vals )
{
  static GimpParam   values[1];
  gint32             image_ID;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;

  /* initializes i18n support */
  bindtextdomain ( GETTEXT_PACKAGE, LOCALEDIR );
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
#endif
  textdomain ( GETTEXT_PACKAGE );

  run_mode = param[0].data.d_int32;
  image_ID = param[1].data.d_int32;

  /* initializes with default values */
  vals = default_vals;
  dvals = default_dvals;
  dvals.drawable = gimp_drawable_get ( param[2].data.d_drawable );
  dvals.width = gimp_drawable_width ( dvals.drawable->drawable_id );
  dvals.height = gimp_drawable_height ( dvals.drawable->drawable_id );
  dvals.has_alpha = gimp_drawable_has_alpha ( dvals.drawable->drawable_id );
  dvals.is_rgb = gimp_drawable_is_rgb ( dvals.drawable->drawable_id );
  dvals.size = ( dvals.width * dvals.height * (1 + ((2 * ((gint32) (dvals.is_rgb)))) + ((gint32) (dvals.has_alpha))) );

  if ( ( strcmp(name, PROCEDURE_NAME) ) == 0 )
    {
      switch (run_mode)
        {
        case GIMP_RUN_NONINTERACTIVE:
          if (dvals.is_rgb)
            {
              g_strlcpy ( vals.str_red_chan, param[3].data.d_string, 1+(MIN(strlen(param[3].data.d_string), FORMULA_STR_MAX_LEN)));
              g_strlcpy ( vals.str_green_chan, param[4].data.d_string, 1+(MIN(strlen(param[4].data.d_string), FORMULA_STR_MAX_LEN)));
              g_strlcpy ( vals.str_blue_chan, param[5].data.d_string, 1+(MIN(strlen(param[5].data.d_string), FORMULA_STR_MAX_LEN)));
            }
          else
            {
              g_strlcpy ( vals.str_gray_chan, param[5].data.d_string, 1+(MIN(strlen(param[6].data.d_string), FORMULA_STR_MAX_LEN)) );
            }

          if ( dvals.has_alpha )
            g_strlcpy ( vals.str_alpha_chan, param[6].data.d_string, 1+(MIN(strlen(param[7].data.d_string), FORMULA_STR_MAX_LEN)) );

          break;

        case GIMP_RUN_INTERACTIVE:
          gimp_get_data ( DATA_KEY_VALS, &vals );

          if ( !dialog(image_ID, &dvals, &vals) )
            status = GIMP_PDB_CANCEL;
          break;

        case GIMP_RUN_WITH_LAST_VALS:
          gimp_get_data ( DATA_KEY_VALS, &vals );
          break;

        default:
          break;
        }
    }
  else
    {
      status = GIMP_PDB_CALLING_ERROR;
    }

  if ( run_mode == GIMP_RUN_INTERACTIVE )
    gimp_set_data ( DATA_KEY_VALS, &vals, sizeof(vals) );

  if ( status == GIMP_PDB_SUCCESS )
    {
      render_to_gimpdrawable ( image_ID, &dvals, &vals );
      gimp_displays_flush ( );
      gimp_drawable_detach ( dvals.drawable );
    }

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}


/*
 * GIMP plugin information structure.
 */
GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


/*
 * GIMP plugin main function.
 */
MAIN ()
