/*** header counterpart to auto-gen'd leapseconds.def */
#if !defined INCLUDED_leap_seconds_h_
#define INCLUDED_leap_seconds_h_

#include <stdint.h>

/**
 * Number of known leap corrections. */
extern const size_t nleaps;

/**
 * Absolute difference in seconds, TAI - UTC, at transition points */
extern const int32_t leaps_corr[];

/**
 * YMD representation of transitions. */
extern const uint32_t leaps_ymd[];

/**
 * YMCW representation of transitions. */
extern const uint32_t leaps_ymcw[];

/**
 * daisy representation of transitions. */
extern const uint32_t leaps_d[];

/**
 * sexy representation of transitions. */
extern const int32_t leaps_s[];

/**
 * HMS representation of transitions. */
extern const uint32_t leaps_hms[];

#endif	/* INCLUDED_leap_seconds_h_ */
