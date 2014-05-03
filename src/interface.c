/*
 * interface.c
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
#include <libgimp/gimpui.h>
#include <string.h>
#include "main.h"
#include "error.h"
#include "render.h"
#include "interface.h"
#include "plugin-intl.h"


#define PREVIEW_WIDTH  128
#define PREVIEW_HEIGHT 128


static gboolean   auto_update_preview = FALSE;
static GtkWidget *text_red_chan;
static GtkWidget *text_gray_chan;
static GtkWidget *text_green_chan;
static GtkWidget *text_blue_chan;
static GtkWidget *text_alpha_chan;
static GtkWidget *preview;
static GdkPixbuf *preview_pixbuf;
static GdkPixbuf *original_small_pixbuf;
static gdouble    aspect_ratio_w;
static gdouble    aspect_ratio_h;
static PlugInDrawableVals preview_dvals;


/*
 * Reads formulas from text entries.
 */
static void
get_formulas ( PlugInDrawableVals *dvals,
               PlugInVals         *vals )
{
  /* we can use g_stpscpy() safely, because we defined the maximum size of the text entry */

  if ( dvals->is_rgb )
    {
      g_stpcpy ( vals->str_red_chan, gtk_entry_get_text(GTK_ENTRY(text_red_chan)) );
      g_stpcpy ( vals->str_green_chan, gtk_entry_get_text(GTK_ENTRY(text_green_chan)) );
      g_stpcpy ( vals->str_blue_chan, gtk_entry_get_text(GTK_ENTRY(text_blue_chan)) );
    }
  else
    g_stpcpy ( vals->str_gray_chan, gtk_entry_get_text(GTK_ENTRY(text_gray_chan)) );

  if ( dvals->has_alpha )
    g_stpcpy ( vals->str_alpha_chan, gtk_entry_get_text(GTK_ENTRY(text_alpha_chan)) );
}


/*
 * Resets formulas in text entries.
 */
static void
reset_formulas ( PlugInDrawableVals *dvals,
                 PlugInVals         *vals )
{
  if ( dvals->is_rgb )
    {
      g_stpcpy ( vals->str_red_chan, default_vals.str_red_chan );
      g_stpcpy ( vals->str_green_chan, default_vals.str_green_chan );
      g_stpcpy ( vals->str_blue_chan, default_vals.str_blue_chan );

      gtk_entry_set_text ( GTK_ENTRY(text_red_chan), vals->str_red_chan );
      gtk_entry_set_text ( GTK_ENTRY(text_green_chan), vals->str_green_chan );
      gtk_entry_set_text ( GTK_ENTRY(text_blue_chan), vals->str_blue_chan );
    }
  else
    {
      g_stpcpy ( vals->str_gray_chan, default_vals.str_gray_chan );
      gtk_entry_set_text ( GTK_ENTRY(text_gray_chan), vals->str_gray_chan );
    }

  if ( dvals->has_alpha )
    {
      g_stpcpy ( vals->str_alpha_chan, default_vals.str_alpha_chan );
      gtk_entry_set_text ( GTK_ENTRY(text_alpha_chan), vals->str_alpha_chan );
    }
}


/*
 * Creates the preview pixel buffer.
 */
static GdkPixbuf *
make_preview_pixbuf ( PlugInDrawableVals *dvals )
{
  GdkPixbuf *pixbuf;

  if ( ( pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, PREVIEW_WIDTH, PREVIEW_HEIGHT) ) == NULL )
    return NULL;

  gdk_pixbuf_fill(pixbuf, 0);

  return pixbuf;
}


/*
 * Creates the pixel buffer of the original image.
 */
static GdkPixbuf *
make_original_small_pixbuf ( PlugInDrawableVals *dvals )
{
  GimpPixelRgn  in_pr;
  GdkPixbuf    *pixbuf;
  GdkPixbuf    *new_pixbuf;
  guchar       *in_image, *in_ptr, *pixbuf_pixels, *row_ptr, *ptr;
  gint          rowstride;

  if ( ( pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, dvals->width, dvals->height) ) == NULL )
    return NULL;

  memcpy ( &preview_dvals, dvals, sizeof(PlugInDrawableVals) );
  preview_dvals.width = PREVIEW_WIDTH;
  preview_dvals.height = PREVIEW_HEIGHT;

  pixbuf_pixels = gdk_pixbuf_get_pixels ( pixbuf );
  rowstride = gdk_pixbuf_get_rowstride ( pixbuf );

  /* we calculate some aspect ratios */
  aspect_ratio_w = ( (gdouble) dvals->width / (gdouble) PREVIEW_WIDTH );
  aspect_ratio_h = ( (gdouble) dvals->height / (gdouble) PREVIEW_HEIGHT );

  /* we copy the gimp drawable into a pixbuf (if someone knows another way to do it....) */
  in_image = g_new ( guchar, dvals->size );
  in_ptr = in_image;
  gimp_pixel_rgn_init ( &in_pr, dvals->drawable, 0, 0, dvals->width, dvals->height, FALSE, FALSE );
  gimp_pixel_rgn_get_rect ( &in_pr, in_image, 0, 0, dvals->width, dvals->height );

  /* we separate the loops, so we don't waste time testing for alpha and rgb */
  if ( dvals->is_rgb )
    {
      const gint col_size = dvals->height*rowstride;
      const gint row_size = dvals->width*3*sizeof(guchar);

      if (dvals->has_alpha)
        {
          for ( row_ptr=pixbuf_pixels; row_ptr<(pixbuf_pixels+col_size); row_ptr+=rowstride )
            for ( ptr=row_ptr; ptr<(row_ptr+row_size); ptr+=3, in_ptr+=4 )
              memcpy ( ptr, in_ptr, 3*sizeof(guchar) );
        }
      else
        {
          for ( row_ptr=pixbuf_pixels; row_ptr<(pixbuf_pixels+col_size); row_ptr+=rowstride, in_ptr+=row_size )
            memcpy ( row_ptr, in_ptr, row_size );
        }
    }
  else
    {
      const gint col_size = dvals->height*rowstride;
      const gint row_size = dvals->width*3*sizeof(guchar);

      if ( dvals->has_alpha )
        {
          for ( row_ptr=pixbuf_pixels; row_ptr<(pixbuf_pixels+col_size); row_ptr+=rowstride )
            for ( ptr=row_ptr; ptr<(row_ptr+row_size); ptr+=3, in_ptr+=2 )
              memset ( ptr, *in_ptr, 3*sizeof(guchar) );
        }
      else
        {
          for ( row_ptr=pixbuf_pixels; row_ptr<(pixbuf_pixels+col_size); row_ptr+=rowstride )
            for ( ptr=row_ptr; ptr<(row_ptr+row_size); ptr+=3, ++in_ptr )
              memset ( ptr, *in_ptr, 3*sizeof(guchar) );
        }
    }

  if ( ( new_pixbuf = gdk_pixbuf_scale_simple(pixbuf, PREVIEW_WIDTH, PREVIEW_HEIGHT, GDK_INTERP_TILES) ) == NULL )
    return NULL;
  
  g_free ( in_image );
  g_object_unref ( pixbuf );
  return new_pixbuf;
}


/*
 * Updates the preview.
 */
static gboolean
update_preview ( GtkWidget *widget, 
                 gpointer   data )
{
  PlugInVals vals = default_vals ;
  get_formulas ( &preview_dvals, &vals );

  /* if the widget is null, then this is not a callback... we do not report errors */
  if ( widget == NULL )
    {
      render_to_pixbuf ( preview_pixbuf, original_small_pixbuf, 
                         &preview_dvals, &vals,
                         aspect_ratio_w, aspect_ratio_h,
                         FALSE );
    }
  else
    {
      render_to_pixbuf ( preview_pixbuf, original_small_pixbuf,
                         &preview_dvals, &vals,
                         aspect_ratio_w, aspect_ratio_h,
                         TRUE );
    }

  gtk_widget_queue_draw ( preview );
  return TRUE;
}


/*
 * Listens the changes of the auto-update-preview toggle.
 */
static gboolean
auto_update_preview_toggled ( GtkWidget *widget, 
                              gpointer   data )
{
  if ( GTK_TOGGLE_BUTTON(widget)->active )
    {
      auto_update_preview = TRUE;
      update_preview(NULL, NULL);
    }
  else
    auto_update_preview = FALSE;

  return TRUE;
}


/*
 * Listens the changes of the text entries.
 */
static gboolean
txt_changed ( GtkWidget *widget, 
              gpointer   data )
{
  if (auto_update_preview)
    update_preview(NULL, NULL);

  return TRUE;
}


/*
 * Creates the plugin dialog.
 */
gboolean
dialog ( gint32               image_ID,
         PlugInDrawableVals  *dvals,
         PlugInVals          *vals )
{
  GtkWidget *dlg;
  GtkWidget *main_hbox;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *button;
  gint       response;

  gimp_ui_init(PLUGIN_NAME, TRUE);

  dlg = gimp_dialog_new (_("Formulas Rendering Plug-In"), PLUGIN_NAME,
                         NULL, GTK_DIALOG_NO_SEPARATOR,
                         gimp_standard_help_func, "plug-in-formulas",
                         GTK_STOCK_HELP,   GTK_RESPONSE_HELP,
                         GIMP_STOCK_RESET, RESPONSE_RESET,
                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                         GTK_STOCK_OK,     GTK_RESPONSE_OK,
                         NULL);

  auto_update_preview = vals->auto_update_preview;

  /* top-level box */
  main_hbox = gtk_hbox_new(FALSE, 1);
  gtk_container_set_border_width(GTK_CONTAINER(main_hbox), 2);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG (dlg)->vbox), main_hbox);

  /* preview's frame */
  frame = gtk_frame_new(_("Preview"));
  gtk_container_set_border_width(GTK_CONTAINER (frame), 6);
  gtk_box_pack_start(GTK_BOX(main_hbox), frame, FALSE, TRUE, 0);
  gtk_widget_show(frame);

  /* preview's table */
  table = gtk_table_new(1, 3, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table), 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_table_set_row_spacing(GTK_TABLE (table), 0, 4);
  gtk_container_set_border_width(GTK_CONTAINER(table), 6);
  gtk_container_add(GTK_CONTAINER(frame), table);
  gtk_widget_show(table);

  /* preview image */
  if ((original_small_pixbuf = make_original_small_pixbuf(dvals)) == NULL)
    {
      error(NULL, _("Unable to make the small pixbuf of the original image."));
      return 0;
    }

  if ((preview_pixbuf = make_preview_pixbuf(dvals)) == NULL)
    {
      error(NULL, _("Unable to make the preview pixbuf."));
      return 0;
    }

  preview = gtk_image_new_from_pixbuf(preview_pixbuf);
  gtk_widget_set_size_request(preview, PREVIEW_WIDTH, PREVIEW_HEIGHT);
  gtk_table_attach_defaults(GTK_TABLE(table), preview, 0, 1, 0, 1);

  /* automatic preview update toggle */
  button = gtk_check_button_new_with_label(_("Automatic Update"));
  gimp_help_set_help_data (button, _("Enable/Disable the automatic update of the preview"), NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), auto_update_preview);
  g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(auto_update_preview_toggled), NULL);
  gtk_table_attach(GTK_TABLE(table), button, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
  gtk_widget_show(button);

  /* preview update button */
  button = gtk_button_new_with_label(_("Update"));
  gimp_help_set_help_data (button, _("Update the preview"), NULL);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(update_preview), NULL);
  gtk_table_attach(GTK_TABLE(table), button, 0, 1, 2, 3, GTK_FILL, 0, 0, 0); 
  gtk_widget_show(button);

  /* formulas' frame */
  frame = gtk_frame_new(_("Formulas"));
  gtk_container_set_border_width(GTK_CONTAINER (frame), 6);
  gtk_box_pack_start(GTK_BOX(main_hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show(frame);

  /* formula's table */
  table = gtk_table_new(2, 4, FALSE);
  gtk_table_set_homogeneous(GTK_TABLE(table), FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table), 4);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_table_set_row_spacing(GTK_TABLE (table), 0, 4);
  gtk_container_set_border_width(GTK_CONTAINER(table), 6);
  gtk_container_add(GTK_CONTAINER(frame), table);
  gtk_widget_show(table);

  /* labels and text entries */
  if (dvals->is_rgb)
    {
      label = gtk_label_new(_("Red Channel:"));
      gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
      gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, 0, 0);
      gtk_widget_show(label);

      label = gtk_label_new(_("Green Channel:"));
      gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
      gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2, 0, 0, 0, 0);
      gtk_widget_show(label);

      label = gtk_label_new(_("Blue Channel:"));
      gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
      gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, 0, 0, 0, 0);
      gtk_widget_show(label);
    }
  else
    {
      label = gtk_label_new(_("Gray Channel:"));
      gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
      gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, 0, 0, 0, 0);
      gtk_widget_show(label);
    }

  label = gtk_label_new(_("Alpha Channel:"));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 3, 4, 0, 0, 0, 0);
  gtk_widget_show(label);

  if (dvals->is_rgb)
    {
      text_red_chan = gtk_entry_new_with_max_length(FORMULA_STR_MAX_LEN);
      gtk_table_attach(GTK_TABLE(table), text_red_chan, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
      gimp_help_set_help_data (text_red_chan, _("The formula for the red channel of the image"), NULL);
      gtk_entry_set_text(GTK_ENTRY(text_red_chan), vals->str_red_chan);
      g_signal_connect(G_OBJECT(text_red_chan), "changed", G_CALLBACK(txt_changed), NULL);
      gtk_widget_show(text_red_chan);

      text_green_chan = gtk_entry_new_with_max_length(FORMULA_STR_MAX_LEN);
      gtk_table_attach(GTK_TABLE(table), text_green_chan, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
      gimp_help_set_help_data (text_green_chan, _("The formula for the green channel of the image"), NULL);
      gtk_entry_set_text(GTK_ENTRY(text_green_chan), vals->str_green_chan);
      g_signal_connect(G_OBJECT(text_green_chan), "changed", G_CALLBACK(txt_changed), NULL);
      gtk_widget_show(text_green_chan);

      text_blue_chan = gtk_entry_new_with_max_length(FORMULA_STR_MAX_LEN);
      gtk_table_attach(GTK_TABLE(table), text_blue_chan, 1, 2, 2, 3, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
      gimp_help_set_help_data (text_blue_chan, _("The formula for the blue channel of the image"), NULL);
      gtk_entry_set_text(GTK_ENTRY(text_blue_chan), vals->str_blue_chan);
      g_signal_connect(G_OBJECT(text_blue_chan), "changed", G_CALLBACK(txt_changed), NULL);
      gtk_widget_show(text_blue_chan);
    }
  else
    {
      text_red_chan = NULL;
      text_blue_chan = NULL;
      text_green_chan = NULL;
      text_gray_chan = gtk_entry_new_with_max_length(FORMULA_STR_MAX_LEN);
      gtk_table_attach(GTK_TABLE(table), text_gray_chan, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
      gimp_help_set_help_data (text_gray_chan, _("The formula for the gray channel of the image"), NULL);
      gtk_entry_set_text(GTK_ENTRY(text_gray_chan), vals->str_gray_chan);
      g_signal_connect(G_OBJECT(text_gray_chan), "changed", G_CALLBACK(txt_changed), NULL);
      gtk_widget_show(text_gray_chan);
    }

  /* alpha channel */
  text_alpha_chan = gtk_entry_new_with_max_length(FORMULA_STR_MAX_LEN);
  gtk_table_attach(GTK_TABLE(table), text_alpha_chan, 1, 2, 3, 4, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0 );
  gimp_help_set_help_data (text_alpha_chan, _("The formula for the alpha channel of the image"), NULL);
  gtk_entry_set_text(GTK_ENTRY(text_alpha_chan), vals->str_alpha_chan);

  if (!dvals->has_alpha)
    gtk_entry_set_editable(GTK_ENTRY(text_alpha_chan), FALSE);
  else
    g_signal_connect(G_OBJECT(text_alpha_chan), "changed", G_CALLBACK(txt_changed), NULL);

  gtk_widget_show(text_alpha_chan);

  /* user can see the dialog */
  update_preview(NULL, NULL);
  gtk_widget_show(preview);
  gtk_widget_show(main_hbox);
  gtk_widget_show(dlg);

  /* let's roll */
  do
    {
      response = gimp_dialog_run ( GIMP_DIALOG(dlg) );

      if ( response == GTK_RESPONSE_HELP )
        gimp_help("http://developer.gimp.org/formulas/help", "plug-in-formulas");
      else if ( response == RESPONSE_RESET )
        reset_formulas ( dvals, vals );
    }
  while ( ( response == GTK_RESPONSE_HELP ) || ( response == RESPONSE_RESET ) );

  /* we get the formulas */
  get_formulas(dvals, vals);

  /* that's all folks! */
  vals->auto_update_preview = auto_update_preview;
  gtk_widget_destroy ( dlg );
  g_object_unref ( preview_pixbuf );

  return (response == GTK_RESPONSE_OK);
}
