/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2017 Yu YongZhen <yuyz@rock-chips.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_IEPDEINTERLACE_H__
#define __GST_IEPDEINTERLACE_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

typedef enum {
  GST_IEP_DEINTERLACE_LAYOUT_TFF,
  GST_IEP_DEINTERLACE_LAYOUT_BFF
} GstIepDeinterlaceFieldLayout;

typedef enum {
    GST_IEP_DEINTERLACE_METHOD_DISABLE,
    GST_IEP_DEINTERLACE_METHOD_I2O1,
    GST_IEP_DEINTERLACE_METHOD_I4O1,
    GST_IEP_DEINTERLACE_METHOD_I4O2,
    GST_IEP_DEINTERLACE_METHOD_BYPASS
} GstIepDeinterlaceMethod;

/* #defines don't like whitespacey bits */
#define GST_TYPE_IEPDEINTERLACE \
  (gst_iep_deinterlace_get_type())
#define GST_IEPDEINTERLACE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_IEPDEINTERLACE,GstIepDeinterlace))
#define GST_IEPDEINTERLACE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_IEPDEINTERLACE,GstIepDeinterlaceClass))
#define GST_IS_IEPDEINTERLACE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_IEPDEINTERLACE))
#define GST_IS_IEPDEINTERLACE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_IEPDEINTERLACE))

typedef struct _GstIepDeinterlace      GstIepDeinterlace;
typedef struct _GstIepDeinterlaceClass GstIepDeinterlaceClass;

struct _GstIepDeinterlace
{
  GstVideoFilter videofilter;

  GstPad *sinkpad, *srcpad;

  gboolean silent;

  GstIepDeinterlaceMethod method;

  GstIepDeinterlaceFieldLayout field_layout;
};

struct _GstIepDeinterlaceClass 
{
  GstVideoFilterClass parent_class;
};

GType gst_iep_deinterlace_get_type (void);

G_END_DECLS

#endif /* __GST_IEPDEINTERLACE_H__ */
