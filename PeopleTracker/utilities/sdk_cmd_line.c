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
 * File: sdk_cmd_line.c
 *
 * This file provides utility functions that are not part of the PittPatt
 * SDK, but can be useful when writing programs that use the SDK.  The
 * functions provide and easy way to parse commadn line arguments.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "sdk_cmd_line.h"
#include "sdk_cmd_line_utils.h"
#include "models_path.h"

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/*****************************************************************************
 * Local Function Prototypes
 *****************************************************************************/

static const char* sdk_cmd_line_get_landmark_range_string(ppr_landmark_range_type landmark_range);

static const char* sdk_cmd_line_get_recognizer_string(ppr_recognizer_type recognizer);

static const char* sdk_cmd_line_utils_on_or_off(int val);

static int sdk_cmd_line_get_on_or_off(char *str, int *on_or_off);

static int sdk_cmd_line_get_landmark_range(char *landmark_range_str, ppr_landmark_range_type *landmark_range);

static int sdk_cmd_line_get_recognizer_type(char *recognizer_str, ppr_recognizer_type *recognizer);

int sdk_cmd_line_get_search_region_from_string(const char *search_region_str,
                                               int *x1,
                                               int *y1,
                                               int *x2,
                                               int *y2);

/*****************************************************************************
 * Public Functions
 *****************************************************************************/

int sdk_cmd_line_parse(int argc,
                       char *argv[],
                       const char *usage_args,
                       const char *example,
                       sdk_cmd_line_settings_selection_type settings_selection,
                       sdk_cmd_line_options_selection_type options_selection,
                       ppr_settings_type *settings,
                       sdk_cmd_line_options_type *options)
{
  int rv = 0;

  int i_first_file;
  ppr_settings_type default_settings = *settings;
  sdk_cmd_line_options_type default_options = *options;

  int help = 0;
  int threads = 0;
  int x1=0, y1=0, x2=0, y2=0;
  int extract_thumbnails = 0;
  char *search_region_str = NULL;
  char *landmarks_str = NULL;
  char *landmark_range_str = NULL;
  char *best_face_str = NULL;
  char *extract_thumbnails_str = NULL;
  char *shot_boundary_detection_str = NULL;
  char *recognizer_str = NULL;
  char *binary_output_str = NULL;
  char *overlays_str = NULL;
  char *write_thumbnails_str = NULL;
  char *verbose_str = NULL;
  char *overlay_frame_nums_str = NULL;
  char *write_visual_instances_str = NULL;
  char *subjects_unique_str = NULL;
  char *trans_matching_str = NULL;

  i_first_file = sdk_cmd_line_utils_parse(argc, argv, 
                                          "-threads %d", &threads,
                                          "-search_pruning %d", &settings->detection.search_pruning_aggressiveness,
                                          "-min_size %d", &settings->detection.min_size,
                                          "-max_size %d", &settings->detection.max_size,
                                          "-adaptive_min %f", &settings->detection.adaptive_min_size,
                                          "-adaptive_max %f", &settings->detection.adaptive_max_size,
                                          "-detection_threshold %f", &settings->detection.threshold,
                                          "-region %S", &search_region_str,
                                          "-landmarks %S", &landmarks_str,
                                          "-range %S", &landmark_range_str,
                                          "-best_face %S", &best_face_str,
                                          "-extract_thumbnails %S", &extract_thumbnails_str,
                                          "-visual_instances %d", &settings->tracking.num_visual_instances,
                                          "-tracking_cutoff %d", &settings->tracking.cutoff,
                                          "-shot_boundary_detection %S", &shot_boundary_detection_str,
                                          "-shot_boundary_threshold %f", &settings->tracking.shot_boundary_threshold,
                                          "-recognizer %S", &recognizer_str,
                                          "-models %S", &options->models_path,
                                          "-binary_output %S", &binary_output_str,
                                          "-overlays %S", &overlays_str,
                                          "-write_thumbnails %S", &write_thumbnails_str,
                                          "-verbose %S", &verbose_str,
                                          "-aggressiveness %d", &options->aggressiveness,
                                          "-max_dispersed_faces %d", &options->max_dispersed,
                                          "-n_representative_faces %d", &options->num_rep_faces,
                                          "-known_gallery %S", &options->known_gallery,
                                          "-match_threshold %f", &options->match_threshold,
                                          "-overlay_frame_numbers %S", &overlay_frame_nums_str,
                                          "-write_visual_instances %S", &write_visual_instances_str,
                                          "-subjects_unique %S", &subjects_unique_str,
                                          "-max_length %d", &options->max_length,
                                          "-rank_threshold %f", &options->rank_threshold,
                                          "-transitive_matching %S", &trans_matching_str,
                                          "-help", &help,
                                          (void*) 0);

  if (i_first_file < 0)
  {
    fprintf(stderr, "ERROR: %s\n", sdk_cmd_line_utils_error_message(i_first_file));
    sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                      default_settings, default_options);
  }

  if (help)
    sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                      default_settings, default_options);

  if (threads > 0)
  {
    settings->detection.num_threads = threads;
    settings->recognition.num_comparison_threads = threads;
  }

  if (search_region_str != NULL)
  {
    settings->detection.search_region.enable = 1;
    rv = sdk_cmd_line_get_search_region_from_string(search_region_str, 
                                                    &x1, &y1, &x2, &y2);
    settings->detection.search_region.top_left.x = (float)x1;
    settings->detection.search_region.top_left.y = (float)y1;
    settings->detection.search_region.bottom_right.x = (float)x2;
    settings->detection.search_region.bottom_right.y = (float)y2;
    free(search_region_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid search region argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (landmarks_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(landmarks_str, &settings->landmarks.enable);
    free(landmarks_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid landmarks argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (landmark_range_str != NULL)
  {
    rv = sdk_cmd_line_get_landmark_range(landmark_range_str, &settings->landmarks.landmark_range);
    free(landmark_range_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid landmark range argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (best_face_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(best_face_str, &settings->detection.detect_best_face_only);
    free(best_face_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid best face argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (extract_thumbnails_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(extract_thumbnails_str, &extract_thumbnails);
    settings->detection.extract_thumbnails = extract_thumbnails;
    settings->recognition.extract_thumbnails = extract_thumbnails;
    free(extract_thumbnails_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid extract thumbnails argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (shot_boundary_detection_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(shot_boundary_detection_str, &settings->tracking.enable_shot_boundary_detection);
    free(shot_boundary_detection_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid shot boundary detection argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (recognizer_str != NULL)
  {
    rv = sdk_cmd_line_get_recognizer_type(recognizer_str, &settings->recognition.recognizer);
    free(recognizer_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid shot boundary detection argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (binary_output_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(binary_output_str, &options->binary_output);
    free(binary_output_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid binary output argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (overlays_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(overlays_str, &options->overlays);
    free(overlays_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid overlays argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (write_thumbnails_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(write_thumbnails_str, &options->write_thumbnails);
    free(write_thumbnails_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid thumbnails argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (options->models_path == NULL)
  {
    options->models_path = (char*) malloc(strlen(MODELS_PATH) + 1);
    strcpy(options->models_path, MODELS_PATH);
  }

  if (verbose_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(verbose_str, &options->verbose);
    free(verbose_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid verbose argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection, 
                                        default_settings, default_options);
    }
  }

  if (overlay_frame_nums_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(overlay_frame_nums_str, &options->overlay_frame_nums);
    free(overlay_frame_nums_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid overlay frame numbers argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection,
                                        default_settings, default_options);
    }
  }

  if (write_visual_instances_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(write_visual_instances_str, &options->write_visual_instances);
    free(write_visual_instances_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid write visual instances argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection,
                                        default_settings, default_options);
    }
  }

  if (subjects_unique_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(subjects_unique_str, &options->subjects_unique_within_image);
    free(subjects_unique_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid subjects unique argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection,
                                        default_settings, default_options);
    }
  }

  if (trans_matching_str != NULL)
  {
    rv = sdk_cmd_line_get_on_or_off(trans_matching_str, &options->use_trans_matching);
    free(trans_matching_str);
    if (rv < 0)
    {
      fprintf(stderr, "ERROR: invalid transitive matching argument\n");
      sdk_cmd_line_print_usage_and_exit(argv[0], usage_args, example, settings_selection, options_selection,
                                        default_settings, default_options);
    }
  }

  return i_first_file;
}

/*****************************************************************************/

void sdk_cmd_line_print_usage_and_exit(const char *exe_name,
                                       const char *usage_args,
                                       const char *example,
                                       sdk_cmd_line_settings_selection_type settings_selection,
                                       sdk_cmd_line_options_selection_type options_selection,
                                       ppr_settings_type default_settings,
                                       sdk_cmd_line_options_type default_options)
{
  fprintf(stderr, "\n");
  fprintf(stderr, "usage: %s [flags] %s\n", exe_name, usage_args);
  fprintf(stderr, "\n");
  fprintf(stderr, "  where flags are one or more of the following:\n");
  fprintf(stderr, "\n");

  /**********************/
  /* settings selection */
  /**********************/

  if ((SDK_CMD_NUM_THREADS & settings_selection) == SDK_CMD_NUM_THREADS)
    fprintf(stderr, "   -threads <int>                    :  number of threads to use [%d]\n",
            MAX(default_settings.detection.num_threads, default_settings.recognition.num_comparison_threads));

  if ((SDK_CMD_SEARCH_PRUNING_AGGRESSIVENESS & settings_selection) == SDK_CMD_SEARCH_PRUNING_AGGRESSIVENESS)
    fprintf(stderr, "   -search_pruning <int>             :  search pruning aggressiveness (0 - %d) [%d]\n",
            PPR_MAX_SEARCH_PRUNING_AGGRESSIVENESS, default_settings.detection.search_pruning_aggressiveness);

  if ((SDK_CMD_MIN_SIZE & settings_selection) == SDK_CMD_MIN_SIZE)
    fprintf(stderr, "   -min_size <int>                   :  minimum size (%d - %d) [%d]\n",
            PPR_MIN_MIN_SIZE, PPR_MAX_MAX_SIZE, default_settings.detection.min_size);

  if ((SDK_CMD_MAX_SIZE & settings_selection) == SDK_CMD_MAX_SIZE)
    fprintf(stderr, "   -max_size <int>                   :  maximum size (%d - %d) [%d]\n",
            PPR_MIN_MIN_SIZE, PPR_MAX_MAX_SIZE, default_settings.detection.max_size);

  if ((SDK_CMD_ADAPTIVE_MIN_SIZE & settings_selection) == SDK_CMD_ADAPTIVE_MIN_SIZE)
    fprintf(stderr, "   -adaptive_min <float>             :  adaptive minimum size (0.0 - 1.0) [%.2f]\n",
            default_settings.detection.adaptive_min_size);

  if ((SDK_CMD_ADAPTIVE_MAX_SIZE & settings_selection) == SDK_CMD_ADAPTIVE_MAX_SIZE)
    fprintf(stderr, "   -adaptive_max <float>             :  adaptive maximum size (0.0 - 1.0) [%.2f]\n",
            default_settings.detection.adaptive_max_size);

  if ((SDK_CMD_DETECTION_THRESHOLD & settings_selection) == SDK_CMD_DETECTION_THRESHOLD)
    fprintf(stderr, "   -detection_threshold <float>      :  detection threshold (-1.0 - 1.0) [%.2f]\n",
            default_settings.detection.threshold);

  if ((SDK_CMD_SEARCH_REGION & settings_selection) == SDK_CMD_SEARCH_REGION)
    fprintf(stderr, "   -region <int>,<int>,<int>,<int>   :  search region (x1,y1) -> (x2,y2) [search entire image]\n");

  if ((SDK_CMD_DETECT_LANDMARKS & settings_selection) == SDK_CMD_DETECT_LANDMARKS)
    fprintf(stderr, "   -landmarks (on|off)               :  detect landmarks [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_settings.landmarks.enable));

  if ((SDK_CMD_LANDMARK_RANGE & settings_selection) == SDK_CMD_LANDMARK_RANGE)
    fprintf(stderr, "   -range (frontal|ext|full|comp)    :  landmark range [%s]\n",
            sdk_cmd_line_get_landmark_range_string(default_settings.landmarks.landmark_range));
 
  if ((SDK_CMD_BEST_FACE_ONLY & settings_selection) == SDK_CMD_BEST_FACE_ONLY)
    fprintf(stderr, "   -best_face (on|off)               :  find only best face in each image [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_settings.detection.detect_best_face_only));

  if ((SDK_CMD_EXTRACT_THUMBNAILS & settings_selection) == SDK_CMD_EXTRACT_THUMBNAILS)
    fprintf(stderr, "   -extract_thumbnails (on|off)      :  extract thumbnails [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_settings.detection.extract_thumbnails | 
                                         default_settings.recognition.extract_thumbnails));

  if ((SDK_CMD_NUM_VISUAL_INSTANCES & settings_selection) == SDK_CMD_NUM_VISUAL_INSTANCES)
    fprintf(stderr, "   -visual_instances <int>           :  number of visual instances [%d]\n",
            default_settings.tracking.num_visual_instances);

  if ((SDK_CMD_TRACKING_CUTOFF & settings_selection) == SDK_CMD_TRACKING_CUTOFF)
    fprintf(stderr, "   -tracking_cutoff <int>            :  tracking cutoff (%d - %d) [%d]\n",
            PPR_MIN_TRACKING_CUTOFF, PPR_MAX_TRACKING_CUTOFF, default_settings.tracking.cutoff);

  if ((SDK_CMD_SHOT_BOUNDARY_DETECTION & settings_selection) == SDK_CMD_SHOT_BOUNDARY_DETECTION)
    fprintf(stderr, "   -shot_boundary_detection (on|off) :  shot boundary detection [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_settings.tracking.enable_shot_boundary_detection));

  if ((SDK_CMD_SHOT_BOUNDARY_THRESHOLD & settings_selection) == SDK_CMD_SHOT_BOUNDARY_THRESHOLD)
    fprintf(stderr, "   -shot_boundary_threshold <float>  :  shot boundary threshold [%.2f]\n",
            default_settings.tracking.shot_boundary_threshold);

  if ((SDK_CMD_RECOGNIZER_TYPE & settings_selection) == SDK_CMD_RECOGNIZER_TYPE)
    fprintf(stderr, "   -recognizer (frontal|mp|fast-mp)  :  recognizer type [%s]\n",
            sdk_cmd_line_get_recognizer_string(default_settings.recognition.recognizer));

  /*********************/
  /* options selection */
  /*********************/

  if ((SDK_CMD_OPT_BINARY_OUTPUT & options_selection) == SDK_CMD_OPT_BINARY_OUTPUT)
    fprintf(stderr, "   -binary_output (on|off)           :  write output to binary file [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.binary_output));

  if ((SDK_CMD_OPT_OVERLAYS & options_selection) == SDK_CMD_OPT_OVERLAYS)
    fprintf(stderr, "   -overlays (on|off)                :  write overlays [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.overlays));

  if ((SDK_CMD_OPT_WRITE_THUMBNAILS & options_selection) == SDK_CMD_OPT_WRITE_THUMBNAILS)
    fprintf(stderr, "   -write_thumbnails (on|off)        :  write thumbnail images [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.write_thumbnails));

  if ((SDK_CMD_OPT_MODELS_PATH & options_selection) == SDK_CMD_OPT_MODELS_PATH)
    fprintf(stderr, "   -models <string>                  :  path to models directory [%s]\n", MODELS_PATH);

  if ((SDK_CMD_OPT_VERBOSE & options_selection) == SDK_CMD_OPT_VERBOSE)
    fprintf(stderr, "   -verbose (on|off)                 :  print verbose output [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.verbose));

  if ((SDK_CMD_OPT_AGRESSIVENESS & options_selection) == SDK_CMD_OPT_AGRESSIVENESS)
    fprintf(stderr, "   -aggressiveness <int>             :  clustering aggressiveness (0 - %d) [%d]\n",
            PPR_MAX_CLUSTERING_AGGRESSIVENESS, default_options.aggressiveness);

  if ((SDK_CMD_OPT_MAX_DISPERSED & options_selection) == SDK_CMD_OPT_MAX_DISPERSED)
    fprintf(stderr, "   -max_dispersed_faces <int>        :  maximum number of dispersed faces to select from each track [%d]\n",
            default_options.max_dispersed);

  if ((SDK_CMD_OPT_NUM_REP_FACES & options_selection) == SDK_CMD_OPT_NUM_REP_FACES)
    fprintf(stderr, "   -n_representative_faces <int>     :  number of representative faces to select from each track [%d]\n",
            default_options.num_rep_faces);

  if ((SDK_CMD_OPT_KNOWN_GALLERY & options_selection) == SDK_CMD_OPT_KNOWN_GALLERY)
    fprintf(stderr, "   -known_gallery <string>           :  compare subjects to specified known gallery\n");
            
  if ((SDK_CMD_OPT_MATCH_THRESHOLD & options_selection) == SDK_CMD_OPT_MATCH_THRESHOLD)
    fprintf(stderr, "   -match_threshold <float>          :  threshold that must be exceeded to match a subject in the known gallery [%.3f]\n",
            default_options.match_threshold);

  if ((SDK_CMD_OPT_OVERLAY_FRAME_NUMS & options_selection) == SDK_CMD_OPT_OVERLAY_FRAME_NUMS)
    fprintf(stderr, "   -overlay_frame_numbers (on|off)   :  overlay frame numbers on output [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.overlay_frame_nums));

  if ((SDK_CMD_OPT_WRITE_VISUAL_INSTANCES & options_selection) == SDK_CMD_OPT_WRITE_VISUAL_INSTANCES)
    fprintf(stderr, "   -write_visual_instances (on|off)  :  write visual instances to disk [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.write_visual_instances));

  if ((SDK_CMD_OPT_SUBJECTS_UNIQUE & options_selection) == SDK_CMD_OPT_SUBJECTS_UNIQUE)
    fprintf(stderr, "   -subjects_unique (on|off)         :  indicates that subjects are unique within an image [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.subjects_unique_within_image));

  if ((SDK_CMD_OPT_MAX_LENGTH & options_selection) == SDK_CMD_OPT_MAX_LENGTH)
    fprintf(stderr, "   -max_length <int>                 :  maximum length of ranked lists [%d]\n",
            default_options.max_length);

  if ((SDK_CMD_OPT_RANK_THRESHOLD & options_selection) == SDK_CMD_OPT_RANK_THRESHOLD)
    fprintf(stderr, "   -rank_threshold <float>           :  threshold that must be met to add entry to ranked list [%.3f]\n",
            default_options.rank_threshold);

  if ((SDK_CMD_OPT_TRANS_MATCHING & options_selection) == SDK_CMD_OPT_TRANS_MATCHING)
    fprintf(stderr, "   -transitive_matching (on|off)     :  apply transitive matching to similarity matrix [%s]\n",
            sdk_cmd_line_utils_on_or_off(default_options.use_trans_matching));

  fprintf(stderr, "   -help                             :  display this usage message and exit\n");

  fprintf(stderr, "\n");
  fprintf(stderr, "example: %s %s\n", exe_name, example);
  fprintf(stderr, "\n");

  exit(EXIT_FAILURE);
}

/*****************************************************************************/

sdk_cmd_line_options_type sdk_cmd_line_get_default_options(void)
{
  sdk_cmd_line_options_type options;

  options.models_path= NULL;
  options.verbose = 0;
  options.binary_output = 0;
  options.overlays = 0;
  options.write_thumbnails = 0;
  options.aggressiveness = 6;
  options.max_dispersed = 50;
  options.num_rep_faces = 5;
  options.match_threshold = 0.5;
  options.known_gallery = NULL;
  options.overlay_frame_nums = 0;
  options.write_visual_instances = 0;
  options.subjects_unique_within_image = 0;
  options.max_length = 100;
  options.rank_threshold = 0.5;
  options.use_trans_matching = 0;
  return options;
}

/*****************************************************************************/

void sdk_cmd_line_free_options(sdk_cmd_line_options_type options)
{
  free(options.models_path);
  free(options.known_gallery);
}

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

static const char* sdk_cmd_line_utils_on_or_off(int val)
{
  if (val)
    return "on";
  else
    return "off";
}

/*****************************************************************************/

static const char* sdk_cmd_line_get_landmark_range_string(ppr_landmark_range_type landmark_range)
{
  switch(landmark_range)
  {
    case PPR_LANDMARK_RANGE_FRONTAL:
      return "frontal";
    case PPR_LANDMARK_RANGE_EXTENDED:
      return "ext";
    case PPR_LANDMARK_RANGE_FULL:
      return "full";
    case PPR_LANDMARK_RANGE_COMPREHENSIVE:
      return "comp";
    default:
      return "unknown";
  }
}

/*****************************************************************************/

static const char* sdk_cmd_line_get_recognizer_string(ppr_recognizer_type recognizer)
{
  switch(recognizer)
  {
    case PPR_RECOGNIZER_FRONTAL:
      return "frontal";
    case PPR_RECOGNIZER_MULTI_POSE:
      return "mp";
    case PPR_RECOGNIZER_FAST_MULTI_POSE:
      return "fast-mp";
    default:
      return "unknown";
  }
}

/*****************************************************************************/

static int sdk_cmd_line_get_on_or_off(char *str, int *on_or_off)
{
  if (strcmp(str, "on") == 0)
    *on_or_off = 1;
  else if (strcmp(str, "off") == 0)
    *on_or_off = 0;
  else
    return -1;

  return 0;
}

/*****************************************************************************/

static int sdk_cmd_line_get_landmark_range(char *landmark_range_str, ppr_landmark_range_type *landmark_range)
{
  if (strcmp(landmark_range_str, "frontal") == 0)
    *landmark_range = PPR_LANDMARK_RANGE_FRONTAL;
  else if (strcmp(landmark_range_str, "ext") == 0)
    *landmark_range = PPR_LANDMARK_RANGE_EXTENDED;
  else if (strcmp(landmark_range_str, "full") == 0)
    *landmark_range = PPR_LANDMARK_RANGE_FULL;
  else if (strcmp(landmark_range_str, "comp") == 0)
    *landmark_range = PPR_LANDMARK_RANGE_COMPREHENSIVE;
  else
    return -1;

  return 0;
}

/*****************************************************************************/

static int sdk_cmd_line_get_recognizer_type(char *recognizer_str, ppr_recognizer_type *recognizer)
{
  if (strcmp(recognizer_str, "frontal") == 0)
    *recognizer = PPR_RECOGNIZER_FRONTAL;
  else if (strcmp(recognizer_str, "mp") == 0)
    *recognizer = PPR_RECOGNIZER_MULTI_POSE;
  else if (strcmp(recognizer_str, "fast-mp") == 0)
    *recognizer = PPR_RECOGNIZER_FAST_MULTI_POSE;
  else
    return -1;

  return 0;
}

/*****************************************************************************/

int sdk_cmd_line_get_search_region_from_string(const char *search_region_str,
                                               int *x1,
                                               int *y1,
                                               int *x2,
                                               int *y2)
{
  char *num;

  if (search_region_str == NULL)
    return -1;

  num = strtok((char*)search_region_str, ",");
  if (num == NULL)
    return -1;
  *x1 = atoi(num);

  num = strtok(NULL, ",");
  if (num == NULL)
    return -1;
  *y1 = atoi(num);

  num = strtok(NULL, ",");
  if (num == NULL)
    return -1;
  *x2 = atoi(num);

  num = strtok(NULL, ",");
  if (num == NULL)
    return -1;
  *y2 = atoi(num);

  printf("%d %d %d %d\n", *x1, *y1, *x2, *y2);
  return 0;
}

/******************************************************************************/
