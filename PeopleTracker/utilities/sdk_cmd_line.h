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
 * File: sdk_cmd_line.h
 *
 * This file provides utility functions that are not part of the PittPatt SDK,
 * but can be useful when writing programs that use the SDK.
 *
 * These functions provide an easy way to populate the SDK settings via
 * command line parameters.
 */

#ifndef __SDK_CMD_LINE_H
#define __SDK_CMD_LINE_H

#include "pittpatt_types.h"

/* If C++ then we need to __extern "C". Compiler defines __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Data Types
 *****************************************************************************/

typedef enum 
{
  SDK_CMD_NONE                           = 0x00000,
  SDK_CMD_MIN_SIZE                       = 0x00001,
  SDK_CMD_MAX_SIZE                       = 0x00002,
  SDK_CMD_ADAPTIVE_MIN_SIZE              = 0x00004,
  SDK_CMD_ADAPTIVE_MAX_SIZE              = 0x00008,
  SDK_CMD_DETECTION_THRESHOLD            = 0x00010,
  SDK_CMD_SEARCH_REGION                  = 0x00020,
  SDK_CMD_NUM_THREADS                    = 0x00040,
  SDK_CMD_SEARCH_PRUNING_AGGRESSIVENESS  = 0x00080,
  SDK_CMD_BEST_FACE_ONLY                 = 0x00100,
  SDK_CMD_EXTRACT_THUMBNAILS             = 0x00200,
  SDK_CMD_DETECT_LANDMARKS               = 0x00400,
  SDK_CMD_LANDMARK_RANGE                 = 0x00800,
  SDK_CMD_TRACKING_CUTOFF                = 0x01000,
  SDK_CMD_NUM_VISUAL_INSTANCES           = 0x02000,
  SDK_CMD_SHOT_BOUNDARY_DETECTION        = 0x04000,
  SDK_CMD_SHOT_BOUNDARY_THRESHOLD        = 0x08000,
  SDK_CMD_RECOGNIZER_TYPE                = 0x10000,
  SDK_CMD_DETECTION_SETTINGS             = 0x003ff,
  SDK_CMD_LANDMARK_SETTINGS              = 0x00c00,
  SDK_CMD_TRACKING_SETTINGS              = 0x0f000,
  SDK_CMD_RECOGNITION_SETTINGS           = 0x10040,
  SDK_CMD_ALL_SETTINGS                   = 0x1FFFF
} sdk_cmd_line_settings_selection_type;

typedef enum
{
  SDK_CMD_OPT_NONE                   = 0x0000,
  SDK_CMD_OPT_MODELS_PATH            = 0x0001,
  SDK_CMD_OPT_VERBOSE                = 0x0002,
  SDK_CMD_OPT_BINARY_OUTPUT          = 0x0004,
  SDK_CMD_OPT_OVERLAYS               = 0x0008,
  SDK_CMD_OPT_WRITE_THUMBNAILS       = 0x0010,
  SDK_CMD_OPT_AGRESSIVENESS          = 0x0020,
  SDK_CMD_OPT_MAX_DISPERSED          = 0x0040,
  SDK_CMD_OPT_NUM_REP_FACES          = 0x0080,
  SDK_CMD_OPT_KNOWN_GALLERY          = 0x0100,
  SDK_CMD_OPT_MATCH_THRESHOLD        = 0x0200,
  SDK_CMD_OPT_OVERLAY_FRAME_NUMS     = 0x0400,
  SDK_CMD_OPT_WRITE_VISUAL_INSTANCES = 0x0800,
  SDK_CMD_OPT_SUBJECTS_UNIQUE        = 0x1000,
  SDK_CMD_OPT_MAX_LENGTH             = 0x2000,
  SDK_CMD_OPT_RANK_THRESHOLD         = 0x4000,
  SDK_CMD_OPT_TRANS_MATCHING         = 0x8000
} sdk_cmd_line_options_selection_type;

typedef struct
{
  char *models_path;
  int verbose;
  int binary_output;
  int overlays;
  int write_thumbnails;
  int aggressiveness;
  int max_dispersed;
  int num_rep_faces;
  float match_threshold;
  char *known_gallery;
  int overlay_frame_nums;
  int write_visual_instances;
  int subjects_unique_within_image;
  int use_trans_matching;
  int max_length;
  float rank_threshold;
} sdk_cmd_line_options_type;

/*****************************************************************************
 * Functions
 *****************************************************************************/

int sdk_cmd_line_parse(int argc, 
                       char *argv[], 
                       const char *usage_args,
                       const char *example,
                       sdk_cmd_line_settings_selection_type settings_selection, 
                       sdk_cmd_line_options_selection_type options_selection,
                       ppr_settings_type *settings,
                       sdk_cmd_line_options_type *options);

void sdk_cmd_line_print_usage_and_exit(const char *exe_name,
                                       const char *usage_args,
                                       const char *example,
                                       sdk_cmd_line_settings_selection_type settings_selection,
                                       sdk_cmd_line_options_selection_type options_selection,
                                       ppr_settings_type default_settings,
                                       sdk_cmd_line_options_type default_options);

sdk_cmd_line_options_type sdk_cmd_line_get_default_options(void);

void sdk_cmd_line_free_options(sdk_cmd_line_options_type options);

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __SDK_CMD_LINE_H */
