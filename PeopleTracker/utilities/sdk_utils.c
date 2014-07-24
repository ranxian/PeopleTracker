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



#if !defined (WIN32) && (defined (_WIN32) || defined (__WIN32) || defined (_WIN32_) || defined (__WIN32__))
#define WIN32
#endif

#if !defined (linux) && (defined (__linux) || defined (__linux__) || defined (__gnu_linux__))
#define linux
#endif

/* For get_num_cpus() */
#if defined (WIN32)
#include <windows.h>
#elif defined (linux)
#include <sys/sysinfo.h>
#elif defined (__MACH__)
#include <sys/sysctl.h>
#endif

#if defined (linux) && !defined (__USE_BSD)
/* required for S_IFDIR from sys/stat.h on older version of linux */
#define __USE_BSD
#endif

#include <sys/stat.h>
#if defined (WIN32)
#include <direct.h>
#else
#include <dirent.h>
#endif

#include <string.h>

#include "sdk_utils.h"
#include "pittpatt_raw_image_io.h"

/*******************************************************************************
 * Data Types
 ******************************************************************************/

typedef enum
{
  SDK_UTILS_DIRECTORY_ITEM_ALL,
  SDK_UTILS_DIRECTORY_ITEM_DIRECTORIES_ONLY,
  SDK_UTILS_DIRECTORY_ITEM_FILES_ONLY
} sdk_utils_directory_item_type;

/*******************************************************************************
 * Local Function Prototypes
 ******************************************************************************/

static char *sdk_utils_get_last_slash_ptr(const char *input);

static void sdk_utils_get_num_items_in_directory(
  const char *parent_directory,
  int *num_items,
  sdk_utils_directory_item_type item_type);

static void sdk_utils_get_item_list_from_directory(
  const char* parent_directory,
  sdk_utils_file_list_type *item_list,
  sdk_utils_directory_item_type item_type);

/*******************************************************************************
 * Miscellaneous Functions
 ******************************************************************************/

/* Returns the number of CPUs in the system */
int sdk_utils_get_num_cpus(void)
{
  int ncpu = 0;

#if defined (WIN32)
  SYSTEM_INFO sysInfo;
  GetSystemInfo (&sysInfo);
  ncpu = sysInfo.dwNumberOfProcessors;
#elif defined (linux) && !defined(ANDROID)
  ncpu = get_nprocs();
#elif defined (__MACH__)
  size_t size = sizeof(int);
  sysctlbyname("hw.ncpu", &ncpu, &size, NULL, (size_t)0);
#else
  ncpu = 1;
#endif

  if (ncpu < 2)
    ncpu = 1;

  return ncpu;
}

/*****************************************************************************/

/* Creates a directory */
void sdk_utils_make_directory(
  const char *directory_name)
{
  int err;

#ifdef WIN32
  /* Without permissions argument */
  err = _mkdir(directory_name);
#else
  /* With permissions argument - 0 prefix makes it octal */
  err = mkdir(directory_name, 0777);
#endif

  if (err < 0)
  {
    fprintf(stderr, "ERROR: could not create directory %s\n", directory_name);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
}

/******************************************************************************/

/* Write all visual instances of all completed tracks to specified directory */
void sdk_utils_write_visual_instances(const char *directory,
                                      const char *output_image_format,
                                      ppr_track_list_type track_list)
{
  int track_num;
  int instance_num;

  char instance_filename[FILENAME_MAX];

  for (track_num = 0; track_num < track_list.length; track_num++)
  {
    for (instance_num = 0; instance_num < track_list.tracks[track_num].visual_instances.length; instance_num++)
    {
      sprintf(instance_filename, "%s/id_%06d-instance_%02d_frame_%05d.%s",
              directory,
              track_list.tracks[track_num].track_summary.track_id,
              instance_num,
              track_list.tracks[track_num].visual_instances.instances[instance_num].frame_number,
              output_image_format);
      TRY_RAW_IMAGE(ppr_raw_image_io_write(instance_filename, track_list.tracks[track_num].visual_instances.instances[instance_num].image));
    }
  }
}

/**********************************************************************
 * Miscellaneous Recognition Functions
 **********************************************************************/

/* Writes face thumbnail to the specified file.  Writes gray image if template has no thumbnail */
void sdk_utils_write_face_thumbnail(
  const char *file_name,
  ppr_context_type context,
  ppr_face_type the_face)
{
  int i;
  
  ppr_error_type err;
  ppr_raw_image_type thumbnail;
  
  err = ppr_get_face_thumbnail(context, the_face, &thumbnail);
  if (err != PPR_SUCCESS)
  {
    if (err == PPR_NO_THUMBNAIL)
    {
      TRY_RAW_IMAGE(ppr_raw_image_create(&thumbnail, 48, 64, PPR_RAW_IMAGE_GRAY8));
      for (i = 0; i < 48 * 64; i++)
        thumbnail.data[i] = 128;
    }
    else
    {
      sdk_utils_check_ppr_error(err, "sdk_utils_write_face_thumbnail");
    }
  }
  
  TRY_RAW_IMAGE(ppr_raw_image_io_write(file_name, thumbnail));
  ppr_raw_image_free(thumbnail);
}

/******************************************************************************/

/* Writes thumbnails of all faces in a subject to a directory */
void sdk_utils_write_subject_thumbnails(
  const char *directory,
  ppr_context_type context,
  ppr_gallery_type gallery,
  int subject_id)
{
  int i;

  ppr_face_ref_type face_ref;
  ppr_id_list_type face_id_list;
  char thumbnail_filename[FILENAME_MAX];

  sdk_utils_make_directory(directory);

  TRY(ppr_get_face_id_list_for_subject(context, gallery, subject_id, &face_id_list));
  for (i = 0; i < face_id_list.length; i++)
  {
    sprintf(thumbnail_filename, "%s/thumbnail_%04d.jpg", directory, face_id_list.ids[i]);
    TRY(ppr_get_face_reference(context, gallery, face_id_list.ids[i], &face_ref));
    sdk_utils_write_face_thumbnail(thumbnail_filename,
                                   context,
                                   *face_ref);
    ppr_release_face_reference(face_ref);
  }
  ppr_free_id_list(face_id_list);
}

/******************************************************************************/

/* Writes a directory structure of thumbnails where each directory is a single cluster */
void sdk_utils_write_cluster_thumbnails(
  const char *directory,
  ppr_context_type context,
  ppr_gallery_type gallery,
  ppr_cluster_list_type cluster_list)
{
  int i, j, k;
  int count;
  ppr_id_list_type face_id_list;
  ppr_face_ref_type face_ref;
  char curr_cluster_directory[FILENAME_MAX];
  char thumbnail_filename[FILENAME_MAX];
  char full_thumbnail_filename[FILENAME_MAX];
  char result_filename[FILENAME_MAX];
  FILE *filepointer;

  sprintf(result_filename, "%s/clustering_results.html", directory);
  filepointer = fopen(result_filename, "w");
  sdk_utils_check_file_open(filepointer, result_filename, __FUNCTION__);
  fprintf(filepointer, "<html>\n<body>\n");


  for (i = 0; i < cluster_list.length; i++)
  {
    sprintf(curr_cluster_directory, "%s/cluster_%02d", directory, i);
    sdk_utils_make_directory(curr_cluster_directory);
    sprintf(curr_cluster_directory, "cluster_%02d", i);
    count = 0;
    fprintf(filepointer, "<hr><h3>Cluster %d</h3>\n", i);
    
    for (j = 0; j < cluster_list.clusters[i].length; j++)
    {
      TRY(ppr_get_face_id_list_for_subject(context, gallery, cluster_list.clusters[i].ids[j], &face_id_list));
      for (k = 0; k < face_id_list.length; k++)
      {
        TRY(ppr_get_face_reference(context, gallery, face_id_list.ids[k], &face_ref));
        sprintf(thumbnail_filename, "%s/thumbnail_%02d_id_%04d.jpg", curr_cluster_directory, count++, face_id_list.ids[k]);
        sprintf(full_thumbnail_filename, "%s/%s", directory, thumbnail_filename);
        sdk_utils_write_face_thumbnail(full_thumbnail_filename, context, *face_ref);
        ppr_release_face_reference(face_ref);

        fprintf(filepointer, "<img src=\"%s\">\n", thumbnail_filename);
      }
      ppr_free_id_list(face_id_list);
    }
    fprintf(filepointer, "<br>\n");
  }

  fprintf(filepointer, "</body>\n</html>\n");
  fclose(filepointer);
}

/*****************************************************************************
 * File List Functions
 *****************************************************************************/

/* Returns 1 if the path specifies a directory, 0 otherwise */
int sdk_utils_is_directory(
  const char *path)
{
#ifdef WIN32

  struct _stat stat_buf;
  _stat(path, &stat_buf);

#else

  struct stat stat_buf;
  stat(path, &stat_buf);

#endif

  if (stat_buf.st_mode & S_IFDIR)
    return 1;
  
  return 0;
}

/******************************************************************************/

/* Gets a list of files from a directory (does not include directories) */
void sdk_utils_get_file_list_from_directory(
  const char* parent_directory,
  sdk_utils_file_list_type *file_list)
{
  sdk_utils_get_item_list_from_directory(parent_directory, file_list, SDK_UTILS_DIRECTORY_ITEM_FILES_ONLY);
}

/******************************************************************************/

/* Gets a list of directories from a directory (does not include files) */
void sdk_utils_get_directory_list_from_directory(
  const char* parent_directory,
  sdk_utils_file_list_type *directory_list)
{
  sdk_utils_get_item_list_from_directory(parent_directory, directory_list, SDK_UTILS_DIRECTORY_ITEM_DIRECTORIES_ONLY);
}

/******************************************************************************/

/* Loads a file list from a file to an array of strings */
void sdk_utils_read_file_list(
  const char* filename,
  sdk_utils_file_list_type *file_list)
{
  char c = '\0';
  int i, row, j;

  int rowcount = 0;
  int size = 0;
  int prev;

  FILE *filepointer;
  char *file_array;
  
  /* Open filelist, make sure it exists. */
  filepointer = fopen(filename, "rt");
  sdk_utils_check_file_open(filepointer, filename, __FUNCTION__);

  /* Find length of file */
  while (c != EOF)
  {
    c = (char)getc(filepointer);
    size++;
  }  

  /* Read file */
  file_array = (char*)malloc(size);
  sdk_utils_check_allocation(file_array, "file_array", __FUNCTION__);

  rewind(filepointer);  
  for (i = 0; i < size; i++)
    file_array[i] = (char)getc(filepointer);

  if (file_array[size - 1] != EOF)
  {
    fprintf(stderr, "ERROR: EOF not found at end of file in file_list_open()\n");
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

  /* Change EOF to \n to ensure it acts as an ending seperator */
  file_array[size - 1] = '\n';  
  
  prev = 0;
  i = 0;
  
  /* Skip initial blank lines */
  while (file_array[i] == '\n' && i < size)
    i++;
  
  /* Count valid lines */
  for ( ; i < size; i++)      
  {
    if (file_array[i] == '\n')
    {
      /* Ignore consecutive end-of-lines (blank lines) */
      if (i - prev > 1)
        rowcount++;
      prev = i;
    }
  }
  
  file_list->filenames = (char**)malloc(rowcount * sizeof(char*));
  sdk_utils_check_allocation(file_list->filenames, "file_list->filenames", __FUNCTION__);

  file_list->length = rowcount;
  prev = 0;
  i = 0;
  
  /* Break filelist into array of strings. */
  for (row = 0; row < file_list->length; row++)
  {
    i = prev;
    c = file_array[i];
    
    /* Find next newline */
    while (c != '\n')
      c = file_array[++i]; 

    /* Copy current filename */
    file_list->filenames[row] = (char*)malloc(i - prev + 1);  
    sdk_utils_check_allocation(file_list->filenames[row], "file_list->filenames[row]", __FUNCTION__);

    for (j = 0; j < i - prev; j++)
      file_list->filenames[row][j] = file_array[prev + j];
    
    /* Terminate copied string */
    file_list->filenames[row][i - prev] = '\0';         
    
    /* Prepare for next iteration */
    prev = i;
    
    /* Skip blank lines */
    if (row < file_list->length - 1)
    {
      while (file_array[prev] == '\n')
        prev++;
    }
  }
  
  free(file_array);
  fclose(filepointer);
}

/******************************************************************************/

/* Combines multiple files / file lists into a single list - a .txt extension is assumed to be a list */
void sdk_utils_create_combined_file_list(
  char *filenames[],
  int n_filenames,
  sdk_utils_file_list_type *file_list)
{
  int i;
  int j;
  int n_total_files;
  sdk_utils_file_list_type tmp_file_list;
  char extension[FILENAME_MAX];

  /* Count total number of files */
  n_total_files = 0;
  for (i = 0; i < n_filenames; ++i)
  {
    sdk_utils_get_file_extension(filenames[i], extension);
    if (strcmp(extension, "txt") == 0)
    {
      sdk_utils_read_file_list(filenames[i], &tmp_file_list);
      n_total_files += tmp_file_list.length;
      sdk_utils_free_file_list(tmp_file_list);
    }
    else
    {
      n_total_files++;
    }
  }

  /* Allocate a file list for the total number of files */
  file_list->filenames = (char**)malloc(n_total_files * sizeof(char*));
  sdk_utils_check_allocation(file_list->filenames, "file_list->filenames", "sdk_utils_create_combined_file_list");

  file_list->length = 0;

  /* Put file names into file list */
  for (i = 0 ; i < n_filenames; i++)
  {
    sdk_utils_get_file_extension(filenames[i], extension);
    if (strcmp(extension, "txt") == 0)
    {
      sdk_utils_read_file_list(filenames[i], &tmp_file_list);
      for (j = 0; j < tmp_file_list.length; j++)
      {
        file_list->filenames[file_list->length] = (char*)malloc(strlen(tmp_file_list.filenames[j]) + 1);
        sdk_utils_check_allocation(file_list->filenames, "file_list->filenames[file_list->length]", "sdk_utils_create_combined_file_list");
        strcpy(file_list->filenames[file_list->length], tmp_file_list.filenames[j]);
        file_list->length++;
      }

      sdk_utils_free_file_list(tmp_file_list);
    }
    else
    {
      file_list->filenames[file_list->length] = (char*)malloc(strlen(filenames[i]) + 1);
      strcpy(file_list->filenames[file_list->length], filenames[i]);
      file_list->length++;
    }
  }
}

/*****************************************************************************/

/* Frees memory used to hold file list components */
void sdk_utils_free_file_list(
  sdk_utils_file_list_type file_list)
{
  int i;

  for (i = 0; i < file_list.length; i++)
    free(file_list.filenames[i]);

  free(file_list.filenames);
}

/*****************************************************************************
 * Filename Manipulation Functions
 *****************************************************************************/

/* Get directory from a string */
void sdk_utils_get_file_directory(const char *input,
                                  char *directory)
{
  const char *last_slash_ptr;
  int directory_len;

  last_slash_ptr = sdk_utils_get_last_slash_ptr(input);

  if (last_slash_ptr == NULL)
  {
    directory[0] = '\0';
  }
  else
  {
    directory_len = (int)(last_slash_ptr - input);
    strncpy(directory, input, directory_len);
    directory[directory_len] = '\0';
  }
}

/*****************************************************************************/

/* Get title from a string */
void sdk_utils_get_file_title(const char *input,
                              char *title)
{
  const char *last_dot_ptr;
  const char *last_slash_ptr;
  const char *begin_ptr;
  const char *end_ptr;
  int title_len;

  last_dot_ptr = strrchr(input, '.');
  
  last_slash_ptr = sdk_utils_get_last_slash_ptr(input);

  if (last_dot_ptr == NULL || last_dot_ptr < last_slash_ptr)
    end_ptr = input + strlen(input) - 1;
  else
    end_ptr = last_dot_ptr - 1;

  if (last_slash_ptr == NULL)
    begin_ptr = input;
  else
    begin_ptr = last_slash_ptr + 1;

  title_len = (int)(end_ptr - begin_ptr + 1);
  strncpy(title, begin_ptr, title_len);
  title[title_len] = '\0';
}

/*****************************************************************************/

/* Get extension from a string */
void sdk_utils_get_file_extension(const char *input,
                                  char *extension)
{
  const char *last_dot_ptr;
  const char *last_slash_ptr;

  last_dot_ptr = strrchr(input, '.');
  
  last_slash_ptr = sdk_utils_get_last_slash_ptr(input);

  if (last_dot_ptr == NULL || last_dot_ptr < last_slash_ptr)
    extension[0] = '\0';
  else
    strcpy(extension, last_dot_ptr + 1);
}

/*****************************************************************************/

/* Get basename from a string */
void sdk_utils_get_file_basename(const char *input,
                                 char *basename)
{
  const char *last_dot_ptr;
  int basename_len;

  last_dot_ptr = strrchr(input, '.');

  if (last_dot_ptr == NULL)
  {
    strcpy(basename, input);
  }
  else
  {
    basename_len = (int)(last_dot_ptr - input);
    strncpy(basename, input, basename_len);
    basename[basename_len] = '\0';
  }
}

/*****************************************************************************/

/* Get filename from a string */
void sdk_utils_get_file_filename(const char *input,
                                 char *filename)
{
  const char *last_slash_ptr;

  last_slash_ptr = sdk_utils_get_last_slash_ptr(input);

  if (last_slash_ptr == NULL)
    strcpy(filename, input);
  else
    strcpy(filename, last_slash_ptr + 1);
}

/*****************************************************************************
 * Error Checking Functions
 *****************************************************************************/

/* Prints message if err is anything other than PPR_SUCCESS, exits if fatal */ 
void sdk_utils_check_ppr_error(
  ppr_error_type err,
  const char *function_name)
{
  if (err != PPR_SUCCESS)
  {
    fprintf(stderr, "ERROR returned from %s(): %s\n", function_name, ppr_error_message(err));
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
}

/*****************************************************************************/

/* Prints message and exits if err is not PPR_RAW_IMAGE_SUCCESS */ 
void sdk_utils_check_ppr_raw_image_error(
  ppr_raw_image_error_type err,
  const char *function_name)
{
  if (err != PPR_RAW_IMAGE_SUCCESS)
  {
    fprintf(stderr, "ERROR returned from %s(): %s\n", function_name, ppr_raw_image_error_message(err));
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
}

/*****************************************************************************/

/* Prints message and exits if err is not PPR_VIDEO_IO_SUCCESS */ 
void sdk_utils_check_ppr_video_io_error(
  ppr_video_io_error_type err,
  const char *function_name)
{
  if (err != PPR_VIDEO_IO_SUCCESS)
  {
    fprintf(stderr, "ERROR returned from %s(): %s\n", function_name, ppr_video_io_error_message(err));
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
}

/*****************************************************************************/

/* Prints message and exits if input is NULL */ 
void sdk_utils_check_allocation(
  void *input,
  const char *input_name,
  const char *calling_function_name)
{
  if (input == NULL)
  {
    fprintf(stderr, "ERROR: could not allocate %s in %s()\n", input_name, calling_function_name);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
}

/*****************************************************************************/

/* Prints message and exits if file descriptor is NULL */ 
void sdk_utils_check_file_open(
  FILE *fid,
  const char *file_name,
  const char *calling_function_name)
{
  if (fid == NULL)
  {
    fprintf(stderr, "ERROR: could not open %s in %s()\n", file_name, calling_function_name);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
}

/*******************************************************************************
 * Local Functions
 ******************************************************************************/

/* Get the last slash or backslash in a string */
static char *sdk_utils_get_last_slash_ptr(const char *input)
{
  char *last_forwardslash_ptr;
  char *last_backslash_ptr;
  char *last_slash_ptr;

  last_forwardslash_ptr = strrchr(input, '/');
  last_backslash_ptr = strrchr(input, '\\');

  if (last_forwardslash_ptr == NULL)
  {
    last_slash_ptr = last_backslash_ptr;
  }
  else if (last_backslash_ptr == NULL)
  {
    last_slash_ptr = last_forwardslash_ptr;
  }
  else
  {
    if (last_forwardslash_ptr > last_backslash_ptr)
      last_slash_ptr = last_forwardslash_ptr;
    else
      last_slash_ptr = last_backslash_ptr;
  }

  return last_slash_ptr;
}

/*****************************************************************************/

/* Gets the number of items (directories and/or files) in a directory, not including ones that start with a '.' */
static void sdk_utils_get_num_items_in_directory(
  const char *parent_directory,
  int *num_items,
  sdk_utils_directory_item_type item_type)
{
#ifdef WIN32

  HANDLE h;
  WIN32_FIND_DATAA data;
  char filename[FILENAME_MAX];
  int is_directory;
  BOOL items_remain;

  sprintf(filename, "%s\\*", parent_directory);
  h = FindFirstFileA(filename, &data);
  if (h == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "ERROR: could not read directory %s in %s()\n", parent_directory, __FUNCTION__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }
  
  *num_items = 0;
  items_remain = TRUE;
  while (items_remain)
  {
    if (data.cFileName[0] != '.' && data.cFileName[0] != '\0')
    {
      sprintf(filename, "%s\\%s", parent_directory, data.cFileName);
      is_directory = sdk_utils_is_directory(filename);
      if ((item_type == SDK_UTILS_DIRECTORY_ITEM_ALL) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_DIRECTORIES_ONLY && is_directory) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_FILES_ONLY && !is_directory))
      {
        (*num_items)++;
      }
    }

    items_remain = FindNextFileA(h, &data);
  }

  if (GetLastError() != ERROR_NO_MORE_FILES)
  {
    fprintf(stderr, "ERROR: could not read directory %s in %s()\n", parent_directory, __FUNCTION__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

#else

  DIR *dir_stream;
  struct dirent *entry;
  char filename[FILENAME_MAX];
  int is_directory;

  dir_stream = opendir(parent_directory);
  if (dir_stream == NULL)
  {
    fprintf(stderr, "ERROR: could not read directory %s in %s()\n", parent_directory, __FUNCTION__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

  *num_items = 0;
  while((entry = readdir(dir_stream)) != NULL)
  {
    if (entry->d_name[0] != '.' && entry->d_name[0] != '\0')
    {
      sprintf(filename, "%s/%s", parent_directory, entry->d_name);
      is_directory = sdk_utils_is_directory(filename);
      if ((item_type == SDK_UTILS_DIRECTORY_ITEM_ALL) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_DIRECTORIES_ONLY && is_directory) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_FILES_ONLY && !is_directory))
      {
        (*num_items)++;
      }
    }
  }

  closedir(dir_stream);

#endif
}

/******************************************************************************/

/* Gets a list of items (directories and/or files) in a directory */
static void sdk_utils_get_item_list_from_directory(
  const char* parent_directory,
  sdk_utils_file_list_type *item_list,
  sdk_utils_directory_item_type item_type)
{
#ifdef WIN32

  int i;
  HANDLE h;
  WIN32_FIND_DATAA data;
  size_t len;
  char filename[FILENAME_MAX];
  int is_directory;
  BOOL items_remain;

  sdk_utils_get_num_items_in_directory(parent_directory, &item_list->length, item_type);
  if (item_list->length == 0)
    return;

  sprintf(filename, "%s\\*", parent_directory);
  h = FindFirstFileA(filename, &data);
  if (h == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "ERROR: could not read directory %s in %s()\n", parent_directory, __FUNCTION__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

  item_list->filenames = (char**)malloc(item_list->length * sizeof(char*));
  sdk_utils_check_allocation(item_list->filenames, "item_list->filenames", __FUNCTION__);

  i = 0;
  items_remain = TRUE;
  while (items_remain)
  {
    if (data.cFileName[0] != '.' && data.cFileName[0] != '\0')
    {
      sprintf(filename, "%s/%s", parent_directory, data.cFileName);
      is_directory = sdk_utils_is_directory(filename);
      if ((item_type == SDK_UTILS_DIRECTORY_ITEM_ALL) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_DIRECTORIES_ONLY && is_directory) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_FILES_ONLY && !is_directory))
      {
        len = strlen(data.cFileName);
        item_list->filenames[i] = (char*)malloc((len + 1) * sizeof(char));
        sdk_utils_check_allocation(item_list->filenames[i], "item_list->filenames[i]", __FUNCTION__);
        strcpy(item_list->filenames[i], data.cFileName);
        i++;
      }
    }
    items_remain = FindNextFileA(h, &data);
  }

  if (GetLastError() != ERROR_NO_MORE_FILES)
  {
    fprintf(stderr, "ERROR: could not read directory %s in %s()\n", parent_directory, __FUNCTION__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

#else

  int i;
  
  DIR *dir_stream;
  struct dirent *entry;
  int len;
  char filename[FILENAME_MAX];
  int is_directory;

  sdk_utils_get_num_items_in_directory(parent_directory, &item_list->length, item_type);
  if (item_list->length == 0)
    return;
  
  dir_stream = opendir(parent_directory);
  if (dir_stream == NULL)
  {
    fprintf(stderr, "ERROR: could not read directory %s in %s()\n", parent_directory, __FUNCTION__);
    ppr_finalize_sdk();
    exit(EXIT_FAILURE);
  }

  item_list->filenames = (char**)malloc(item_list->length * sizeof(char*));
  sdk_utils_check_allocation(item_list->filenames, "item_list->filenames", __FUNCTION__);

  i = 0;
  while((entry = readdir(dir_stream)) != NULL)
  {
    if (entry->d_name[0] != '.' && entry->d_name[0] != '\0')
    {
      sprintf(filename, "%s/%s", parent_directory, entry->d_name);
      is_directory = sdk_utils_is_directory(filename);
      if ((item_type == SDK_UTILS_DIRECTORY_ITEM_ALL) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_DIRECTORIES_ONLY && is_directory) ||
          (item_type == SDK_UTILS_DIRECTORY_ITEM_FILES_ONLY && !is_directory))
      {
        len = strlen(entry->d_name);
        item_list->filenames[i] = (char*)malloc((len + 1) * sizeof(char));
        sdk_utils_check_allocation(item_list->filenames[i], "item_list->filenames[i]", __FUNCTION__);
        strcpy(item_list->filenames[i], entry->d_name);
        i++;
      }
    }
  }

  closedir(dir_stream);

#endif
}

/******************************************************************************/
