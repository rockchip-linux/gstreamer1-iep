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

/**
 * SECTION:element-iepdeinterlace
 *
 * FIXME:Describe iepdeinterlace here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 v4l2src num-buffers=300 ! video/x-raw, format=NV12, width=704, height=480 ! iepdeinterlace ! filesink location=/tmp/yuv.raw
 * ]|
 * |[
 * gst-launch-1.0 videotestsrc num-buffers=300 ! video/x-raw, format=NV12, width=704, height=480 ! iepdeinterlace ! filesink location=/tmp/yuv.raw
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstiepdeinterlace.h"
#include "iep_api.h"

GST_DEBUG_CATEGORY_STATIC (gst_iep_deinterlace_debug);
#define GST_CAT_DEFAULT gst_iep_deinterlace_debug

#define DEFAULT_METHOD GST_IEP_DEINTERLACE_METHOD_I2O1
#define DEFAULT_FIELD_LAYOUT GST_IEP_DEINTERLACE_LAYOUT_TFF

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_METHOD,
  PROP_FIELD_LAYOUT
};

static const GEnumValue methods_types[] = {
  /*{GST_IEP_DEINTERLACE_METHOD_DISABLE, "Disable Iep Deinterlace",
      "disable"},*/
  {GST_IEP_DEINTERLACE_METHOD_I2O1, "Iep Deinterlace Input 2 Fields Output 1 Frame",
      "I2O1"},
  /*
  {GST_IEP_DEINTERLACE_METHOD_I4O1, "Iep Deinterlace Input 4 Fields Output 1 Frame",
      "I4O1"},
  {GST_IEP_DEINTERLACE_METHOD_I4O2, "Iep Deinterlace Input 4 Fields Output 2 Frames",
      "I4O2"},*/
  {GST_IEP_DEINTERLACE_METHOD_BYPASS, "Iep Deinterlace Bypass",
      "Bypass"},
  {0, NULL, NULL},
};

#define GST_TYPE_IEP_DEINTERLACE_METHODS (gst_iep_deinterlace_methods_get_type ())
static GType
gst_iep_deinterlace_methods_get_type (void)
{
  static GType iep_deinterlace_methods_type = 0;

  if (!iep_deinterlace_methods_type) {
    iep_deinterlace_methods_type =
        g_enum_register_static ("GstIepDeinterlaceMethods", methods_types);
  }
  return iep_deinterlace_methods_type;
}

#define GST_TYPE_IEP_DEINTERLACE_FIELD_LAYOUT (gst_iep_deinterlace_field_layout_get_type ())
static GType
gst_iep_deinterlace_field_layout_get_type (void)
{
  static GType iep_deinterlace_field_layout_type = 0;

  static const GEnumValue field_layout_types[] = {
    {GST_IEP_DEINTERLACE_LAYOUT_TFF, "Top field first", "tff"},
    {GST_IEP_DEINTERLACE_LAYOUT_BFF, "Bottom field first", "bff"},
    {0, NULL, NULL},
  };

  if (!iep_deinterlace_field_layout_type) {
    iep_deinterlace_field_layout_type =
        g_enum_register_static ("GstDeinterlaceFieldLayout",
        field_layout_types);
  }
  return iep_deinterlace_field_layout_type;
}

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ NV12, NV21 }"))
    /*GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ AYUV, "
            "ARGB, BGRA, ABGR, RGBA, Y444, "
            "xRGB, RGBx, xBGR, BGRx, RGB, BGR, Y42B, NV12, "
            "NV21, YUY2, UYVY, YVYU, I420, YV12, IYUV, Y41B }"))*/
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ NV12, NV21 }"))
    /*GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ AYUV, "
            "ARGB, BGRA, ABGR, RGBA, Y444, "
            "xRGB, RGBx, xBGR, BGRx, RGB, BGR, Y42B, NV12, "
            "NV21, YUY2, UYVY, YVYU, I420, YV12, IYUV, Y41B }"))*/
    );

#define gst_iep_deinterlace_parent_class parent_class
G_DEFINE_TYPE (GstIepDeinterlace, gst_iep_deinterlace, GST_TYPE_VIDEO_FILTER);

static void gst_iep_deinterlace_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_iep_deinterlace_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static gboolean gst_iep_deinterlace_set_info (GstVideoFilter * vfilter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_iep_deinterlace_transform_frame (GstVideoFilter * vfilter,
    GstVideoFrame * in_frame, GstVideoFrame * out_frame);
static gboolean gst_iep_deinterlace_start (GstBaseTransform * trans);
static gboolean gst_iep_deinterlace_stop (GstBaseTransform * trans);
/* GObject vmethod implementations */

/* initialize the iepdeinterlace's class */
static void
gst_iep_deinterlace_class_init (GstIepDeinterlaceClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;
  GstBaseTransformClass *trans_class = (GstBaseTransformClass *) klass;
  GstVideoFilterClass *vfilter_class = (GstVideoFilterClass *) klass;

  gobject_class->set_property = gst_iep_deinterlace_set_property;
  gobject_class->get_property = gst_iep_deinterlace_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
    "IepDeinterlace",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    "Yu YongZhen <yuyz@rock-chips.com>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));

  /**
   * GstDeinterlace:method:
   *
   * This select which method used to deinterlace frame.
   */
  g_object_class_install_property (gobject_class, PROP_METHOD,
      g_param_spec_enum ("method",
          "Method",
          "IEP Deinterlace Method",
          GST_TYPE_IEP_DEINTERLACE_METHODS,
          DEFAULT_METHOD, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
      );
  /**
   * GstDeinterlace:layout:
   *
   * This selects which fields is the first in time.
   *
   */
  g_object_class_install_property (gobject_class, PROP_FIELD_LAYOUT,
      g_param_spec_enum ("tff",
          "tff",
          "IEP Deinterlace top field first",
          GST_TYPE_IEP_DEINTERLACE_FIELD_LAYOUT,
          DEFAULT_FIELD_LAYOUT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)
      );

  trans_class->start = GST_DEBUG_FUNCPTR (gst_iep_deinterlace_start);
  trans_class->stop = GST_DEBUG_FUNCPTR (gst_iep_deinterlace_stop);
  vfilter_class->set_info = GST_DEBUG_FUNCPTR (gst_iep_deinterlace_set_info);
  vfilter_class->transform_frame =
      GST_DEBUG_FUNCPTR (gst_iep_deinterlace_transform_frame);
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_iep_deinterlace_init (GstIepDeinterlace * self)
{
  gint i;

  self->silent = FALSE;
  self->method = DEFAULT_METHOD;
  self->field_layout = DEFAULT_FIELD_LAYOUT;

  for (i = 0; i < MPP_MAX_BUFFERS; i++) {
    self->input_buffer[i] = NULL;
    self->output_buffer[i] = NULL;
  }

  self->input_group = NULL;
  self->output_group = NULL;
}

static void
gst_iep_deinterlace_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstIepDeinterlace *filter = GST_IEPDEINTERLACE (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_METHOD:
      filter->method = g_value_get_enum (value);
      break;
    case PROP_FIELD_LAYOUT:
      filter->field_layout = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_iep_deinterlace_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstIepDeinterlace *filter = GST_IEPDEINTERLACE (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_METHOD:
      g_value_set_enum (value, filter->method);
      break;
    case PROP_FIELD_LAYOUT:
      g_value_set_enum (value, filter->field_layout);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */
static gboolean
gst_iep_deinterlace_set_info (GstVideoFilter * vfilter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstIepDeinterlace *self = GST_IEPDEINTERLACE (vfilter);

  GST_DEBUG_OBJECT (self,
      "setting caps: in %" GST_PTR_FORMAT " out %" GST_PTR_FORMAT, incaps,
      outcaps);

  switch (GST_VIDEO_INFO_FORMAT (in_info)) {
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_NV21:
      break;
    default:
      goto invalid_caps;
      break;
  }
  return TRUE;

  /* ERRORS */
invalid_caps:
  {
    GST_ERROR_OBJECT (self, "Invalid caps: %" GST_PTR_FORMAT, incaps);
    return FALSE;
  }
}

static void
gst_iep_alloc_internal_buffer(GstIepDeinterlace *self)
{
  GstVideoInfo *info = &GST_VIDEO_FILTER (self)->in_info;

  if (self->input_group == NULL) {
    gint i = 0;
    gint size = 0;

    GST_DEBUG_OBJECT (self, "Filling src caps with output dimensions %ux%u,size %u",
        info->width, info->height, info->size);

    if (mpp_buffer_group_get_internal (&self->input_group, MPP_BUFFER_TYPE_ION))
      goto activate_failed;
    if (mpp_buffer_group_get_internal (&self->output_group,
            MPP_BUFFER_TYPE_ION))
      goto activate_failed;

    for (i = 0; i < MPP_MAX_BUFFERS; i++) {
      if (mpp_buffer_get (self->input_group, &self->input_buffer[i],
              info->size))
        goto activate_failed;
      if (mpp_buffer_get (self->output_group, &self->output_buffer[i],
              info->size))
        goto activate_failed;
    }
  }
  return;

activate_failed:
  GST_ERROR_OBJECT (self, "Alloc Buffer Failed");
  return;
}
static void
gst_iep_process (GstIepDeinterlace *self, GstVideoFrame * in_frame, GstVideoFrame * out_frame)
{
  gint in_width, in_height, in_stride, in_row_wrap;
  gint out_width, out_height, out_stride, out_row_wrap;
  GstVideoFormat in_fmt, out_fmt;
  guint8 *in_data, *out_data;
  struct iep_ops *iep_ops;
  void *iep_obj;
  iep_img src, dst;
  iep_param_yuv_deinterlace_t yuv_dil;
  MppBuffer frame_in;
  MppBuffer frame_out;
  unsigned long src_phy_addr;
  unsigned long src_vir_addr;
  unsigned long dst_phy_addr;
  unsigned long dst_vir_addr;
  int ret;

  in_fmt = GST_VIDEO_FRAME_FORMAT (in_frame);
  in_data = GST_VIDEO_FRAME_COMP_DATA (in_frame, 0);
  in_stride = GST_VIDEO_FRAME_COMP_STRIDE (in_frame, 0);
  in_width = GST_VIDEO_FRAME_COMP_WIDTH (in_frame, 0);
  in_height = GST_VIDEO_FRAME_COMP_HEIGHT (in_frame, 0);
  in_row_wrap = in_stride - in_width;

  out_fmt = GST_VIDEO_FRAME_FORMAT (out_frame);
  out_data = GST_VIDEO_FRAME_COMP_DATA (out_frame, 0);
  out_stride = GST_VIDEO_FRAME_COMP_STRIDE (out_frame, 0);
  out_width = GST_VIDEO_FRAME_COMP_WIDTH (out_frame, 0);
  out_height = GST_VIDEO_FRAME_COMP_HEIGHT (out_frame, 0);
  out_row_wrap = out_stride - out_width;

  /*input & ouput buffer check*/
  if (in_width != out_width || in_height != out_height || in_fmt != out_fmt) {
    GST_ERROR_OBJECT (self, "in out frame size not equal");
    return GST_FLOW_ERROR;
  }

  if (!self->input_group)
    gst_iep_alloc_internal_buffer(self);

  frame_in = self->input_buffer[0];
  frame_out = self->output_buffer[0];

  if (!frame_in || !frame_out) {
    GST_ERROR_OBJECT (self, "no internal buffer for iep");
    return GST_FLOW_ERROR;
  }

  src_phy_addr = mpp_buffer_get_fd(frame_in);
  src_vir_addr = mpp_buffer_get_ptr(frame_in);
  dst_phy_addr = mpp_buffer_get_fd(frame_out);
  dst_vir_addr = mpp_buffer_get_ptr(frame_out);

  memcpy(src_vir_addr, in_data, GST_VIDEO_FRAME_SIZE(in_frame));

  memset(&src, 0, sizeof(iep_img));
  memset(&dst, 0, sizeof(iep_img));
 
  src.act_w = in_width;
  src.act_h = in_height;
  src.x_off = 0;
  src.y_off = 0;
  src.vir_w = in_width;
  src.vir_h = in_height;
  src.format = IEP_FORMAT_YCrCb_420_SP;
  src.mem_addr = src_phy_addr;
  src.uv_addr  = src_phy_addr | (in_width * in_height << 10);

  dst.act_w = in_width;
  dst.act_h = in_height;
  dst.x_off = 0;
  dst.y_off = 0;
  dst.vir_w = in_width;
  dst.vir_h = in_height;
  dst.format = IEP_FORMAT_YCrCb_420_SP;
  dst.mem_addr = dst_phy_addr;
  dst.uv_addr = dst_phy_addr | (in_width * in_height << 10);

  iep_ops = alloc_iep_ops();
  if (!iep_ops)
    return GST_FLOW_ERROR;
  iep_obj = iep_ops->claim();
  if (!iep_obj)
    return GST_FLOW_ERROR;

  iep_ops->init(iep_obj, &src, &dst);
 
  yuv_dil.high_freq_en = 1;
  yuv_dil.dil_mode = IEP_DEINTERLACE_MODE_I2O1;
  yuv_dil.field_order = FIELD_ORDER_BOTTOM_FIRST;
  yuv_dil.dil_high_freq_fct = 50;
  yuv_dil.dil_ei_mode = 1;
  yuv_dil.dil_ei_smooth = 1;
  yuv_dil.dil_ei_sel = 0;
  yuv_dil.dil_ei_radius = 2;

  ret = iep_ops->config_yuv_deinterlace_param1(iep_obj, &yuv_dil);

  if (ret < 0) {
    goto ERR;
  }

  ret = iep_ops->run_sync(iep_obj);
  if (ret < 0) {
    goto ERR;
  }
  iep_ops->reclaim(iep_obj);
  free_iep_ops(iep_ops);

  memcpy(out_data, dst_vir_addr, GST_VIDEO_FRAME_SIZE(out_frame));
  return;

ERR:
  GST_ERROR_OBJECT (self, "iep deinterlace failed");
  iep_ops->reclaim(iep_obj);
  free_iep_ops(iep_ops);
}

static GstFlowReturn
gst_iep_deinterlace_transform_frame (GstVideoFilter * vfilter,
    GstVideoFrame * in_frame, GstVideoFrame * out_frame)
{
  GstIepDeinterlace *self = GST_IEPDEINTERLACE (vfilter);
 
  if (0)
    gst_video_frame_copy(out_frame, in_frame);
  else
    gst_iep_process(self, in_frame, out_frame);

  return GST_FLOW_OK;
}

static gboolean
gst_iep_deinterlace_start (GstBaseTransform * trans)
{
  return TRUE;
}

static gboolean
gst_iep_deinterlace_stop (GstBaseTransform * trans)
{
  gint i;
  GstIepDeinterlace *self = GST_IEPDEINTERLACE (trans);

  /*free mpp buffer*/
  for (i = 0; i < MPP_MAX_BUFFERS; i++) {
    if (self->input_buffer[i]) {
      mpp_buffer_put (self->input_buffer[i]);
      self->input_buffer[i] = NULL;
    }
    if (self->output_buffer[i]) {
      mpp_buffer_put (self->output_buffer[i]);
      self->output_buffer[i] = NULL;
    }
  }

  /* Must be destroy before input_group */
  if (self->input_group) {
    mpp_buffer_group_put (self->input_group);
    self->input_group = NULL;
  }

  if (self->output_group) {
    mpp_buffer_group_put (self->output_group);
    self->output_group = NULL;
  }

  GST_DEBUG_OBJECT (self, "Stopped");
  return TRUE;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
iepdeinterlace_init (GstPlugin * iepdeinterlace)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template iepdeinterlace' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_iep_deinterlace_debug, "iepdeinterlace",
      0, "Template iepdeinterlace");

  return gst_element_register (iepdeinterlace, "iepdeinterlace", GST_RANK_NONE,
      GST_TYPE_IEPDEINTERLACE);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstiepdeinterlace"
#endif

/* gstreamer looks for this structure to register iepdeinterlaces
 *
 * exchange the string 'Template iepdeinterlace' with your iepdeinterlace description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    iepdeinterlace,
    "Template iepdeinterlace",
    iepdeinterlace_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
