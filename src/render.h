/*
 * render.h
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


#ifndef __RENDER_H__
#define __RENDER_H__


void render_to_gimpdrawable ( const gint32        image_ID,
                              PlugInDrawableVals *dvals,
                              PlugInVals         *vals );


void render_to_pixbuf ( GdkPixbuf          *pixbuf,
                        GdkPixbuf          *original,
                        PlugInDrawableVals *dvals,
                        PlugInVals         *vals,
                        const gdouble      aspect_ratio_w,
                        const gdouble      aspect_ratio_h,
                        gboolean           report_errors );


#endif
