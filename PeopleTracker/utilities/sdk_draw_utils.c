/**********************************************************************
 
  PittPatt Face Recognition Software Development Kit (PittPatt SDK)
  (C) Copyright 2004-2011 Pittsburgh Pattern Recognition

  This software is covered in part by the following patents:

     US Patent 6,829,384
     US Patent 7,194,114
     US Patent 7,848,566
     US Patent 7,881,505
     Pending US Patent Applications

  Portions of this product are manufactured under license from Carnegie
  Mellon University.

**********************************************************************/


/*
 * File: sdk_draw_utils.c
 *
 * This file provides utility functions that are not part of the PittPatt SDK,
 * but can be useful when writing programs that use the SDK.
 *
 * These functions provide an easy way to draw on images.
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "sdk_draw_utils.h"
#include "sdk_draw_utils_charset.h"
#include "sdk_utils.h"

#include "pittpatt_raw_image_io.h"
#include "pittpatt_video_io.h"

/******************************************************************************
 * Constants
 ******************************************************************************/

#define SDK_DRAW_UTILS_DOTTED_LINE_ON_LENGTH   10
#define SDK_DRAW_UTILS_DOTTED_LINE_OFF_LENGTH  3

#define BIT_RATE                               1000
  
static const char *sdk_draw_utils_static_color_strings[] =
{
  "green",
  "red",
  "blue",
  "yellow",
  "cyan",
  "magenta",
  "orange",
  "pink",
  "brown",
  "tan",
  "gray",
  "black",
  "white"
};

/*******************************************************************************
 * Macros
 ******************************************************************************/

/* Clips a value at 0 if it is less than 0 and at 255 if it is greater than 255 */
#define CLIP8(AA) (unsigned char)(((int)(AA)<0)?0:((int)(AA)>0xFF)?0xFF:(int)(AA))

/******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static int sdk_draw_utils_bit_at(
  unsigned char c,
  int x,
  int y);

static void sdk_draw_utils_color_to_rgb(
  sdk_draw_utils_color_type color,
  unsigned char *r,
  unsigned char *g,
  unsigned char *b);

static void sdk_draw_utils_rgb24_to_gray8(
  unsigned char r,
  unsigned char g,
  unsigned char b,
  unsigned char *gray);

static void sdk_draw_utils_rgb24_to_argb32(
  unsigned char r,
  unsigned char g,
  unsigned char b,
  unsigned int *argb);

static void sdk_draw_utils_rgb24_to_yuv24(
  unsigned char r,
  unsigned char g,
  unsigned char b,
  unsigned char *y,
  unsigned char *u,
  unsigned char *v);

static void sdk_draw_utils_overlay_faces(
  ppr_raw_image_type *raw_image,
  int num_faces,
  ppr_face_type *faces,
  int check_confidence);

static void sdk_draw_utils_overlay_tracked_faces(
  ppr_raw_image_type *raw_image,
  int num_tracked_faces,
  ppr_face_type *tracked_faces,
  int min_confidence);

/******************************************************************************
 * Public Functions
 ******************************************************************************/

/* Initializes line attributes structure with reasonable defaults */
void sdk_draw_utils_init_line_attributes(
  sdk_draw_utils_line_attributes_type *attributes)
{
  attributes->color = SDK_DRAW_UTILS_GREEN;
  attributes->line_type = SDK_DRAW_UTILS_SOLID_LINE;
  attributes->thickness = 1;
}

/******************************************************************************/

/* Initializes text attributes structure with reasonable defaults */
void sdk_draw_utils_init_text_attributes(
  sdk_draw_utils_text_attributes_type *attributes)
{
  attributes->color = SDK_DRAW_UTILS_GREEN;
  attributes->background_color = SDK_DRAW_UTILS_TRANSPARENT;
  attributes->scale = 1;
}

/******************************************************************************/

/* Draws a single pixel on an image in the image's color space */
void sdk_draw_utils_draw_pixel(
  ppr_raw_image_type *raw_image,
  sdk_draw_utils_point_type point,
  sdk_draw_utils_color_type color)
{
  int x;
  int y;
  
  unsigned char r;
  unsigned char g;
  unsigned char b;

  unsigned char gray;
  
  unsigned char y_chan;
  unsigned char u_chan;
  unsigned char v_chan;

  unsigned int argb;
  unsigned int *p_argb;

  /* Don't draw pixel if color is set to transparent */
  if (color == SDK_DRAW_UTILS_TRANSPARENT)
    return;

  x = point.x;
  y = point.y;
  
  /* Don't try to draw pixel if it is out of bounds */
  if (x < 0 || x >= raw_image->width || y < 0 || y >= raw_image->height)
    return;
  
  sdk_draw_utils_color_to_rgb(color, &r, &g, &b);
  
  switch (raw_image->color_space)
  {
    case PPR_RAW_IMAGE_GRAY8:
      sdk_draw_utils_rgb24_to_gray8(r, g, b, &gray);
      raw_image->data[y * raw_image->bytes_per_line + x] = gray;
      break;
      
    case PPR_RAW_IMAGE_RGB24:
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 0] = r;
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 1] = g;
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 2] = b;
      break;
      
    case PPR_RAW_IMAGE_BGR24:
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 0] = b;
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 1] = g;
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 2] = r;
      break;
      
    case PPR_RAW_IMAGE_ARGB32:
      sdk_draw_utils_rgb24_to_argb32(r, g, b, &argb);
      p_argb = (unsigned int*)&raw_image->data[y * raw_image->bytes_per_line + x * 4];
      *p_argb = argb;
      break;
      
    case PPR_RAW_IMAGE_YUV:
      sdk_draw_utils_rgb24_to_yuv24(r, g, b, &y_chan, &u_chan, &v_chan);
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 0] = y_chan;
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 1] = u_chan;
      raw_image->data[y * raw_image->bytes_per_line + x * 3 + 2] = v_chan;
      break;

    default:
      fprintf(stderr, "ERROR: illegal color space in sdk_draw_utils_draw_pixel()\n");
      exit(EXIT_FAILURE);
  }
}

/******************************************************************************/

/* Draws a line on an image */
void sdk_draw_utils_draw_line(
  ppr_raw_image_type *raw_image,
  ppr_coordinate_type point1,
  ppr_coordinate_type point2,
  sdk_draw_utils_line_attributes_type attributes)
{
  int upper_offset;
  int lower_offset;
  ppr_coordinate_type diff;
  float slope;
  float intercept;
  float inverse_slope;
  int start;
  int stop;
  int x;
  int y;
  int i;
  int j;
  int width;
  int height;
  int draw_count;
  sdk_draw_utils_point_type pixel;

  if (attributes.thickness <= 0)
  {
    fprintf(stderr, "ERROR: draw_line() requires thickness > 0\n");
    return;
  }

  width = raw_image->width;
  height = raw_image->height;

  /* Do not draw line if both points are outside of the image */
  if ((point1.x < 0 || point1.x >= width || point1.y < 0 || point1.y >= height) &&
      (point2.x < 0 || point2.x >= width || point2.y < 0 || point2.y >= height))
  {
    return;
  }

  if ((attributes.thickness % 2) == 0)
  {
    lower_offset = attributes.thickness / 2;
    upper_offset = (attributes.thickness - 1) / 2;
  }
  else
  {
    lower_offset = attributes.thickness / 2;
    upper_offset = attributes.thickness / 2;
  }
  
  diff.x = point2.x - point1.x;
  diff.y = point2.y - point1.y;
  
  if (fabs(diff.y) < fabs(diff.x))
  {
    slope = diff.y / diff.x;
    intercept = point1.y - slope * point1.x;

    if (point1.x > point2.x)
    {
      start = (int)point2.x;
      stop  = (int)point1.x;
    }
    else
    {
      start = (int)point1.x;
      stop  = (int)point2.x;
    }

    if (start < 0)
      start = 0;
    if (stop >= width)
      stop = width - 1;

    x = start;
    draw_count = 0;
    while (x <= stop)
    {
      y = (int)(slope * (float)x + intercept);

      for (i = y - lower_offset; i <= y + upper_offset; i++)
      {
        pixel.x = x;
        pixel.y = i;
        sdk_draw_utils_draw_pixel(raw_image, pixel, attributes.color);
      }

      x++;

      if (attributes.line_type == SDK_DRAW_UTILS_DOTTED_LINE)
      {
        draw_count++;
        if (draw_count == SDK_DRAW_UTILS_DOTTED_LINE_ON_LENGTH)
        {
          draw_count = 0;
          x += SDK_DRAW_UTILS_DOTTED_LINE_OFF_LENGTH;
        }
      }
    }
  }
  else
  {
    inverse_slope = diff.x / diff.y;
    intercept = point1.x - point1.y * inverse_slope;

    if (point1.y > point2.y)
    {
      start = (int)point2.y;
      stop  = (int)point1.y;
    }
    else
    {
      start = (int)point1.y;
      stop  = (int)point2.y;
    }

    if (start < 0)
      start = 0;
    if (stop >= height)
      stop = height - 1;

    y = start;
    draw_count = 0;
    while (y <= stop)
    {
      x = (int)((float)y * inverse_slope + intercept);

      for (j = x - lower_offset; j <= x + upper_offset; j++)
      {
        pixel.x = j;
        pixel.y = y;
        sdk_draw_utils_draw_pixel(raw_image, pixel, attributes.color);
      }

      y++;

      if (attributes.line_type == SDK_DRAW_UTILS_DOTTED_LINE)
      {
        draw_count++;
        if (draw_count == SDK_DRAW_UTILS_DOTTED_LINE_ON_LENGTH)
        {
          draw_count = 0;
          y += SDK_DRAW_UTILS_DOTTED_LINE_OFF_LENGTH;
        }
      }
    }
  }
}

/*****************************************************************************/

/* Draw a cross (plus sign) on an image */
void sdk_draw_utils_draw_cross(
  ppr_raw_image_type *raw_image,
  ppr_coordinate_type center,
  int size,
  sdk_draw_utils_line_attributes_type attributes)
{
  ppr_coordinate_type line1_pnt1;
  ppr_coordinate_type line1_pnt2;
  ppr_coordinate_type line2_pnt1;
  ppr_coordinate_type line2_pnt2;

  line1_pnt1.x = center.x - size / 2;
  line1_pnt1.y = center.y;

  line1_pnt2.x = center.x + size / 2;
  line1_pnt2.y = center.y;

  sdk_draw_utils_draw_line(raw_image, line1_pnt1, line1_pnt2, attributes);

  line2_pnt1.x = center.x;
  line2_pnt1.y = center.y - size / 2;

  line2_pnt2.x = center.x;
  line2_pnt2.y = center.y + size / 2;

  sdk_draw_utils_draw_line(raw_image, line2_pnt1, line2_pnt2, attributes);
}

/******************************************************************************/

/* Draws a polygon on an image */
void sdk_draw_utils_draw_polygon(
  ppr_raw_image_type *raw_image,
  ppr_polygon_type polygon,
  sdk_draw_utils_line_attributes_type attributes)
{
  int i;
  
  for (i = 0; i < polygon.num_points; i++)
    sdk_draw_utils_draw_line(raw_image, polygon.points[i], polygon.points[(i + 1) % polygon.num_points], attributes);
}

/******************************************************************************/

/* Draws a single character on an image at the specified position */
void sdk_draw_utils_draw_char(
  ppr_raw_image_type *raw_image,
  char c,
  sdk_draw_utils_point_type top_left,
  sdk_draw_utils_text_attributes_type attributes)
{
  int y;
  int x;
  int i;
  int j;
  int img_x;
  int img_y;
  sdk_draw_utils_point_type pixel;

  if (top_left.x >= raw_image->width || top_left.y >= raw_image->height)
    return;

  img_y = top_left.y;
  for (y = 0; y < SDK_DRAW_UTILS_CHAR_HEIGHT; y++)
  {
    for (i = 0; i < attributes.scale && img_y < raw_image->height; i++)
    {
      img_x = top_left.x;

      for (x = 0; x < SDK_DRAW_UTILS_CHAR_WIDTH; x++)
      {
        for (j = 0; j < attributes.scale && img_x < raw_image->width; j++)
        {
          pixel.x = img_x;
          pixel.y = img_y;
          
          if (sdk_draw_utils_bit_at(c, x, y))
            sdk_draw_utils_draw_pixel(raw_image, pixel, attributes.color);
          else
            sdk_draw_utils_draw_pixel(raw_image, pixel, attributes.background_color);
          
          img_x++;
        }
      }
      img_y++;
      
      if (img_y >= raw_image->height)
        return;
    }
  }
}

/******************************************************************************/

/* Draws a string on an image at the specified position */
void sdk_draw_utils_draw_string(
  ppr_raw_image_type *raw_image,
  const char *c,
  sdk_draw_utils_point_type top_left,
  sdk_draw_utils_text_attributes_type attributes)
{
  int letter;
  int original_x = top_left.x;
  
  letter = *c;
  while (letter != '\0')
  {
    if (letter == '\n')
    {
      top_left.x = original_x;
      top_left.y += SDK_DRAW_UTILS_CHAR_HEIGHT * attributes.scale;
    }
    else
    {
      sdk_draw_utils_draw_char(raw_image, (char)letter, top_left, attributes);
      top_left.x += SDK_DRAW_UTILS_CHAR_WIDTH * attributes.scale;
    }
    
    c++;
    letter = *c;
  }
}

/******************************************************************************/

/* Returns a string for the sdk_draw_utils_color_type */
const char *sdk_draw_utils_get_color_string(
  sdk_draw_utils_color_type color)
{
  if (color >= SDK_DRAW_UTILS_NUM_COLORS || (int)color < 0)
    return "ILLEGAL_COLOR";

  return sdk_draw_utils_static_color_strings[color];
}

/*****************************************************************************
 * Overlay Functions
 *****************************************************************************/

/* Overlays bounding box and landmarks from a single face onto an image and puts a string under the bounding box */
void sdk_draw_utils_overlay_face(
  ppr_raw_image_type *raw_image,
  ppr_face_type face,
  const char *text,
  sdk_draw_utils_line_attributes_type line_attributes)
{
  int landmark_num;

  ppr_polygon_type polygon;
  ppr_landmark_list_type landmark_list;
  sdk_draw_utils_text_attributes_type text_attributes;
  sdk_draw_utils_point_type string_top_left;

  sdk_draw_utils_init_text_attributes(&text_attributes);
  text_attributes.color = SDK_DRAW_UTILS_WHITE;
  text_attributes.background_color = SDK_DRAW_UTILS_BLACK;
  
  /* If color space is GRAY8, covert to RGB24 to get color overlays */
  if (raw_image->color_space == PPR_RAW_IMAGE_GRAY8)
    ppr_raw_image_convert(raw_image, PPR_RAW_IMAGE_RGB24);

  ppr_get_face_boundary(face, PPR_BOUNDING_BOX_STYLE_DEFAULT, &polygon);

  sdk_draw_utils_get_string_position_below_polygon(polygon, text, text_attributes.scale, &string_top_left);
  sdk_draw_utils_draw_polygon(raw_image, polygon, line_attributes);
  sdk_draw_utils_draw_string(raw_image, text, string_top_left, text_attributes);
  ppr_free_polygon(polygon);

  line_attributes.line_type = SDK_DRAW_UTILS_SOLID_LINE;
  line_attributes.thickness = SDK_DRAW_UTILS_CROSS_THICKNESS;

  ppr_get_face_landmarks(face, &landmark_list);
  for (landmark_num = 0; landmark_num < landmark_list.length; landmark_num++)
    sdk_draw_utils_draw_cross(raw_image, landmark_list.landmarks[landmark_num].position, SDK_DRAW_UTILS_CROSS_SIZE, line_attributes);
  ppr_free_landmark_list(landmark_list);
}

/*****************************************************************************/

/* Overlays bounding boxes and landmarks from all faces onto an image */
void sdk_draw_utils_overlay_face_list(
  ppr_raw_image_type *raw_image,
  ppr_face_list_type face_list,
  int check_confidence)
{
  sdk_draw_utils_overlay_faces(raw_image, face_list.length, face_list.faces, check_confidence);
}

/*****************************************************************************/

/* Overlays bounding boxes and landmarks from all tracked faces that meet the minimum confidence */
void sdk_draw_utils_overlay_tracked_face_list(
  ppr_raw_image_type *raw_image,
  ppr_face_list_type tracked_face_list,
  int min_confidence)
{
  sdk_draw_utils_overlay_tracked_faces(raw_image, tracked_face_list.length, tracked_face_list.faces, min_confidence);
}

/******************************************************************************/

/* Overlays a frame number in the upper-left corner of an image */
void sdk_draw_utils_overlay_frame_number(
  ppr_raw_image_type *raw_image,
  int frame_num)
{
  char frame_number_string[32];
  sdk_draw_utils_point_type string_top_left;
  sdk_draw_utils_text_attributes_type text_attributes;

  sdk_draw_utils_init_text_attributes(&text_attributes);
  text_attributes.color = SDK_DRAW_UTILS_WHITE;
  text_attributes.background_color = SDK_DRAW_UTILS_BLACK;
  text_attributes.scale = 2;
  
  string_top_left.x = 0;
  string_top_left.y = 0;
  
  sprintf(frame_number_string, "%d", frame_num);
  sdk_draw_utils_draw_string(raw_image, frame_number_string, string_top_left, text_attributes);
}

/******************************************************************************/

/* Advance tracked face indices until frame_num is reached or passed */
void sdk_draw_utils_advance_indices_to_frame(
  ppr_track_list_type track_list,
  int *i_tracked_face_lists,
  int frame_num)
{
  int i;
  ppr_face_attributes_type face_attributes;

  for (i = 0; i < track_list.length; i++)
  {
    while (i_tracked_face_lists[i] < track_list.tracks[i].tracked_faces.length - 1)
    {
      ppr_get_face_attributes(track_list.tracks[i].tracked_faces.faces[i_tracked_face_lists[i]], &face_attributes);
      if(!(face_attributes.tracking_info.frame_number < frame_num))
        break;
      i_tracked_face_lists[i]++;
    }
  }
}

/*****************************************************************************/

/* Overlays bounding boxes and landmarks from tracks onto a copy of the images in the list  */
void sdk_draw_utils_overlay_track_list_on_image_list(
  sdk_utils_file_list_type in_image_list,
  ppr_track_list_type track_list,
  int min_confidence,
  int overlay_frame_numbers,
  const char *output_image_format)
{
  int i;
  int frame_num;

  int *i_tracked_face_lists;

  ppr_raw_image_type raw_image;
  char title[FILENAME_MAX];
  char out_filename[FILENAME_MAX];
  ppr_face_attributes_type face_attributes;
  
  /* Create array of indices into tracked face arrays */
  i_tracked_face_lists = (int*)malloc(track_list.length * sizeof(int));
  sdk_utils_check_allocation(i_tracked_face_lists, "i_tracked_face_lists", __FUNCTION__);
  for (i = 0; i < track_list.length; i++)
    i_tracked_face_lists[i] = 0;

  /* Loop through each image in list */
  for (frame_num = 0; frame_num < in_image_list.length; frame_num++)
  {
    /* Read image */
    TRY_RAW_IMAGE(ppr_raw_image_io_read(in_image_list.filenames[frame_num], &raw_image));

    /* Overlay polygons on frame */
    sdk_draw_utils_advance_indices_to_frame(track_list, i_tracked_face_lists, frame_num);
    for (i = 0; i < track_list.length; i++)
    {
      ppr_get_face_attributes(track_list.tracks[i].tracked_faces.faces[i_tracked_face_lists[i]], &face_attributes);
      if (face_attributes.tracking_info.frame_number == frame_num)
      {
        /* Overlay polygon for this face */
        sdk_draw_utils_overlay_tracked_faces(&raw_image, 1, &track_list.tracks[i].tracked_faces.faces[i_tracked_face_lists[i]], min_confidence);
      }
    }

    /* Overlay frame number on frame */
    if (overlay_frame_numbers)
      sdk_draw_utils_overlay_frame_number(&raw_image, frame_num);

    /* Write image to file */
    sdk_utils_get_file_title(in_image_list.filenames[frame_num], title);
    sprintf(out_filename, "%s_track_overlay.%s", title, output_image_format);
    TRY_RAW_IMAGE(ppr_raw_image_io_write(out_filename, raw_image));

    /* Free image */
    ppr_raw_image_free(raw_image);
  }

  /* Clean up */
  free(i_tracked_face_lists);
}

/*****************************************************************************/

/* Overlays bounding boxes and landmarks from tracks onto copies of the frames in the video */
void sdk_draw_utils_overlay_track_list_on_video(
  const char *in_filename,
  ppr_track_list_type track_list,
  int min_confidence,
  int overlay_frame_numbers,
  const char *output_video_format)
{
  int i;
  int frame_num;

  ppr_video_io_type input_video;
  ppr_video_io_type output_video;
  int frame_rate_numerator;
  int frame_rate_denominator;
  int *i_tracked_face_lists;
  ppr_face_attributes_type face_attributes;

  ppr_raw_image_type raw_image;
  char title[FILENAME_MAX];
  char out_filename[FILENAME_MAX];
  
  /* Open video */
  TRY_VIDEO(ppr_video_io_open(&input_video, in_filename));
  TRY_VIDEO(ppr_video_io_get_frame_rate(input_video, &frame_rate_numerator, &frame_rate_denominator));

  /* Create output video */
  sdk_utils_get_file_title(in_filename, title);
  sprintf(out_filename, "%s_track_overlay.%s", title, output_video_format);
  TRY_VIDEO(ppr_video_io_create(&output_video, out_filename, BIT_RATE, frame_rate_numerator, frame_rate_denominator));

  /* Create array of indices into tracked face arrays */
  i_tracked_face_lists = (int*)malloc(track_list.length * sizeof(int));
  sdk_utils_check_allocation(i_tracked_face_lists, "i_tracked_face_lists", __FUNCTION__);
  for (i = 0; i < track_list.length; i++)
    i_tracked_face_lists[i] = 0;

  /* Loop through each frame of video */
  frame_num = 0;
  while (ppr_video_io_is_frame_available(input_video))
  {
    /* Read frame from video */
    TRY_VIDEO(ppr_video_io_get_frame(input_video, &raw_image));

    /* Overlay polygons on frame */
    sdk_draw_utils_advance_indices_to_frame(track_list, i_tracked_face_lists, frame_num);
    for (i = 0; i < track_list.length; i++)
    {
      ppr_get_face_attributes(track_list.tracks[i].tracked_faces.faces[i_tracked_face_lists[i]], &face_attributes);
      if (face_attributes.tracking_info.frame_number == frame_num)
      {
        /* Overlay polygon for this face */
        sdk_draw_utils_overlay_tracked_faces(&raw_image, 1, &track_list.tracks[i].tracked_faces.faces[i_tracked_face_lists[i]], min_confidence);
      }
    }

    /* Overlay frame number on frame */
    if (overlay_frame_numbers)
      sdk_draw_utils_overlay_frame_number(&raw_image, frame_num);

    /* Write frame to output video */
    TRY_VIDEO(ppr_video_io_add_frame(output_video, raw_image));

    /* Free frame */
    ppr_raw_image_free(raw_image);

    /* Advance to next frame */
    TRY_VIDEO(ppr_video_io_step_forward(input_video));
    frame_num++;
  }

  /* Close video */
  TRY_VIDEO(ppr_video_io_close(input_video));
  TRY_VIDEO(ppr_video_io_close(output_video));

  /* Clean up */
  free(i_tracked_face_lists);
}

/******************************************************************************/

void sdk_draw_utils_get_string_position_below_polygon(
  ppr_polygon_type polygon,
  const char *str,
  int scale,
  sdk_draw_utils_point_type *position)
{
  int i;
  int cnt;
  ppr_coordinate_type top_left;
  ppr_coordinate_type bottom_right;
  int num_chars;
  int num_pixels;

  top_left.x = FLT_MAX;
  top_left.y = FLT_MAX;
  bottom_right.x = -FLT_MAX;
  bottom_right.y = -FLT_MAX;
  
  /* Get extents of polygon */
  for (i = 0; i < polygon.num_points; i++)
  {
    if (polygon.points[i].x < top_left.x)
      top_left.x = polygon.points[i].x;

    if (polygon.points[i].x > bottom_right.x)
      bottom_right.x = polygon.points[i].x;

    if (polygon.points[i].y < top_left.y)
      top_left.y = polygon.points[i].y;

    if (polygon.points[i].y > bottom_right.y)
      bottom_right.y = polygon.points[i].y;
  }

  /* Get the maximum number of characters on a single line */
  i = 0;
  cnt = 0;
  num_chars = 0;
  while (str[i] != '\0')
  {
    if (str[i] == '\n')
    {
      if (cnt > num_chars)
        num_chars = cnt;
      cnt = 0;
    }
    else
    {
      cnt++;
    }
    
    i++;
  }
  if (cnt > num_chars)
    num_chars = cnt;
  
  num_pixels = num_chars * SDK_DRAW_UTILS_CHAR_WIDTH * scale;

  position->x = (int)((bottom_right.x + top_left.x) / 2.0f);
  position->x -= num_pixels / 2;

  position->y = (int)bottom_right.y + 5;
}

/*****************************************************************************
 * Color Functions
 *****************************************************************************/

/* Allocates the internal buffers of an id color list and sets the number of colors to 0 */
void sdk_draw_utils_id_color_list_allocate(
  sdk_draw_utils_id_color_list_type *id_color_list,
  int max_num_colors)
{
  id_color_list->num_colors = 0;

  id_color_list->colors = (sdk_draw_utils_color_type*)malloc(max_num_colors * sizeof(sdk_draw_utils_color_type));
  sdk_utils_check_allocation(id_color_list->colors, "id_color_list->colors", __FUNCTION__);

  id_color_list->ids = (int*)malloc(max_num_colors * sizeof(int));
  sdk_utils_check_allocation(id_color_list->ids, "id_color_list->ids", __FUNCTION__);

  id_color_list->capacity = max_num_colors;
}

/******************************************************************************/

/* Adds an entry to an id color list, exiting with an error if the capacity is exceeded */
void sdk_draw_utils_id_color_list_append(
  sdk_draw_utils_id_color_list_type *id_color_list,
  int id,
  sdk_draw_utils_color_type color)
{
  if (id_color_list->num_colors == id_color_list->capacity)
  {
    fprintf(stderr, "ERROR: attempted to exceed capacity of id color list\n");
    exit(EXIT_FAILURE);
  }

  id_color_list->colors[id_color_list->num_colors] = color;
  id_color_list->ids[id_color_list->num_colors] = id;

  id_color_list->num_colors++;
}

/******************************************************************************/

/* Returns the color associated with an id in the id color list */
sdk_draw_utils_color_type sdk_draw_utils_id_color_list_get_color(
  sdk_draw_utils_id_color_list_type id_color_list,
  int id)
{
  int i;

  for (i = 0; i < id_color_list.num_colors; i++)
  {
    if (id_color_list.ids[i] == id)
      return id_color_list.colors[i];
  }

  return (sdk_draw_utils_color_type)-1;
}

/******************************************************************************/

/* Frees the internal buffers of an id color list */
void sdk_draw_utils_id_color_list_free(
  sdk_draw_utils_id_color_list_type id_color_list)
{
  free(id_color_list.colors);
  free(id_color_list.ids);
}

/******************************************************************************
 * Local Functions
 ******************************************************************************/

static int sdk_draw_utils_bit_at(
  unsigned char c,
  int x,
  int y)
{
  unsigned char row;
  
  if (c < 0x20)
    c = 0x20;
  
  c -= 0x20;

  /*
   * Characters are 5 x 7 right-justified in an 8x9 bit field.
   * Shifting to the left to center them.  Since the width is odd, it can't be
   * perfectly centered.  Choosing to err to the left side and draw_char ignores
   * the last column.
   */
  row = charset[c][y] << 2;
  
  return (row >> (7-x)) & 1;
}

/******************************************************************************/

static void sdk_draw_utils_color_to_rgb(
  sdk_draw_utils_color_type color,
  unsigned char *r,
  unsigned char *g,
  unsigned char *b)
{
  switch (color)
  {
  case SDK_DRAW_UTILS_BLACK:
    *r = 0x00; *g = 0x00; *b = 0x00;
    break;
  case SDK_DRAW_UTILS_WHITE:
    *r = 0xff; *g = 0xff; *b = 0xff;
    break;
  case SDK_DRAW_UTILS_GRAY:
    *r = 0x80; *g = 0x80; *b = 0x80;
    break;
  case SDK_DRAW_UTILS_TAN:
    *r = 0xd2; *g = 0xb4; *b = 0x8c;
    break;
  case SDK_DRAW_UTILS_BROWN:
    *r = 0xa5; *g = 0x2a; *b = 0x2a;
    break;
  case SDK_DRAW_UTILS_PINK:
    *r = 0xff; *g = 0xc0; *b = 0xcb;
    break;
  case SDK_DRAW_UTILS_RED:
    *r = 0xff; *g = 0x00; *b = 0x00;
    break;
  case SDK_DRAW_UTILS_ORANGE:
    *r = 0xff; *g = 0x80; *b = 0x00;
    break;
  case SDK_DRAW_UTILS_YELLOW:
    *r = 0xff; *g = 0xff; *b = 0x00;
    break;
  case SDK_DRAW_UTILS_GREEN:
    *r = 0x00; *g = 0xff; *b = 0x00;
    break;
  case SDK_DRAW_UTILS_CYAN:
    *r = 0x00; *g = 0xff; *b = 0xff;
    break;
  case SDK_DRAW_UTILS_BLUE:
    *r = 0x00; *g = 0x00; *b = 0xff;
    break;
  case SDK_DRAW_UTILS_MAGENTA:
    *r = 0xff; *g = 0x00; *b = 0xff;
    break;
  default:
    fprintf(stderr, "ERROR: illegal color in sdk_draw_utils_color_to_rgb()\n");
    fprintf(stderr, "  Defaulting to BLACK\n");
    *r = 0x00; *g = 0x00; *b = 0x00;
    break;
  }
}

/******************************************************************************/

static void sdk_draw_utils_rgb24_to_gray8(
  unsigned char r,
  unsigned char g,
  unsigned char b,
  unsigned char *gray)
{
  float grayf;
  
  grayf = (0.299f * (float)r + 0.587f * (float)g + 0.114f * (float)b);
  *gray = CLIP8(grayf);
}

/******************************************************************************/

static void sdk_draw_utils_rgb24_to_argb32(
  unsigned char r,
  unsigned char g,
  unsigned char b,
  unsigned int *argb)
{
  *argb = (0xFF << 24) | (r << 16) | (g << 8) | (b);
}

/******************************************************************************/

static void sdk_draw_utils_rgb24_to_yuv24(
  unsigned char r,
  unsigned char g,
  unsigned char b,
  unsigned char *y,
  unsigned char *u,
  unsigned char *v)
{
  float yf;
  float uf;
  float vf;

  yf = ( 0.2990f * (float)r) + ( 0.5870f * (float)g) + ( 0.1140f * (float)b);
  uf = (-0.1687f * (float)r) + (-0.3313f * (float)g) + ( 0.5000f * (float)b) + 128.0f;
  vf = ( 0.5000f * (float)r) + (-0.4187f * (float)g) + (-0.0813f * (float)b) + 128.0f;

  *y = CLIP8(yf);
  *u = CLIP8(uf);
  *v = CLIP8(vf);
}

/******************************************************************************/

/* Overlays bounding boxes and landmarks from all faces onto an image */
static void sdk_draw_utils_overlay_faces(
  ppr_raw_image_type *raw_image,
  int num_faces,
  ppr_face_type *faces,
  int check_confidence)
{
  int face_num;

  ppr_face_attributes_type face_attributes;
  sdk_draw_utils_line_attributes_type line_attributes;

  sdk_draw_utils_init_line_attributes(&line_attributes);
  line_attributes.line_type = SDK_DRAW_UTILS_SOLID_LINE;

  for (face_num = 0; face_num < num_faces; face_num++)
  {
    ppr_get_face_attributes(faces[face_num], &face_attributes);

    line_attributes.color = SDK_DRAW_UTILS_HIGH_CONFIDENCE_COLOR;
    line_attributes.thickness = SDK_DRAW_UTILS_HIGH_CONFIDENCE_THICKNESS;
    if (check_confidence)
    {
      if (face_attributes.confidence < SDK_DRAW_UTILS_MEDIUM_CONFIDENCE_THRESHOLD)
      {
        line_attributes.color = SDK_DRAW_UTILS_LOW_CONFIDENCE_COLOR;
        line_attributes.thickness = SDK_DRAW_UTILS_LOW_CONFIDENCE_THICKNESS;
      }
      else if (face_attributes.confidence < SDK_DRAW_UTILS_HIGH_CONFIDENCE_THRESHOLD)
      {
        line_attributes.color = SDK_DRAW_UTILS_MEDIUM_CONFIDENCE_COLOR;
        line_attributes.thickness = SDK_DRAW_UTILS_MEDIUM_CONFIDENCE_THICKNESS;
      }
    }
    
    sdk_draw_utils_overlay_face(raw_image, faces[face_num], "", line_attributes);
  }
}

/*****************************************************************************/

/* Overlays bounding boxes and landmarks from all tracked faces that meet the minimum confidence */
static void sdk_draw_utils_overlay_tracked_faces(
  ppr_raw_image_type *raw_image,
  int num_tracked_faces,
  ppr_face_type *tracked_faces,
  int min_confidence)
{
  int tracked_face_num;
  char id_string[32];
  ppr_face_attributes_type face_attributes;
  sdk_draw_utils_line_attributes_type line_attributes;

  sdk_draw_utils_init_line_attributes(&line_attributes);
  line_attributes.color = SDK_DRAW_UTILS_HIGH_CONFIDENCE_COLOR;
  line_attributes.line_type = SDK_DRAW_UTILS_SOLID_LINE;
  line_attributes.thickness = SDK_DRAW_UTILS_HIGH_CONFIDENCE_THICKNESS;
  
  for (tracked_face_num = 0; tracked_face_num < num_tracked_faces; tracked_face_num++)
  {
    ppr_get_face_attributes(tracked_faces[tracked_face_num], &face_attributes);
    if (face_attributes.tracking_info.confidence_level >= min_confidence)
    {
      sprintf(id_string, "id=%d", face_attributes.tracking_info.track_id);
      sdk_draw_utils_overlay_face(raw_image, tracked_faces[tracked_face_num], id_string, line_attributes);
    }
  }
}

/******************************************************************************/
