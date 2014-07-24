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

#include "sdk_print_utils.h"
#include "pittpatt_sdk.h"
#include "sdk_utils.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/

static const char *landmark_category_strings[] =
{
  "left eye",
  "right eye",
  "nose base",
  "nose bridge",
  "eye nose",
  "left upper cheek",
  "left lower cheek",
  "right upper cheek",
  "right lower cheek"
};

/*******************************************************************************
 * Print Functions
 ******************************************************************************/

void sdk_print_utils_print_boolean(int boolean)
{
  if (boolean)
    printf("true\n");
  else
    printf("false\n");
}

/******************************************************************************/

void sdk_print_utils_print_landmark_category(ppr_landmark_category_type landmark_category)
{
  if ((int)landmark_category < 0 || landmark_category >= PPR_NUM_LANDMARK_CATEGORIES)
    printf("invalid landmark category");
  else
    printf("%s", landmark_category_strings[landmark_category]);

  printf("\n");
}

/******************************************************************************/

void sdk_print_utils_print_coordinate(ppr_coordinate_type coordinate)
{
  printf("(%.3f, %.3f)\n", coordinate.x, coordinate.y);
}

/******************************************************************************/

void sdk_print_utils_print_dimensions(ppr_dimensions_type dimensions)
{
  printf("%.3f x %.3f (w x h)\n", dimensions.width, dimensions.height);
}

/******************************************************************************/

void sdk_print_utils_print_rotation(ppr_rotation_type rotation)
{
  printf("y=%.2f, p=%.2f, r=%.2f\n", rotation.yaw, rotation.pitch, rotation.roll);
}

/******************************************************************************/

void sdk_print_utils_print_landmark(ppr_landmark_type landmark)
{
  printf("   - category        : ");
  sdk_print_utils_print_landmark_category(landmark.category);
  printf("   - position        : ");
  sdk_print_utils_print_coordinate(landmark.position);
}

/******************************************************************************/

void sdk_print_utils_print_polygon(ppr_polygon_type polygon)
{
  int i;

  printf(" - num_points : %d\n", polygon.num_points);
  for (i = 0; i < polygon.num_points; ++i)
    printf("  - point[%d] : (%8.3f, %8.3f)\n", i, polygon.points[i].x, polygon.points[i].y);
}

/******************************************************************************/

void sdk_print_utils_print_id_list(ppr_id_list_type id_list)
{
  int i;
  
  printf(" - length : %d\n", id_list.length);
  printf(" - ids    : [");
  for (i = 0; i < id_list.length; ++i)
  {
    printf("%d", id_list.ids[i]);
    if (i < id_list.length - 1)
      printf(", ");
  }
  printf("]\n");
}

/******************************************************************************/

void sdk_print_utils_print_score_list(ppr_score_list_type score_list)
{
  int i;
  
  printf(" - length : %d\n", score_list.length);
  for (i = 0; i < score_list.length; ++i)
    printf("  - %d : %5.3f\n", score_list.ids[i], score_list.scores[i]);
}

/******************************************************************************/

void sdk_print_utils_print_search_region(ppr_search_region_type search_region)
{
  printf("  - top_left                       : ");
  sdk_print_utils_print_coordinate(search_region.top_left);

  printf("  - bottom_right                   : ");
  sdk_print_utils_print_coordinate(search_region.bottom_right);
}

/******************************************************************************/

void sdk_print_utils_print_detection_settings(ppr_detection_settings_type detection_settings,
                                              int print_disabled_settings)
{
  printf(" DETECTION SETTINGS\n");
  printf(" - enable                          : ");
  sdk_print_utils_print_boolean(detection_settings.enable);
  if (detection_settings.enable || print_disabled_settings)
  {
    printf(" - min_size                        : %d\n", detection_settings.min_size);
    printf(" - max_size                        : %d\n", detection_settings.max_size);
    printf(" - adaptive_min_size               : %.2f\n", detection_settings.adaptive_min_size);
    printf(" - adaptive_max_size               : %.2f\n", detection_settings.adaptive_max_size);
    printf(" - threshold                       : %.2f\n", detection_settings.threshold);
    printf(" - search_region                   : ");
    sdk_print_utils_print_boolean(detection_settings.search_region.enable);
    if (detection_settings.search_region.enable || print_disabled_settings)
      sdk_print_utils_print_search_region(detection_settings.search_region);
    printf(" - use_serial_face_detection       : ");
    sdk_print_utils_print_boolean(detection_settings.use_serial_face_detection);
    printf(" - num_threads                     : %d\n", detection_settings.num_threads);
    printf(" - image_queue_size                : %d\n", detection_settings.image_queue_size);
    printf(" - search_pruning_aggressiveness   : %d\n", detection_settings.search_pruning_aggressiveness);
    printf(" - detect_best_face_only           : ");
    sdk_print_utils_print_boolean(detection_settings.detect_best_face_only);
    printf(" - extract_thumbnails              : ");
    sdk_print_utils_print_boolean(detection_settings.extract_thumbnails);
    printf(" - results_callback                : %s\n", (detection_settings.results_callback)?("set"):("not set")); 
  }
}

/******************************************************************************/

void sdk_print_utils_print_landmark_range(ppr_landmark_range_type landmark_range)
{
  switch (landmark_range)
  {
    case PPR_LANDMARK_RANGE_FRONTAL:
      printf("frontal\n");
      break;
    case PPR_LANDMARK_RANGE_EXTENDED:
      printf("extended\n");
      break;
    case PPR_LANDMARK_RANGE_FULL:
      printf("full\n");
      break;
    case PPR_LANDMARK_RANGE_COMPREHENSIVE:
      printf("comprehensive\n");
      break;
    default:
      printf("INVALID\n");
  }
}

/******************************************************************************/

void sdk_print_utils_print_landmark_settings(ppr_landmark_settings_type landmark_settings,
                                             int print_disabled_settings)
{
  printf(" LANDMARK SETTINGS\n");
  printf(" - enable                          : ");
  sdk_print_utils_print_boolean(landmark_settings.enable);
  if (landmark_settings.enable || print_disabled_settings)
  {
    printf(" - landmark_range                  : ");
    sdk_print_utils_print_landmark_range(landmark_settings.landmark_range);
    printf(" - manually_detect_landmarks       : ");
    sdk_print_utils_print_boolean(landmark_settings.manually_detect_landmarks);
  }
}

/******************************************************************************/

void sdk_print_utils_print_tracking_settings(ppr_tracking_settings_type tracking_settings,
                                             int print_disabled_settings)
{
  printf(" TRACKING SETTINGS\n");
  printf(" - enable                          : ");
  sdk_print_utils_print_boolean(tracking_settings.enable);
  if (tracking_settings.enable || print_disabled_settings)
  {
    printf(" - cutoff                          : %d\n", tracking_settings.cutoff);
    printf(" - num_visual_instances            : %d\n", tracking_settings.num_visual_instances);
    printf(" - discard_completed_tracks        : ");
    sdk_print_utils_print_boolean(tracking_settings.discard_completed_tracks);
    printf(" - enable_shot_boundary_detection  : ");
    sdk_print_utils_print_boolean(tracking_settings.enable_shot_boundary_detection);
    if (tracking_settings.enable_shot_boundary_detection || print_disabled_settings)
      printf(" - shot_boundary_threshold         : %.2f\n", tracking_settings.shot_boundary_threshold);
  }
}

/******************************************************************************/

void sdk_print_utils_print_recognizer(ppr_recognizer_type recognizer)
{
  switch (recognizer)
  {
    case PPR_RECOGNIZER_FRONTAL:
      printf("frontal\n");
      break;
    case PPR_RECOGNIZER_MULTI_POSE:
      printf("multi-pose\n");
      break;
    case PPR_RECOGNIZER_FAST_MULTI_POSE:
      printf("fast multi-pose\n");
      break;
    default:
      printf("INVALID\n");
  }
}

/******************************************************************************/

void sdk_print_utils_print_recognition_settings(ppr_recognition_settings_type recognition_settings,
                                                int print_disabled_settings)
{
  printf(" RECOGNITION SETTINGS\n");
  printf(" - enable_extraction               : ");
  sdk_print_utils_print_boolean(recognition_settings.enable_extraction);
  printf(" - enable_comparison               : ");
  sdk_print_utils_print_boolean(recognition_settings.enable_comparison);
  if (recognition_settings.enable_extraction || recognition_settings.enable_comparison || print_disabled_settings)
  {
    printf(" - recognizer                      : ");
    sdk_print_utils_print_recognizer(recognition_settings.recognizer);
    printf(" - num_comparison_threads          : %d\n", recognition_settings.num_comparison_threads);
    printf(" - automatically_extract_templates : ");
    sdk_print_utils_print_boolean(recognition_settings.automatically_extract_templates);
    printf(" - extract_thumbnails              : ");
    sdk_print_utils_print_boolean(recognition_settings.extract_thumbnails);
  }
}

/******************************************************************************/

void sdk_print_utils_print_settings(ppr_settings_type settings,
                                    int print_disabled_settings)
{
  printf("----------------------------------------------------------------------\n");
  printf(" Settings for %s\n", ppr_version());
  printf("----------------------------------------------------------------------\n");
  
  sdk_print_utils_print_detection_settings(settings.detection, print_disabled_settings);
  printf("\n");
  
  sdk_print_utils_print_landmark_settings(settings.landmarks, print_disabled_settings);
  printf("\n");

  sdk_print_utils_print_tracking_settings(settings.tracking, print_disabled_settings);
  printf("\n");

  sdk_print_utils_print_recognition_settings(settings.recognition, print_disabled_settings);
  
  printf("----------------------------------------------------------------------\n");
}

/******************************************************************************/

void sdk_print_utils_print_enabled_settings(ppr_settings_type settings)
{
  sdk_print_utils_print_settings(settings, 0);
}

/******************************************************************************/

void sdk_print_utils_print_all_settings(ppr_settings_type settings)
{
  sdk_print_utils_print_settings(settings, 1);
}

/******************************************************************************/

void sdk_print_utils_print_face(ppr_context_type context,
                                ppr_face_type face)
{
  int rv;
  ppr_string_type face_string;
  ppr_face_attributes_type face_attributes;
  ppr_landmark_list_type landmark_list;
  ppr_polygon_type face_boundary;
  ppr_raw_image_type thumbnail;
  ppr_settings_type settings;
  int is_extractable;
  int has_template;
  
  TRY(ppr_get_face_attributes(face, &face_attributes));
  sdk_print_utils_print_face_attributes(face_attributes);

  TRY(ppr_get_face_landmarks(face, &landmark_list));
  sdk_print_utils_print_landmark_list(landmark_list);

  TRY(ppr_get_face_boundary(face, PPR_BOUNDING_BOX_STYLE_DEFAULT, &face_boundary));
  printf("----------------------------------------------------------------------\n");
  printf(" Default Bounding Box Style \n");
  printf("----------------------------------------------------------------------\n");
  sdk_print_utils_print_polygon(face_boundary);

  printf("----------------------------------------------------------------------\n");
  printf("----------------------------------------------------------------------\n");
  printf(" Additional Info\n");
  printf("----------------------------------------------------------------------\n");
  TRY(ppr_get_face_string(context, face, &face_string));
  printf(" - String: %s\n", face_string.str);
  
  rv = ppr_get_face_thumbnail(context, face, &thumbnail);
  if (rv == PPR_SUCCESS)
  {
    printf(" - Face HAS THUMBNAIL\n");
    ppr_raw_image_free(thumbnail);
  }
  else if (rv == PPR_NO_THUMBNAIL)
  {
    printf(" - Face DOES NOT have thumbnail\n");
  }
  else
  {
    fprintf(stderr, "ERROR - %s: ppr_get_face_thumbnail", ppr_error_message(rv));
    fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

  TRY(ppr_get_settings(context, &settings));
  if (settings.recognition.enable_extraction)
  {
    TRY(ppr_is_template_extractable(context, face, &is_extractable));
    if (is_extractable)
      printf(" - Template CAN be extracted\n");
    else
      printf(" - Template CANNOT be extracted\n");

    TRY(ppr_face_has_template(context, face, &has_template));
    if (has_template)
      printf(" - Face HAS template\n");
    else
      printf(" - Face DOES NOT have template\n");
  }

  printf("----------------------------------------------------------------------\n");

  ppr_free_string(face_string);
  ppr_free_landmark_list(landmark_list);
  ppr_free_polygon(face_boundary);
}

/******************************************************************************/

void sdk_print_utils_print_face_attributes(ppr_face_attributes_type face_attributes)
{
  printf("----------------------------------------------------------------------\n");
  printf(" Face Attributes\n");
  printf("----------------------------------------------------------------------\n");
  
  printf(" - position         : ");
  sdk_print_utils_print_coordinate(face_attributes.position);
  
  printf(" - dimensions       : ");
  sdk_print_utils_print_dimensions(face_attributes.dimensions);
  
  printf(" - rotation         : ");
  sdk_print_utils_print_rotation(face_attributes.rotation);
  
  printf(" - size             : %.2f\n", face_attributes.size);
  printf(" - confidence       : %.2f\n", face_attributes.confidence);
  
  printf(" - num_landmarks    : %d\n", face_attributes.num_landmarks);
  
  if (face_attributes.tracking_info.frame_number != -1 &&
      face_attributes.tracking_info.track_id != -1 &&
      face_attributes.tracking_info.confidence_level != -1)
  {
    printf(" - tracking info\n");
    printf("  - frame_number     : %d\n", face_attributes.tracking_info.frame_number);
    printf("  - tracked_id       : %d\n", face_attributes.tracking_info.track_id);
    printf("  - confidence_level : %d\n", face_attributes.tracking_info.confidence_level);
  }

  printf("----------------------------------------------------------------------\n");
}

/******************************************************************************/

void sdk_print_utils_print_landmark_list(ppr_landmark_list_type landmark_list)
{
  int i;

  printf("----------------------------------------------------------------------\n");
  printf(" Landmarks\n");
  printf("----------------------------------------------------------------------\n");

  printf(" - num_landmarks : %d\n", landmark_list.length);
  
  for (i = 0; i < landmark_list.length; ++i)
  {
    printf("  - landmark[%d]\n", i);
    sdk_print_utils_print_landmark(landmark_list.landmarks[i]);
  }
  printf("----------------------------------------------------------------------\n");
}

/******************************************************************************/

void sdk_print_utils_print_face_list(ppr_face_list_type face_list)
{
  int i;
  ppr_face_attributes_type face_attributes;
  ppr_landmark_list_type landmark_list;

  if (face_list.length >= 0)
  {
    printf("----------------------------------------------------------------------\n");
    printf(" Number of Faces: %d\n", face_list.length);
    printf("     source name: %s\n", face_list.source_name.str);
    printf("----------------------------------------------------------------------\n");

    for (i = 0; i < face_list.length; i++)
    {
      printf("----------------------------------------------------------------------\n");
      printf(" Face[%d]\n", i);
      printf("----------------------------------------------------------------------\n");
      TRY(ppr_get_face_attributes(face_list.faces[i], &face_attributes));
      sdk_print_utils_print_face_attributes(face_attributes);
      TRY(ppr_get_face_landmarks(face_list.faces[i], &landmark_list));
      sdk_print_utils_print_landmark_list(landmark_list);
      ppr_free_landmark_list(landmark_list);
      printf("\n");
    }

    printf("----------------------------------------------------------------------\n");
  }
  else
  {
    printf("----------------------------------------------------------------------\n");
    printf(" INVALID FACE LIST\n");
    printf("----------------------------------------------------------------------\n");
  }
}

/******************************************************************************/

void sdk_print_utils_print_image_results_list(ppr_image_results_list_type image_results_list)
{
  int i;

  if (image_results_list.length >= 0)
  {
    printf("----------------------------------------------------------------------\n");
    printf(" Number of Results: %d\n", (int)image_results_list.length);
    printf("----------------------------------------------------------------------\n");

    for (i = 0; i < image_results_list.length; i++)
    {
      printf("======================================================================\n");
      printf(" Result[%d]\n", i);
      printf("======================================================================\n");
      sdk_print_utils_print_face_list(image_results_list.face_lists[i]);
      printf("\n");
    }
 
    printf("----------------------------------------------------------------------\n");
  }
  else
  {
    printf("----------------------------------------------------------------------\n");
    printf(" INVALID IMAGE RESULTS LIST\n");
    printf("----------------------------------------------------------------------\n");
  }
}

/******************************************************************************/
void sdk_print_utils_print_track_summary(ppr_track_summary_type track_summary)
{
  printf(" - track_id           : %d\n", track_summary.track_id);
  printf(" - start_frame_number : %d\n", track_summary.start_frame_number);
  printf(" - stop_frame_number  : %d\n", track_summary.stop_frame_number);
  printf(" - start_position     : ");
  sdk_print_utils_print_coordinate(track_summary.start_position);
  printf(" - stop_position      : ");
  sdk_print_utils_print_coordinate(track_summary.stop_position);
  printf(" - min_dimensions     : ");
  sdk_print_utils_print_dimensions(track_summary.min_dimensions);
  printf(" - max_dimensions     : ");
  sdk_print_utils_print_dimensions(track_summary.max_dimensions);
  printf(" - avg_dimensions     : ");
  sdk_print_utils_print_dimensions(track_summary.avg_dimensions);
  printf(" - min_confidence     : %.3f\n", track_summary.min_confidence);
  printf(" - max_confidence     : %.3f\n", track_summary.max_confidence);
  printf(" - avg_confidence     : %.3f\n", track_summary.avg_confidence);
}

/******************************************************************************/

void sdk_print_utils_print_track(ppr_track_type track)
{
  sdk_print_utils_print_track_summary(track.track_summary);
  printf("----------------------------------------------------------------------\n");
}

/******************************************************************************/

void sdk_print_utils_print_track_list(ppr_track_list_type track_list)
{
  int i;

  printf("----------------------------------------------------------------------\n");
  printf(" Number of Tracks: %d\n", track_list.length);
  printf("      source name: %s\n", track_list.source_name.str);
  printf("----------------------------------------------------------------------\n");
  
  for (i = 0; i < track_list.length; i++)
  {
    printf("----------------------------------------------------------------------\n");
    printf(" Track[%d]\n", i);
    printf("----------------------------------------------------------------------\n");
    sdk_print_utils_print_track(track_list.tracks[i]);
    printf("\n");
  }

  if (track_list.length == 0)
    printf("\n");
}

/******************************************************************************/

void sdk_print_utils_print_gallery(ppr_context_type context, ppr_gallery_type gallery)
{
  int i, j;
  ppr_string_type string;
  ppr_id_list_type id_list;
  ppr_id_list_type subject_id_list;
  ppr_string_type subject_string;

  TRY(ppr_get_gallery_string(context, gallery, &string));
  printf("string: %s\n", string.str);
  ppr_free_string(string);
  
  TRY(ppr_get_face_id_list (context, gallery, &id_list));
  printf("number of faces: %d\n", id_list.length);
  printf("  face ids: ");
  for (i = 0; i < id_list.length; i++)
    printf("%d ", id_list.ids[i]);
  printf("\n");
  ppr_free_id_list(id_list);

  TRY(ppr_get_subject_id_list(context, gallery, &subject_id_list));
  printf("----------------------------------------------------------------------\n");
  printf("number of subjects: %d\n", subject_id_list.length);
  printf("----------------------------------------------------------------------\n");
  for (i = 0; i < subject_id_list.length; i++)
  {
    TRY(ppr_get_face_id_list_for_subject(context, gallery, subject_id_list.ids[i], &id_list));
    printf("subject %d - %d face%s\n", 
           subject_id_list.ids[i], 
           id_list.length, 
           (id_list.length == 1)?"":"s");
    printf("  face ids: ");
    for (j = 0; j < id_list.length; j++)
      printf("%d ", id_list.ids[j]);
    printf("\n");
    ppr_free_id_list(id_list);
    printf("  subject diffs: ");
    TRY(ppr_get_subject_diffs(context, gallery, subject_id_list.ids[i], &id_list));
    for (j = 0; j < id_list.length; j++)
      printf("%d ", id_list.ids[j]);
    printf("\n");
    printf("  string: ");
    TRY(ppr_get_subject_string(context, gallery, subject_id_list.ids[i], &subject_string));
    printf("%s\n", subject_string.str);
    ppr_free_string(subject_string);
    ppr_free_id_list(id_list);
  }
  ppr_free_id_list(subject_id_list);
}

/******************************************************************************/

void sdk_print_utils_print_similarity_matrix(ppr_context_type context, ppr_similarity_matrix_type similarity_matrix)
{
  ppr_gallery_ref_type query_gallery_ref, target_gallery_ref;
  ppr_id_list_type q_id_list, t_id_list;
  int i, j;
  float score;
  int enabled;
  TRY(ppr_get_gallery_references(context, similarity_matrix, &query_gallery_ref, &target_gallery_ref));

  if (*target_gallery_ref == *query_gallery_ref)
  {
    printf("Self-Similarity Matrix\n");
    sdk_print_utils_print_gallery(context, *query_gallery_ref);
  }
  else
  {
    printf("Query Gallery:\n");
    sdk_print_utils_print_gallery(context, *query_gallery_ref);
    printf("\nTarget Gallery:\n");
    sdk_print_utils_print_gallery(context, *target_gallery_ref);
  }

  printf("\n");
  TRY(ppr_is_transitive_matching_enabled(context, similarity_matrix, &enabled));
  printf("Transitive matching:");
  if (enabled)
    printf(" enabled\n");
  else
    printf(" disabled\n");

  printf("\nScore Matrix:\n");
  TRY(ppr_get_face_id_list(context, *query_gallery_ref, &q_id_list));
  TRY(ppr_get_face_id_list(context, *target_gallery_ref, &t_id_list));
  
  printf(" IDs ");
  for (j = 0; j < t_id_list.length; j++)
    printf("%6d ", t_id_list.ids[j]);
  printf("\n");
  
  for (i = 0; i < q_id_list.length; i++)
  {
    printf("%5d ", q_id_list.ids[i]);
    for (j = 0; j < t_id_list.length; j++)
    {
      TRY(ppr_get_face_similarity_score(context, similarity_matrix, q_id_list.ids[i], t_id_list.ids[j], &score));
      printf("%6.2f ", score);
    }
    printf("\n");
  }
  ppr_free_id_list(q_id_list);
  ppr_free_id_list(t_id_list);
  ppr_release_gallery_reference(query_gallery_ref);
  ppr_release_gallery_reference(target_gallery_ref);
}

/******************************************************************************/

void sdk_print_utils_print_gallery_group(ppr_context_type context, ppr_gallery_group_type gallery_group)
{
  int i;
  ppr_id_list_type id_list;
  ppr_gallery_ref_type gallery_ref;
  ppr_id_list_type id_list_a;
  ppr_id_list_type id_list_b;

  TRY(ppr_get_gallery_id_list_from_gallery_group(context, gallery_group, &id_list));
  printf("number of galleries: %d\n", id_list.length);
  printf("  gallery ids: ");
  for (i = 0; i < id_list.length; ++i)
    printf("%d ", id_list.ids[i]);
  printf("\n");

  for (i = 0; i < id_list.length; ++i)
  {
    printf("gallery %d: \n", id_list.ids[i]);
    TRY(ppr_get_gallery_reference_from_gallery_group(context, gallery_group, id_list.ids[i], &gallery_ref));
    sdk_print_utils_print_gallery(context, *gallery_ref);
    ppr_release_gallery_reference(gallery_ref);
  }

  TRY(ppr_get_subject_diff_pairs_from_gallery_group(context, gallery_group, &id_list_a, &id_list_b));
  printf("\n");
  printf("Subject Diffs:\n");
  for (i = 0; i < id_list_a.length; ++i)
    printf(" - %d <-> %d\n", id_list_a.ids[i], id_list_b.ids[i]);

  ppr_free_id_list(id_list);
  ppr_free_id_list(id_list_a);
  ppr_free_id_list(id_list_b);
}

/******************************************************************************/

void sdk_print_utils_print_cluster_list(ppr_cluster_list_type cluster_list)
{
  int i;
  
  for (i = 0; i < cluster_list.length; ++i)
  {
    printf("cluster %d:\n", i);
    sdk_print_utils_print_id_list(cluster_list.clusters[i]);
  }
}

/******************************************************************************/
