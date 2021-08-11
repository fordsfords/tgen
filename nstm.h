/* nstm.h - simple module for nanosecond-level timings.
# For documentation, see https://github.com/fordsfords/nstm
#
# This code and its documentation is Copyright 2002-2021 Steven Ford
# and licensed "public domain" style under Creative Commons "CC0":
#   http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/nstm
*/
#ifndef NSTM_H
#define NSTM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include <time.h>

#ifdef __MACH__
/* Mac */
#include <mach/clock.h>
#include <mach/mach.h>
#elif defined(_WIN32)
/* Windows */
#else
/* Linux */
#endif

#define NSTM_CLOCK_MONOTONIC 1
#define NSTM_CLOCK_MONOTONIC_RAW 2
#define NSTM_CLOCK_REALTIME 3
#define NSTM_CLOCK_BEST -1

struct nstm_s {
  uint64_t start_ns;
  uint64_t cur_ns;
#ifdef __MACH__
  /* Mac */
  clockid_t clockid;
#elif defined(_WIN32)
  /* Windows */
  LARGE_INTEGER start_ticks;
  LARGE_INTEGER frequency;
#else
  /* Linux */
  clockid_t clockid;
  struct timespec start_ts;
#endif
};
typedef struct nstm_s nstm_t;

nstm_t *nstm_create(int clockid);
void nstm_delete(nstm_t *nstm);
uint64_t nstm_get(nstm_t *nstm);

#ifdef __cplusplus
}
#endif

#endif  /* NSTM_H */
