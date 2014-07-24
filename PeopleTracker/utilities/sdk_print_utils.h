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


#ifndef __SDK_PRINT_UTILS_H
#define __SDK_PRINT_UTILS_H

#include <stdlib.h>

#include "pittpatt_sdk.h"

/* If C++ then we need to extern "C". Compiler defines __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Functions
 ******************************************************************************/

void sdk_print_utils_print_boolean(int boolean);
void sdk_print_utils_print_landmark_category(ppr_landmark_category_type landmark_category);
void sdk_print_utils_print_landmark_range(ppr_landmark_range_type landmark_range);
void sdk_print_utils_print_coordinate(ppr_coordinate_type coordinate);
void sdk_print_utils_print_dimensions(ppr_dimensions_type dimensions);
void sdk_print_utils_print_rotation(ppr_rotation_type rotation);
void sdk_print_utils_print_landmark(ppr_landmark_type landmark);
void sdk_print_utils_print_polygon(ppr_polygon_type polygon);
void sdk_print_utils_print_id_list(ppr_id_list_type id_list);
void sdk_print_utils_print_score_list(ppr_score_list_type score_list);
void sdk_print_utils_print_search_region(ppr_search_region_type search_region);
void sdk_print_utils_print_detection_settings(ppr_detection_settings_type detection_settings,
                                              int print_disabled_settings);
void sdk_print_utils_print_landmark_settings(ppr_landmark_settings_type landmark_settings,
                                             int print_disabled_settings);
void sdk_print_utils_print_tracking_settings(ppr_tracking_settings_type tracking_settings,
                                             int print_disabled_settings);
void sdk_print_utils_print_recognizer(ppr_recognizer_type recognizer);
void sdk_print_utils_print_recognition_settings(ppr_recognition_settings_type recognition_settings,
                                                int print_disabled_settings);
void sdk_print_utils_print_settings(ppr_settings_type settings,
                                    int print_disabled_settings);
void sdk_print_utils_print_enabled_settings(ppr_settings_type settings);
void sdk_print_utils_print_all_settings(ppr_settings_type settings);
void sdk_print_utils_print_face(ppr_context_type context, ppr_face_type face);
void sdk_print_utils_print_face_attributes(ppr_face_attributes_type face_attributes);
void sdk_print_utils_print_landmark_list(ppr_landmark_list_type landmark_list);
void sdk_print_utils_print_face_list(ppr_face_list_type face_list);
void sdk_print_utils_print_image_results_list(ppr_image_results_list_type image_results_list);
void sdk_print_utils_print_track_summary(ppr_track_summary_type track_summary);
void sdk_print_utils_print_track(ppr_track_type track);
void sdk_print_utils_print_track_list(ppr_track_list_type track_list);
void sdk_print_utils_print_gallery(ppr_context_type context, ppr_gallery_type gallery);
void sdk_print_utils_print_similarity_matrix(ppr_context_type context, ppr_similarity_matrix_type similarity_matrix);
void sdk_print_utils_print_gallery_group(ppr_context_type context, ppr_gallery_group_type gallery_group);
void sdk_print_utils_print_cluster_list(ppr_cluster_list_type cluster_list);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __SDK_PRINT_UTILS_H */
