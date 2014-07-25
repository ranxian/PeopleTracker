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


#ifndef __SDK_UTILS_H
#define __SDK_UTILS_H

#include <stdlib.h>

#include "pittpatt_sdk.h"

#include "pittpatt_raw_image_io.h"
#include "pittpatt_video_io.h"


/* If C++ then we need to extern "C". Compiler defines __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Constants
 ******************************************************************************/

#define SDK_UTILS_INDENT_STR_LEN 32
#define FUNCTION_NAME_MAX        256

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define PRINT_FUNCTION_NAME(CC)                                         \
  {                                                                     \
    char _function_name[FUNCTION_NAME_MAX];                             \
    int _i;                                                             \
                                                                        \
    _i = 0;                                                             \
    while (#CC[_i] != ' ' && #CC[_i] != '(')                            \
    {                                                                   \
      _function_name[_i] = #CC[_i];                                     \
      _i++;                                                             \
    }                                                                   \
    _function_name[_i] = '\0';                                          \
    fprintf(stderr, "%s()", _function_name);                            \
  }

#define TRY(CC)                                                         \
  {                                                                     \
    ppr_error_type _err = (CC);                                         \
                                                                        \
    if (_err != PPR_SUCCESS)                                            \
    {                                                                   \
      fprintf(stderr, "ERROR - %s: ", ppr_error_message(_err));         \
      PRINT_FUNCTION_NAME(CC)                                           \
      fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                \
      ppr_finalize_sdk();                                               \
      exit(EXIT_FAILURE);                                               \
    }                                                                   \
  }

#define TRY_RAW_IMAGE(CC)                                               \
  {                                                                     \
    ppr_raw_image_error_type _err = (CC);                               \
    if (_err != PPR_RAW_IMAGE_SUCCESS)                                  \
    {                                                                   \
      fprintf(stderr, "ERROR - %s: ", ppr_raw_image_error_message(_err)); \
      PRINT_FUNCTION_NAME(CC)                                           \
      fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                \
      ppr_finalize_sdk();                                               \
      exit(EXIT_FAILURE);                                               \
    }                                                                   \
  }

#define TRY_VIDEO(CC)                                                   \
  {                                                                     \
    ppr_video_io_error_type _err = (CC);                                \
    if (_err != PPR_VIDEO_IO_SUCCESS)                                   \
    {                                                                   \
      fprintf(stderr, "ERROR - %s: ", ppr_video_io_error_message(_err)); \
      PRINT_FUNCTION_NAME(CC)                                           \
      fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                \
      ppr_finalize_sdk();                                               \
      exit(EXIT_FAILURE);                                               \
    }                                                                   \
  }

/******************************************************************************/

/* Returns the number of CPUs in the system */
int sdk_utils_get_num_cpus(void);

/* Creates a directory */
void sdk_utils_make_directory(
  const char *directory_name);

/* Write all visual instances of all completed tracks to specified directory */
void sdk_utils_write_visual_instances(const char *directory,
                                      const char *output_image_format,
                                      ppr_track_list_type track_list);

/*****************************************************************************
 * Miscellanous Recognition Functions
 *****************************************************************************/

/* Writes face thumbnail to the specified file.  Writes gray image if face has no thumbnail */
void sdk_utils_write_face_thumbnail(
  const char *file_name,
  ppr_context_type context,
  ppr_face_type the_face);

/* Writes thumbnails of all faces in a subject to a directory */
void sdk_utils_write_subject_thumbnails(
  const char *directory,
  ppr_context_type context,
  ppr_gallery_type gallery,
  int subject_id);

/* Writes a directory structure of thumbnails where each directory is a single cluster */
void sdk_utils_write_cluster_thumbnails(
  const char *directory,
  ppr_context_type context,
  ppr_gallery_type gallery,
  ppr_cluster_list_type cluster_list);

/*****************************************************************************
 * Data Types
 *****************************************************************************/

typedef struct
{
  int length;       /* Number of filenames in array */
  char **filenames; /* Array of filenames           */
} sdk_utils_file_list_type;

/*****************************************************************************
 * File List Functions
 *****************************************************************************/

/* Returns 1 if the path specifies a directory, 0 otherwise */
int sdk_utils_is_directory(
  const char *path);

/* Gets a list of files from a directory (does not include directories) */
void sdk_utils_get_file_list_from_directory(
  const char* parent_directory,
  sdk_utils_file_list_type *file_list);

/* Gets a list of directories from a directory (does not include files) */
void sdk_utils_get_directory_list_from_directory(
  const char* parent_directory,
  sdk_utils_file_list_type *directory_list);

/* Loads a file list from a file to an array of strings */
void sdk_utils_read_file_list(
  const char* filename,
  sdk_utils_file_list_type *file_list);

/* Combines multiple files / file lists into a single list - a .txt extension is assumed to be a list */
void sdk_utils_create_combined_file_list(
  char *filenames[],
  int n_filenames,
  sdk_utils_file_list_type *file_list);

/* Frees memory used to hold file list components */
void sdk_utils_free_file_list(
  sdk_utils_file_list_type file_list);

/*****************************************************************************
 * Filename Manipulation Functions
 *****************************************************************************/

/*
 * The filename manipulation functions use the following naming convention:
 *
 * |---------path----------|
 * |                       |
 * |         |--filename---|
 * |         |             |
 * directory/title.extension
 * |             |
 * |--basename---|
 *
 */

/* Get directory from a string */
void sdk_utils_get_file_directory(
  const char *input,
  char *directory);

/* Get title from a string */
void sdk_utils_get_file_title(
  const char *input,
  char *title);

/* Get extension from a string */
void sdk_utils_get_file_extension(
  const char *input,
  char *extension);

/* Get basename from a string */
void sdk_utils_get_file_basename(
  const char *input,
  char *basename);

/* Get filename from a string */
void sdk_utils_get_file_filename(
  const char *input,
  char *filename);

/*****************************************************************************
 * Error Checking Functions
 *****************************************************************************/

/* Prints message if err is anything other than PPR_SUCCESS, exits if fatal */ 
void sdk_utils_check_ppr_error(
  ppr_error_type err,
  const char *function_name);

/* Prints message and exits if err is not PPR_RAW_IMAGE_SUCCESS */ 
void sdk_utils_check_ppr_raw_image_error(
  ppr_raw_image_error_type err,
  const char *function_name);

/* Prints message and exits if err is not PPR_VIDEO_IO_SUCCESS */ 
void sdk_utils_check_ppr_video_io_error(
  ppr_video_io_error_type err,
  const char *function_name);

/* Prints message and exits if input is NULL */ 
void sdk_utils_check_allocation(
  void *input,
  const char *input_name,
  const char *calling_function_name);

/* Prints message and exits if file descriptor is NULL */ 
void sdk_utils_check_file_open(
  FILE *fid,
  const char *file_name,
  const char *calling_function_name);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __SDK_UTILS_H */
