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


#ifndef __SDK_IO_UTILS_H
#define __SDK_IO_UTILS_H

#include "pittpatt_sdk.h"

/* If C++ then we need to extern "C". Compiler defines __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Functions
 ******************************************************************************/

void sdk_io_utils_read_face(
  ppr_context_type context,
  const char *filename,
  ppr_face_type *face);

void sdk_io_utils_write_face(
  ppr_context_type context,
  const char *filename,
  ppr_face_type face);

/******************************************************************************/

void sdk_io_utils_read_face_list(
  ppr_context_type context,
  const char *filename,
  ppr_face_list_type *face_list);

void sdk_io_utils_write_face_list(
  ppr_context_type context,
  const char *filename,
  ppr_face_list_type face_list);

/******************************************************************************/

void sdk_io_utils_read_track_list(
  ppr_context_type context,
  const char *filename,
  ppr_track_list_type *track_list);

void sdk_io_utils_write_track_list(
  ppr_context_type context,
  const char *filename,
  ppr_track_list_type track_list);

/******************************************************************************/

void sdk_io_utils_read_gallery(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_type *gallery);

void sdk_io_utils_write_gallery(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_type gallery);

/******************************************************************************/

void sdk_io_utils_read_gallery_group(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_group_type *gallery_group);

void sdk_io_utils_write_gallery_group(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_group_type gallery);

/******************************************************************************/

void sdk_io_utils_read_similarity_matrix(
  ppr_context_type context,
  const char *filename,
  ppr_gallery_type *query_gallery,
  ppr_gallery_type *target_gallery,
  ppr_similarity_matrix_type *similarity_matrix);

void sdk_io_utils_write_similarity_matrix(
  ppr_context_type context,
  const char *filename,
  ppr_similarity_matrix_type similarity_matrix);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __SDK_IO_UTILS_H */
