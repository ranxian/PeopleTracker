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
 * File: sdk_cmd_line_utils.c
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

#include "sdk_cmd_line_utils.h"
#include "sdk_utils.h"

/*****************************************************************************
 * Constants
 *****************************************************************************/

/* Error Codes */
#define SDK_CMD_LINE_UTILS_BAD_ARG -1 /* Bad argument given                */
#define SDK_CMD_LINE_UTILS_MISSING -2 /* Argument or parameter missing     */
#define SDK_CMD_LINE_UTILS_EXTRA   -3 /* Extra argument or parameter given */

/* Error Messages */
static const char* invalid_error_code = "not a valid error code";
static const char *gbl_error_strings[] =
{
  "success",
  "bad argument given",
  "argument or parameter missing",
  "extra argument or parameter given",
};

/*****************************************************************************
 * Macros
 *****************************************************************************/

/*
 * Recognizes a valid numeric constant or expression by its first character
 */
#define NUMERIC(s) (isdigit(*(s)) || *(s)=='.' || *(s)=='-' || *(s)=='+')

/*****************************************************************************
 * Data Types
 *****************************************************************************/

typedef enum
{
  SDK_CMD_LINE_UTILS_FLAG_UNASSIGNED, /* Flag type has not been assigned yet */
  SDK_CMD_LINE_UTILS_FLAG_SIMPLE,     /* A flag with no parameters           */
  SDK_CMD_LINE_UTILS_FLAG_PARAMETER   /* A flag with one or more parameters  */
} sdk_cmd_line_utils_flag_type;

typedef struct sdk_cmd_line_utils_form
{
  struct sdk_cmd_line_utils_form *next; /* Next form in linked list                            */
  char *format_str;                     /* Format string: "-size %d %F"                        */
  char *flag_str;                       /* Flag portion of format string : "-size"             */
  char *codes_str;                      /* Format codes from format string: "dF"               */
  sdk_cmd_line_utils_flag_type type;    /* Type of flag - simple or with parameters            */
  int n_params;                         /* Number of parameters for flag                       */
  int param_mask;                       /* Bit i = ok to stop before param i, i=0..            */
  int **params;                         /* Parameter pointer list for this flag                */
  int occurrences;                      /* Number of occurrences of this flag in argument list */
} sdk_cmd_line_utils_form_type;

/*****************************************************************************
 * Local Function Prototypes
 *****************************************************************************/

static sdk_cmd_line_utils_form_type* sdk_cmd_line_utils_create_form_list(
  va_list ap);

static void sdk_cmd_line_utils_parse_format_string(
  sdk_cmd_line_utils_form_type *form);

static int sdk_cmd_line_utils_parse_form_list(
  int argc,
  char *argv[],
  sdk_cmd_line_utils_form_type *form_list);

static void sdk_cmd_line_utils_init_form_list(
  sdk_cmd_line_utils_form_type *form_list);

static sdk_cmd_line_utils_form_type* sdk_cmd_line_utils_find_flag_in_form_list(
  char *flag_str,
  sdk_cmd_line_utils_form_type *form_list);

static int sdk_cmd_line_utils_process_arg(
  int argc,
  char *argv[],
  sdk_cmd_line_utils_form_type *form);

static int sdk_cmd_line_utils_get_n_args_before_next_flag(
  int argc,
  char *argv[],
  sdk_cmd_line_utils_form_type *form,
  int *skip);

static int sdk_cmd_line_utils_get_param_values(
  int narg,
  char **arg,
  sdk_cmd_line_utils_form_type *form);

static void sdk_cmd_line_utils_free_form(
  sdk_cmd_line_utils_form_type *form);

static void sdk_cmd_line_utils_free_form_list(
  sdk_cmd_line_utils_form_type *form_list);

/*****************************************************************************
 * Public Functions
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
  ...)
{
  int rv;                                  /* Function return values                      */
  va_list ap;                              /* Variable argument list                      */
  sdk_cmd_line_utils_form_type *form_list; /* List of forms compiled from specified flags */
  
  va_start(ap, argv);
  
  /* Create form list from input parameters */
  form_list = sdk_cmd_line_utils_create_form_list(ap);
  
  /* Parse arguments based on form list */
  rv = sdk_cmd_line_utils_parse_form_list(argc, argv, form_list);

  /* Free memory allocated for form list */
  sdk_cmd_line_utils_free_form_list(form_list);

  va_end(ap);
  
  return rv;
}

/*
 * Returns a string for the specified error.
 */
const char* sdk_cmd_line_utils_error_message(
  int error)
{
  int err = -1 * error;
  if ((err < 0) || (err >= (int)(sizeof(gbl_error_strings)/sizeof(gbl_error_strings[0]))))
    return invalid_error_code;

  return gbl_error_strings[err];
}


/*****************************************************************************
 * Local Functions
 *****************************************************************************/

/*
 * Creates form list from variable argument list.  Assumes va_start has
 * already been called and va_end will be called after returning.
 */
static sdk_cmd_line_utils_form_type* sdk_cmd_line_utils_create_form_list(
  va_list ap)
{
  char *s;                                /* Command line value           */
  int pi;                                 /* Parameter being processed    */
  sdk_cmd_line_utils_form_type *form;     /* Current form in linked list  */
  sdk_cmd_line_utils_form_type *prevform; /* Previous form in linked list */
  sdk_cmd_line_utils_form_type *rootform; /* Head of linked list of forms */
  
  /*
   * varargs syntax is:
   *     formatstr paramptr*
   * where there are as many paramptrs as %'s in the format string
   */

  rootform = NULL;
  prevform = NULL;

  while ((s = va_arg(ap, char*)) != 0) /* Get format string from argument list */
  {
    /* Allocate new form */
    form = (sdk_cmd_line_utils_form_type*)malloc(sizeof(sdk_cmd_line_utils_form_type));
    sdk_utils_check_allocation(form, "form", "sdk_cmd_line_utils_create_form");

    /* Initialize new form */
    form->next = NULL;
    form->format_str = s;
    form->flag_str = NULL;
    form->type = SDK_CMD_LINE_UTILS_FLAG_UNASSIGNED;
    form->params = NULL;
    form->param_mask = 0;

    /* Add to linked list, or put at head */
    if (prevform)
      prevform->next = form;
    else
      rootform = form;
    
    /* Parse format string to create flag string and code string */
    sdk_cmd_line_utils_parse_format_string(form);
    
    /* Allocate space for parameters */
    if (form->n_params == 0)
    {
      form->type = SDK_CMD_LINE_UTILS_FLAG_SIMPLE;
      form->params = (int**)malloc(sizeof(int*));
      sdk_utils_check_allocation(form, "form->params", "sdk_cmd_line_utils_create_form");

      /* Flag parameter is next in argument list */
      *form->params = va_arg(ap, int *);
    }
    else
    {
      form->type = SDK_CMD_LINE_UTILS_FLAG_PARAMETER;
      form->params = (int**)malloc(form->n_params * sizeof(int*));
      sdk_utils_check_allocation(form, "form->params", "sdk_cmd_line_utils_create_form");

      /* Multiple parameters are next in argument list */
      pi = 0;
      while ((s = va_arg(ap, char*)) != 0)
      {          
        /* Store parameter */
        form->params[pi++] = (int*)s;

        /* Stop if all parameters have been stored */
        if (pi >= form->n_params)
          break;
      }
    }
    prevform = form;
  }

  return rootform;
}

/*****************************************************************************/

/*
 * Parse the format string to create the flag string.
 *
 * e.g.: format="-size %d %F" => flag="-size", codes_str="dF", n_params=2
 *
 */
static void sdk_cmd_line_utils_parse_format_string(
  sdk_cmd_line_utils_form_type *form)
{
  char *s;  /* Pointer to current character in format string */
  char *c;  /* Pointer to current character in code          */
  size_t n; /* Size of string                                */
  int np;   /* Number of parameters                          */
  int done; /* Flag to exit loops                            */
  
  /* Check first character of format string to see if it is a flag */
  if (form->format_str[0] == '-')
  {
    /* Advance to end of flag string */
    n = 1;
    done = 0;
    while (!done)
    {
      if (!form->format_str[n] || (form->format_str[n] == ' ') || (form->format_str[n] == '%') || (form->format_str[n] == '['))
        done = 1;
      else
        n++;
    }
 
    /* Allocate flag string (+1 for null terminator) */
    form->flag_str = (char*)malloc((n + 1) * sizeof(char));
    sdk_utils_check_allocation(form->flag_str, "form->flag_str", "sdk_cmd_line_utils_parse_format_string");

    /* Copy flag string to form */
    strncpy(form->flag_str, form->format_str, n);

    /* Set null terminator */
    form->flag_str[n] = '\0';
  }
  else
  {
    /* No flag string - bad call */
    fprintf(stderr, "BADCALL\n");
    exit(EXIT_FAILURE);
  }
  
  s = &form->format_str[n];

  /* extract scanf codes from remainder of format string, put in form->code */
  n = (strlen(form->format_str) - n) / 2;  /* overestimate # of % codes */

  /* Allocate code string (+1 for null terminator) */
  form->codes_str = (char*)malloc((n + 1) * sizeof(char));
  sdk_utils_check_allocation(form->codes_str, "form->codes_str", "sdk_cmd_line_utils_parse_format_string");

  /* Point to beginning of code string */
  c = form->codes_str;
  np = 0;
  done = 0;
  while (!done)
  {
    /* Loop until first format specifier */
    while (*s == ' ' || *s == '[')
    {
      /* Mark as optional parameter if opening bracket */
      if (*s == '[')
        form->param_mask |= (0x00000001 << np);
      s++;
    }

    /* End of string or closing bracket - done */
    if (*s == '\0' || *s == ']')
      done = 1;

    if (!done)
    {
      /* Skip % */
      s++;

      /* Add format specifier to code string */
      *c = *s;

      /* Increment code pointer and number of parameters */
      c++;
      np++;

      /* Move to next character in string */
      s++;
    }
  }

  /* Mark last parameter */
  form->param_mask |= (0x00000001 << np);
  
  /* number of parameters to flag = number of '%'s in format string */
  form->n_params = np;

  /* Terminate code string */
  *c = '\0';
}

/*****************************************************************************/

/*
 * Parses the arguments in argv according to the form list.
 *
 * Returns index of first non-flag argument, negative on failure.
 */
static int sdk_cmd_line_utils_parse_form_list(
  int argc,
  char *argv[],
  sdk_cmd_line_utils_form_type *form_list)
{
  int i;                                /* Index for looping through arguments               */
  int di;                               /* Amount to increment to skip to next flag          */
  sdk_cmd_line_utils_form_type *p_form; /* Pointer to form corresponding to current argument */

  sdk_cmd_line_utils_init_form_list(form_list);

  i = 1;
  while (i < argc)
  {
    if (argv[i][0] == '-' && !NUMERIC(&argv[i][1]))
    {
      /* Flag argument - starts with a '-', but is not a negative number */

      /* Find form corresponding to this flag */
      p_form = sdk_cmd_line_utils_find_flag_in_form_list(argv[i], form_list);
      if (p_form == NULL)
	return SDK_CMD_LINE_UTILS_EXTRA;

      di = sdk_cmd_line_utils_process_arg(argc - i - 1, &argv[i+1], p_form);
      if (di < 0)
        return di;
      
      i += (di + 1);
    }
    else
    {
      /* First non-flag argument */
      return i;
    }
  }
  
  return argc;
}

/*****************************************************************************/

/*
 * Initializes form list before parsing arguments.
 *
 * Sets simple flags and repeat counts to 0.
 */
static void sdk_cmd_line_utils_init_form_list(
  sdk_cmd_line_utils_form_type *form_list)
{
  sdk_cmd_line_utils_form_type *curr_form; /* Pointer to current form in list */

  /* Loop through all forms in form list */
  curr_form = form_list;
  while (curr_form != NULL)
  {
    /* Initialize number of times encountered to 0 */
    curr_form->occurrences = 0;

    /* If flag, initialize to not set */
    if (curr_form->type == SDK_CMD_LINE_UTILS_FLAG_SIMPLE)
      **curr_form->params = 0;

    curr_form = curr_form->next;
  }
}

/*****************************************************************************/

/*
 * Finds the form with the flag matching arg in the form list.
 *
 * Returns pointer to form if found, otherwise returns 0.
 */
static sdk_cmd_line_utils_form_type* sdk_cmd_line_utils_find_flag_in_form_list(
  char *arg,
  sdk_cmd_line_utils_form_type *form_list)
{
  sdk_cmd_line_utils_form_type *curr_form;
  
  curr_form = form_list;
  while (curr_form != NULL)
  {
    /* Return if this form is the flag we are looking for */
    if (strcmp(curr_form->flag_str, arg) == 0)
      return curr_form;
    curr_form = curr_form->next;
  }

  return NULL;
}

/*****************************************************************************/

/*
 * Processes one form by parsing arguments in argv according to a single form.
 *
 * form was found by sdk_cmd_line_utils_find_flag_in_form_list,
 *     so we know av[-1] matches form->flag
 *
 * examine av[0]-av[ac-1] to determine number of parameters supplied
 *     if simple flag, set flag parameter and read no arguments
 *     else parameter flag, do arg-to-param assignments
 *
 * Returns number of arguments gobbled, or negative error code.
 */

static int sdk_cmd_line_utils_process_arg(
  int argc,
  char *argv[],
  sdk_cmd_line_utils_form_type *form)
{
  int narg;
  int skip;
  int used;
  int err;

  /* Increment number of times this form has been processed if flag argument */
  if (form->type == SDK_CMD_LINE_UTILS_FLAG_SIMPLE || form->type == SDK_CMD_LINE_UTILS_FLAG_PARAMETER)
    form->occurrences++;
  
  /* Get number of arguments before the next flag */
  narg = sdk_cmd_line_utils_get_n_args_before_next_flag(argc, argv, form, &skip);
  
  used = 0;
  switch (form->type)
  {
  case SDK_CMD_LINE_UTILS_FLAG_SIMPLE:
    /* Simple flag - turn it on */
    **form->params = 1;
    break;
  default:
    /* convert parameters */
    err = sdk_cmd_line_utils_get_param_values(narg, argv, form);
    if (err)
      return err;
    used = narg < form->n_params ? narg : form->n_params;
    break;
  }
  
  /* Have more arguments than we should ? */
  if (form->type == SDK_CMD_LINE_UTILS_FLAG_PARAMETER && used != narg)
    return SDK_CMD_LINE_UTILS_EXTRA;

  return skip;
}

/*****************************************************************************/

/*
 * Counts number of parameters in argv before the next flag.
 * NOTE: modifies argv
 * Returns number of valid arguments in new argv.
 * Sets *skip to number of argumentss to skip in old argv to get to next flag.
 */
static int sdk_cmd_line_utils_get_n_args_before_next_flag(
  int argc,
  char *argv[],
  sdk_cmd_line_utils_form_type *form,
  int *skip)
{
  char **au;
  char **av0; /* av when function was entered      */
  int i;
  int j;
  int die;    /* Flag to exit loop                 */

  av0 = argv;
  au = argv;

  die = 0;
  i = 0;

  while (i < form->n_params && i < argc && !die)
  {
    for (j = i + 1; !(form->param_mask >> j & 1); j++); /* go until we can stop */

    /* try to grab params i through j-1 */
    while (i < j && i < argc)
    {
      if (au != argv)
        *au = *argv;

      if (argv[0][0] == '-' && !NUMERIC(&argv[0][1]))
      {
        /* Encountered flag - break out of both loops */
	die = 1;
	break;
      }

      i++;
      au++;
      argv++;
    }
  }

  *skip = i;

  return (int)(au - av0);
}

/*****************************************************************************/

/*
 * Reads arguments into parameter array and does conversion.
 *
 * Returns 0 on success, otherwise returns error code.
 */
static int sdk_cmd_line_utils_get_param_values(
  int narg,
  char **arg,
  sdk_cmd_line_utils_form_type *form)
{
  static char str[]="%X"; /* Format string, X is replaced with value from code */
  char *s;                /* String allocated for S                            */
  int i;                  /* Index for looping through arguments               */
  int **p;                /* Pointer to parameter if form's parameter list     */
  
  if (form->n_params < narg)
    narg = form->n_params;

  if (!(form->param_mask >> narg & 1))
    return SDK_CMD_LINE_UTILS_MISSING;

  for (i = 0; i < narg; i++)
  {
    p = form->params;
    str[1] = form->codes_str[i];
    switch (str[1])
    {
    case 'S':
      /*
       * dynamically allocate memory for string
       * in case argv gets clobbered (rare)
       */

      s = (char*)malloc((strlen(arg[i]) + 1) * sizeof(char));
      sdk_utils_check_allocation(s, "s", "sdk_cmd_line_utils_get_param_values");
      strcpy(s, arg[i]);
      *(char **)*p = s;
      break;
    case 's':           /* scanf "%s" strips leading, trailing blanks */
      strcpy((char *) *p, arg[i]);
      break;
    case 'D':
      *(long *)*p = atol(arg[i]);
      break;
    case 'f':
      *(float *)*p = (float)atof(arg[i]);
      break;
    case 'F':
      *(double *)*p = atof(arg[i]);
      break;
    default:
      if (sscanf(arg[i], str, *p) != 1)
	return SDK_CMD_LINE_UTILS_BAD_ARG;
      break;
    }
    p++;
  }

  return 0;
}

/*****************************************************************************/

/* Frees memory allocated within a single form, but does not free the form itself. */
static void sdk_cmd_line_utils_free_form(
  sdk_cmd_line_utils_form_type *form)
{
  /* Don't free format string because it points to argument in argv */
  free(form->flag_str);
  free(form->codes_str);
  free(form->params);
}

/*****************************************************************************/

/* Frees memory allocated for each form in the list, including the forms themselves. */
static void sdk_cmd_line_utils_free_form_list(
  sdk_cmd_line_utils_form_type *form_list)
{
  sdk_cmd_line_utils_form_type *curr_form; /* Pointer to current form in list  */
  sdk_cmd_line_utils_form_type *prev_form; /* Pointer to previous form in list */

  /* Loop through all forms in form list */
  curr_form = form_list;
  while (curr_form != NULL)
  {
    sdk_cmd_line_utils_free_form(curr_form);

    prev_form = curr_form;
    curr_form = curr_form->next;
    free(prev_form);
  }
}
