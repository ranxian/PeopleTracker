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

#ifndef SDK_PROFILE_TIMERS_H
#define SDK_PROFILE_TIMERS_H

/*************/
/* Constants */
/*************/

#define SDK_PROFILE_TIMERS_MAX_N_TIMERS   24 /* Maximum number of timers in use at any time                        */
#define SDK_PROFILE_TIMERS_MAX_TIMER_NAME 31 /* Maximum number of characters in a timer name, including terminator */

/**************/
/* Data Types */
/**************/

typedef int sdk_profile_timer_id_t;

/***********************/
/* Function Prototypes */
/***********************/

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * This must be called before using the profile timers package.  It clears all
 * of the timers.
 */
void sdk_profile_timers_init(void);
 
/*
 * This function names the specified timer.
 */
void sdk_profile_timers_name_timer(
    sdk_profile_timer_id_t timer_id, /* I: ID of timer to assign name to */
    const char *name);               /* I: name to assign to timer       */

/*
 * This function sets the cumulative time for the specified timer to zero.
 */
void sdk_profile_timers_clear_timer(
    sdk_profile_timer_id_t timer_id); /* I: ID of timer to clear time of */

/*
 * This function sets the cumulative time for all timers to zero.
 */
void sdk_profile_timers_clear_all_timers(void);

/*
 * This function starts a timer by recording the timer ticks when the function
 * is called such that it can later be subtracted from the timer ticks when
 * sdk_profile_timers_stop_timer() is called.
 */
void sdk_profile_timers_start_timer(
    sdk_profile_timer_id_t timer_id); /* I: ID of timer to start */

/*
 * This function stops a timer by recording the timer ticks when the function is
 * called and subtracting from it the number of timer ticks when
 * sdk_profile_timers_start_timer() was called.  It then accumulates the number of
 * timer ticks for this timer.
 */
void sdk_profile_timers_stop_timer(
    sdk_profile_timer_id_t timer_id); /* I: ID of timer to stop */

/*
 * This function gets the total number of seconds for the specified timer by
 * using the appropriate conversion factor to convert cycles to seconds.
 */
double sdk_profile_timers_get_total_seconds(
    sdk_profile_timer_id_t timer_id); /* I: ID of timer to get total seconds of */

/*
 * This function prints the name and ID of each timer and the number of cycles
 * and number of seconds elapsed
 */
void sdk_profile_timers_print_timers(void);

#ifdef __cplusplus
}
#endif

#endif
