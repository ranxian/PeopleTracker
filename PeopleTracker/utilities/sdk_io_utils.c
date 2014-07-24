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


#include <stdio.h>
#include <stdlib.h>

#include "sdk_io_utils.h"
#include "pittpatt_sdk.h"
#include "sdk_utilities.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/

void sdk_io_utils_read_face(
  ppr_context_type context,
  const char *filename,
  ppr_face_type *face)
{
  ppr_flat_data_type flat_data;
  
  TRY(ppr_read_flat_data(context, filename, &flat_data));
  TRY(ppr_unflatten_face(context, flat_data, face));
  ppr_free_flat_data(flat_data);
}

void sdk_io_utils_write_face(
  ppr_context_type context,
  const char *filename,
  ppr_face_type face)
{
  ppr_flat_data_type flat_data;
  
  TRY(ppr_flatten_face(context, face, &flat_data));
  TRY(ppr_write_flat_data(context, filename, flat_data));
  ppr_free_flat_data(flat_data);
}

/******************************************************************************/

void sdk_io_utils_read_face_list(
  ppr_context_type context,
  const char *filename,
  ppr_face_list_type *face_list)
{
  ppr_flat_data_type flat_data;
  
  TRY(ppr_read_flat_data(context, filename, &flat_data));
  TRY(ppr_unflatten_face_list(context, flat_data, face_list));
  ppr_free_flat_data(flat_data);
}

void sdk_io_utils_write_face_list(
  ppr_context_type context,
  const char *filename,
  ppr_face_list_type face_list)
{
  ppr_flat_data_type flat_data;
  
  TRY(ppr_flatten_face_list(context, face_list, &flat_data));
  TRY(ppr_write_flat_data(context, filename, flat_data));
  ppr_free_flat_data(flat_data);
}

/******************************************************************************/

void sdk_io_utils_read_track_list(
  ppr_context_type context,
  const char *filename,
  ppr_track_list_type *track_list)
{
  ppr_flat_data_type flat_data;
  
  TRY(ppr_read_flat_data(context, filename, &flat_data));
  TRY(ppr_unflatten_track_list(context, flat_data, track_list));
  ppr_free_flat_data(flat_data);
}

void sdk_io_utils_write_track_list(
  ppr_context_type context,
  const char *filename,
  ppr_track_list_type track_list)
{
  ppr_flat_data_type flat_data;
  
  TRY(ppr_flatten_track_list(context, track_list, &flat_data));
  TRY(ppr_write_flat_data(context, filename, flat_data));
  ppr_free_flat_data(flat_data);
}

/******************************************************************************/

void sdk_io_utils_read_gallery(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_type *gallery)
{
  TRY(ppr_read_gallery(context, filename, gallery));
}

void sdk_io_utils_write_gallery(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_type gallery)
{
  TRY(ppr_write_gallery(context, filename, gallery));
}

/******************************************************************************/

void sdk_io_utils_read_gallery_group(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_group_type *gallery_group)
{
  TRY(ppr_read_gallery_group(context, filename, gallery_group));
}

void sdk_io_utils_write_gallery_group(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_group_type gallery_group)
{
  TRY(ppr_write_gallery_group(context, filename, gallery_group));
}

/******************************************************************************/

void sdk_io_utils_read_similarity_matrix(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_type *query_gallery,
  ppr_gallery_type *target_gallery,
  ppr_similarity_matrix_type *similarity_matrix)
{
  TRY(ppr_read_similarity_matrix(context, filename, query_gallery, target_gallery, similarity_matrix));
}

void sdk_io_utils_write_similarity_matrix(
  ppr_context_type context,
  const char *filename,
  ppr_similarity_matrix_type similarity_matrix)
{
  TRY(ppr_write_similarity_matrix(context, filename, similarity_matrix));
}

/******************************************************************************/
