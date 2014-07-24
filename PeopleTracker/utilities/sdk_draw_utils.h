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
 * File: sdk_draw_utils.h
 *
 * This file provides utility functions that are not part of the PittPatt SDK,
 * but can be useful when writing programs that use the SDK.
 *
 * These functions provide an easy way to draw on images.
 */

#ifndef __SDK_DRAW_UTILS_H
#define __SDK_DRAW_UTILS_H

#include "sdk_utils.h"
#include "pittpatt_detection.h"

/* If C++ then we need to __extern "C". Compiler defines __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* Skipping last column because it is unnecessary padding */
#define SDK_DRAW_UTILS_CHAR_WIDTH  7

/* Added blank top row so background color surrounds text by 1 pixels on all sides */
#define SDK_DRAW_UTILS_CHAR_HEIGHT 9

#define SDK_DRAW_UTILS_LOW_CONFIDENCE_COLOR        SDK_DRAW_UTILS_BLUE
#define SDK_DRAW_UTILS_LOW_CONFIDENCE_THICKNESS    2

#define SDK_DRAW_UTILS_MEDIUM_CONFIDENCE_THRESHOLD 0.5f
#define SDK_DRAW_UTILS_MEDIUM_CONFIDENCE_COLOR     SDK_DRAW_UTILS_YELLOW
#define SDK_DRAW_UTILS_MEDIUM_CONFIDENCE_THICKNESS 3
  
#define SDK_DRAW_UTILS_HIGH_CONFIDENCE_THRESHOLD   1.5f
#define SDK_DRAW_UTILS_HIGH_CONFIDENCE_COLOR       SDK_DRAW_UTILS_GREEN
#define SDK_DRAW_UTILS_HIGH_CONFIDENCE_THICKNESS   4
  
#define SDK_DRAW_UTILS_CROSS_SIZE                  6
#define SDK_DRAW_UTILS_CROSS_THICKNESS             1

/*******************************************************************************
 * Enumerated Types
 ******************************************************************************/

typedef enum
{
  SDK_DRAW_UTILS_GREEN,
  SDK_DRAW_UTILS_RED,
  SDK_DRAW_UTILS_BLUE,
  SDK_DRAW_UTILS_YELLOW,
  SDK_DRAW_UTILS_CYAN,
  SDK_DRAW_UTILS_MAGENTA,
  SDK_DRAW_UTILS_ORANGE,
  SDK_DRAW_UTILS_PINK,
  SDK_DRAW_UTILS_BROWN,
  SDK_DRAW_UTILS_TAN,
  SDK_DRAW_UTILS_GRAY,
  SDK_DRAW_UTILS_BLACK,
  SDK_DRAW_UTILS_WHITE,
  SDK_DRAW_UTILS_NUM_COLORS,
  SDK_DRAW_UTILS_TRANSPARENT
} sdk_draw_utils_color_type;

typedef enum
{
  SDK_DRAW_UTILS_SOLID_LINE,
  SDK_DRAW_UTILS_DOTTED_LINE
} sdk_draw_utils_line_type;

/*******************************************************************************
 * Structure Types
 ******************************************************************************/

typedef struct
{
  int x;
  int y;
} sdk_draw_utils_point_type;
  
typedef struct
{
  sdk_draw_utils_color_type color;
  sdk_draw_utils_line_type line_type;
  int thickness;
} sdk_draw_utils_line_attributes_type;

typedef struct
{
  sdk_draw_utils_color_type color;
  sdk_draw_utils_color_type background_color;
  int scale;
} sdk_draw_utils_text_attributes_type;

typedef struct
{
  int num_colors;
  sdk_draw_utils_color_type *colors;
  int *ids;
  int capacity;
} sdk_draw_utils_id_color_list_type;

/*****************************************************************************
 * Functions
 *****************************************************************************/

/* Initializes line attributes structure with reasonable defaults */
void sdk_draw_utils_init_line_attributes(
  sdk_draw_utils_line_attributes_type *attributes);

/* Initializes text attributes structure with reasonable defaults */
void sdk_draw_utils_init_text_attributes(
  sdk_draw_utils_text_attributes_type *attributes);

/* Draws a single pixel on an image in the image's color space */
void sdk_draw_utils_draw_pixel(
  ppr_raw_image_type *raw_image,
  sdk_draw_utils_point_type point,
  sdk_draw_utils_color_type color);

/* Draws a line on an image */
void sdk_draw_utils_draw_line(
  ppr_raw_image_type *raw_image,
  ppr_coordinate_type point1,
  ppr_coordinate_type point2,
  sdk_draw_utils_line_attributes_type attributes);

/* Draws a cross (plus sign) on an image */
void sdk_draw_utils_draw_cross(
  ppr_raw_image_type *raw_image,
  ppr_coordinate_type center,
  int size,
  sdk_draw_utils_line_attributes_type attributes);

/* Draws a polygon on an image */
void sdk_draw_utils_draw_polygon(
  ppr_raw_image_type *raw_image,
  ppr_polygon_type polygon,
  sdk_draw_utils_line_attributes_type attributes);

/* Draws a single character on an image at the specified position */
void sdk_draw_utils_draw_char(
  ppr_raw_image_type *raw_image,
  char c,
  sdk_draw_utils_point_type top_left,
  sdk_draw_utils_text_attributes_type attributes);

/* Draws a string on an image at the specified position */
void sdk_draw_utils_draw_string(
  ppr_raw_image_type *image,
  const char *c,
  sdk_draw_utils_point_type top_left,
  sdk_draw_utils_text_attributes_type attributes);

/* Returns a string for the sdk_draw_utils_color_type */
const char *sdk_draw_utils_get_color_string(
  sdk_draw_utils_color_type color);

/*****************************************************************************
 * Overlay Functions
 *****************************************************************************/

/* Overlays bounding box and landmarks from a single face onto an image and puts a string under the bounding box */
void sdk_draw_utils_overlay_face(
  ppr_raw_image_type *raw_image,
  ppr_face_type face,
  const char *text,
  sdk_draw_utils_line_attributes_type line_attributes);

/* Overlays bounding boxes and landmarks from all faces onto an image */
void sdk_draw_utils_overlay_face_list(
  ppr_raw_image_type *raw_image,
  ppr_face_list_type face_list,
  int check_confidence);

/* Overlays bounding boxes and landmarks from all tracked faces that meet the minimum confidence */
void sdk_draw_utils_overlay_tracked_face_list(
  ppr_raw_image_type *raw_image,
  ppr_face_list_type tracked_face_list,
  int min_confidence);

/* Overlays a frame number in the upper-left corner of an image */
void sdk_draw_utils_overlay_frame_number(
  ppr_raw_image_type *raw_image,
  int frame_num);
    
/* Advance tracked face indices until frame_num is reached or passed */
void sdk_draw_utils_advance_indices_to_frame(
  ppr_track_list_type track_list,
  int *i_tracked_face_lists,
  int frame_num);

/* Overlays bounding boxes and landmarks from tracks onto a copy of the images in the list  */
void sdk_draw_utils_overlay_track_list_on_image_list(
  sdk_utils_file_list_type in_image_list,
  ppr_track_list_type track_list,
  int min_confidence,
  int overlay_frame_numbers,
  const char *output_image_format);

/* Overlays bounding boxes and landmarks from tracks onto copies of the frames in the video */
void sdk_draw_utils_overlay_track_list_on_video(
  const char *in_filename,
  ppr_track_list_type track_list,
  int min_confidence,
  int overlay_frame_numbers,
  const char *output_image_format);

/* Returns the position to write text below a polygon */
void sdk_draw_utils_get_string_position_below_polygon(
  ppr_polygon_type polygon,
  const char *str,
  int scale,
  sdk_draw_utils_point_type *position);

/*****************************************************************************
 * Color Functions
 *****************************************************************************/

/* Allocates the internal buffers of an id color list and sets the number of colors to 0 */
void sdk_draw_utils_id_color_list_allocate(
  sdk_draw_utils_id_color_list_type *id_color_list,
  int max_num_colors);

/* Adds an entry to an id color list, exiting with an error if the capacity is exceeded */
void sdk_draw_utils_id_color_list_append(
  sdk_draw_utils_id_color_list_type *id_color_list,
  int id,
  sdk_draw_utils_color_type color);

/* Returns the color associated with an id in the id color list */
sdk_draw_utils_color_type sdk_draw_utils_id_color_list_get_color(
  sdk_draw_utils_id_color_list_type id_color_list,
  int id);

/* Frees the internal buffers of an id color list */
void sdk_draw_utils_id_color_list_free(
  sdk_draw_utils_id_color_list_type id_color_list);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __SDK_DRAW_UTILS_H */
