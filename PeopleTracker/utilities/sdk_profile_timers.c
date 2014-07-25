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


/*******************************************************************************
 *
 * This provides functions to assist with profiling code.  This package makes
 * use of static variables and therefore it is NOT thread-safe.  The timer IDs
 * for all functions must be greater than or equal to 0 and less than
 * SDK_PROFILE_TIMERS_MAX_N_TIMERS.
 *
 ******************************************************************************/

#if !defined (WIN32) && (defined (_WIN32) || defined (__WIN32) || defined (_WIN32_) || defined (__WIN32__))
#define WIN32
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#else
#include <windows.h>
#endif

#include "sdk_profile_timers.h"

/*************/
/* Constants */
/*************/

#define USECS_PER_SEC        1e6
#define HUNDRED_NANO_IN_SEC  1e7

/********************/
/* Local Data Types */
/********************/

typedef struct sdk_profile_timer_struct
{
  char name[SDK_PROFILE_TIMERS_MAX_TIMER_NAME];
  
#ifndef WIN32
  struct timeval tv_at_start;
  struct timeval tv_at_stop;
  double cumulative_tv;
#else
  long long int time_at_start;
  long long int time_at_stop;
  long long int cumulative_time;
#endif
  
  int n_times_started;
  int n_times_stopped;
  int is_running;
} sdk_profile_timer_t;

/**************************/
/* Local Global Variables */
/**************************/

static sdk_profile_timer_t static_sdk_profile_timers[SDK_PROFILE_TIMERS_MAX_N_TIMERS];

/*****************************/
/* Local Function Prototypes */
/*****************************/

#ifndef WIN32
static double subtract_tv(struct timeval a, struct timeval b);
#endif

/*************/
/* Functions */
/*************/

/*
 * This must be called before using the sdk profile timers package.  It clears all
 * of the timers.
 */
void sdk_profile_timers_init(void)
{
  sdk_profile_timer_id_t tid;
  sdk_profile_timer_t *p_timer;

  for (tid = 0; tid < SDK_PROFILE_TIMERS_MAX_N_TIMERS; ++tid)
  {
    sdk_profile_timers_clear_timer(tid);
    p_timer = &static_sdk_profile_timers[tid];
    strncpy(p_timer->name, "", SDK_PROFILE_TIMERS_MAX_TIMER_NAME - 1); /* Sets all values to null terminators */
  }
}
 
/*
 * This function names the specified timer.
 */
void sdk_profile_timers_name_timer(
  sdk_profile_timer_id_t timer_id, /* I: ID of timer to assign name to */
  const char *name)                /* I: name to assign to timer       */
{
  sdk_profile_timer_t *p_timer = &static_sdk_profile_timers[timer_id];
  
  /* NULL terminator was set at initialization time */
  strncpy(p_timer->name, name, SDK_PROFILE_TIMERS_MAX_TIMER_NAME - 1);
}

/*
 * This function sets the cumulative time for the specified timer to zero.
 */
void sdk_profile_timers_clear_timer(
  sdk_profile_timer_id_t timer_id) /* I: ID of timer to clear time of */
{
  sdk_profile_timer_t *p_timer = &static_sdk_profile_timers[timer_id];
  
  p_timer->n_times_started = 0;
  p_timer->n_times_stopped = 0;
  p_timer->is_running = 0;

#ifndef WIN32
  p_timer->cumulative_tv = 0.0;
#else
  p_timer->cumulative_time = 0;
#endif
}

/*
 * This function sets the cumulative time for all timers to zero.
 */
void sdk_profile_timers_clear_all_timers(void)
{
  sdk_profile_timer_id_t tid;
  
  for (tid = 0; tid < SDK_PROFILE_TIMERS_MAX_N_TIMERS; ++tid)
    sdk_profile_timers_clear_timer(tid);
}

/*
 * This function starts a timer by recording the timer ticks when the function
 * is called such that it can later be subtracted from the timer ticks when
 * sdk_profile_timers_stop_timer() is called.
 */
void sdk_profile_timers_start_timer(
  sdk_profile_timer_id_t timer_id) /* I: ID of timer to start */
{
  sdk_profile_timer_t *p_timer = &static_sdk_profile_timers[timer_id];
  
#ifndef WIN32
  gettimeofday(&p_timer->tv_at_start, NULL);
#else
  FILETIME now;
  GetSystemTimeAsFileTime(&now);
  p_timer->time_at_start = now.dwHighDateTime;
  p_timer->time_at_start <<= 32;
  p_timer->time_at_start |= now.dwLowDateTime;
#endif

  p_timer->n_times_started++;
  p_timer->is_running = 1;

}

/*
 * This function stops a timer by recording the timer ticks when the function is
 * called and subtracting from it the number of timer ticks when
 * sdk_profile_timers_start_timer() was called.  It then accumulates the number of
 * timer ticks for this timer.
 */
void sdk_profile_timers_stop_timer(
  sdk_profile_timer_id_t timer_id) /* I: ID of timer to stop */
{
  sdk_profile_timer_t *p_timer = &static_sdk_profile_timers[timer_id];

#ifndef WIN32
  gettimeofday(&p_timer->tv_at_stop, NULL);
  p_timer->cumulative_tv += subtract_tv(p_timer->tv_at_stop, p_timer->tv_at_start);
#else
  FILETIME now;
  GetSystemTimeAsFileTime(&now);
  p_timer->time_at_stop = now.dwHighDateTime;
  p_timer->time_at_stop <<= 32;
  p_timer->time_at_stop |= now.dwLowDateTime;
  p_timer->cumulative_time += p_timer->time_at_stop - p_timer->time_at_start;
#endif

  p_timer->is_running = 0;
  p_timer->n_times_stopped++;
}

/*
 * This function gets the total number of seconds for the specified timer by
 * using the appropriate conversion factor to convert cycles to seconds.
 */
double sdk_profile_timers_get_total_seconds(
  sdk_profile_timer_id_t timer_id) /* I: ID of timer to get total seconds of */
{
  sdk_profile_timer_t *p_timer = &static_sdk_profile_timers[timer_id];

#ifndef WIN32
  struct timeval tv;
  double cumulative_tv = p_timer->cumulative_tv;
  if (p_timer->is_running)
  {
    gettimeofday(&tv, NULL);
    cumulative_tv += subtract_tv(tv, p_timer->tv_at_start);
  }
  return cumulative_tv;
#else
  long long int cumulative_time = p_timer->cumulative_time;
  FILETIME now;

  if (p_timer->is_running)
  {
    GetSystemTimeAsFileTime(&now);
    cumulative_time += ((now.dwLowDateTime + now.dwHighDateTime) - p_timer->time_at_start);
  }
  return (cumulative_time / (double)HUNDRED_NANO_IN_SEC);
#endif
}

/*
 * This function prints the name and ID of each timer and the number of cycles
 * and number of seconds elapsed
 */
void sdk_profile_timers_print_timers(void)
{
  sdk_profile_timer_t *p_timer; /* Pointer to current timer            */
  sdk_profile_timer_id_t tid;   /* Index for looping through timer IDs */
  double seconds;               /* Total seconds for current timer     */

  printf("\n");
  printf("------------------------------------------------------------------------------\n");
  printf("| ID |              NAME              |  TOTAL TIME  |  CALLS  |   AVERAGE   |\n");
  printf("------------------------------------------------------------------------------\n");
  for (tid = 0; tid < SDK_PROFILE_TIMERS_MAX_N_TIMERS; tid++)
  {
    p_timer = &static_sdk_profile_timers[tid];
    
    if ((p_timer->n_times_started > 0) || (p_timer->n_times_stopped > 0))
    {
      printf("| %2d | %30s | ", tid, p_timer->name);
      if (p_timer->n_times_started != p_timer->n_times_stopped)
        printf("ERROR - starts (%d) != stops (%d)\n",
               p_timer->n_times_started, p_timer->n_times_stopped);
      else
      {
        seconds = sdk_profile_timers_get_total_seconds(tid);
        printf("%12.6f | ", seconds);
        printf("%7d | ", p_timer->n_times_started);
        printf("%11.6f |\n", seconds / (double)p_timer->n_times_stopped);
      }
    }
  }
  printf("------------------------------------------------------------------------------\n");
}

/* Does a - b, assumes time a occurred after time b */
static double subtract_tv(struct timeval a, struct timeval b)
{
  long int sec_diff;
  long int usec_diff;

  sec_diff = a.tv_sec - b.tv_sec;
  usec_diff = a.tv_usec - b.tv_usec;

  /* Borrow from seconds */
  if (usec_diff < 0)
  {
    sec_diff--;
    usec_diff += (long int)USECS_PER_SEC;
  }

  return ((double)sec_diff + ((double)usec_diff / (double)USECS_PER_SEC));
}
