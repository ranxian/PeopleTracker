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
 * File: sdk_cmd_line_utils.h
 *
 * This file provides utility functions that are not part of the PittPatt SDK,
 * but can be useful when writing programs that use the SDK.
 *
 * These functions provide an easy way to parse command line arguments.
 */

#ifndef __SDK_CMD_LINE_UTILS_H
#define __SDK_CMD_LINE_UTILS_H

/* If C++ then we need to __extern "C". Compiler defines __cplusplus */
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Functions
 *****************************************************************************/

/*
 * Parses command-line arguments according to specified format.
 *
 * Returns index of first non-flag argument in argv[].  Returns negative value
 * for error.
 */
int sdk_cmd_line_utils_parse(
  int argc,
  char *argv[],
  ...);

/*
 * Returns a string for the specified error.
 */
const char* sdk_cmd_line_utils_error_message(
  int error);

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __SDK_CMD_LINE_UTILS_H */
