/* tgen.h - my standard definitions. */
/*
# This code and its documentation is Copyright 2002-2021 Steven Ford
# and licensed "public domain" style under Creative Commons "CC0":
#   http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/tgen
*/
#ifndef TGEN_H
#define TGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #define SLEEP_SEC(s_) Sleep((s_)*1000)
  #define SLEEP_MS Sleep
  #define STRTOK_PORT strtok_s
#else
  #define SLEEP_SEC sleep
  #define SLEEP_MS(ms_) usleep((ms_)*1000)
  #define STRTOK_PORT strtok_r
#endif

/* Error handling macros. These print to stderr and exit(1), or in
 * some cases abort. Most real programs would do something better.
 */

#ifdef _WIN32
  /* perror() is non-portable to Windows and needs a proper equiv. */
  #define PERR(s_) do { \
    char errno_ = errno; \
    char errstr_[1024]; \
    strerror_s(errno_, errstr_, sizeof(errstr_)); \
    fprintf(stderr, "ERROR (%s line %d): %s errno=%u ('%s')\n", \
      __FILE__, __LINE__, s_, errno, errstr_); \
    fflush(stderr); \
    exit(1); \
  } while (0)
#else
  #define PERR(s_) do { \
    char errno_ = errno; \
    char errnostr_[1024]; \
    strerror_r(errno_, errnostr_, sizeof(errnostr_)); \
    fprintf(stderr, "ERROR (%s line %d): %s errno=%u ('%s')\n", \
      __FILE__, __LINE__, s_, errno, errnostr_); \
    fflush(stderr); \
    exit(1); \
  } while (0)
#endif

#define EOK0(expr_) do { \
  int st_ = (expr_); \
  char errstr_[1024]; \
  if (st_ != 0) { \
    sprintf(errstr_, "'%s' is not 0", #expr_); \
    PERR(errstr_); \
  } \
} while (0)

#define ENULL(expr_) do { \
  if ((expr_) == NULL) { \
    char errstr_[1024]; \
    sprintf(errstr_, "'%s' is NULL", #expr_); \
    PERR(errstr_); \
  } \
} while (0)

#define ABRT(s_) do { \
  fprintf(stderr, "ERROR (%s line %d): %s\n", \
    __FILE__, __LINE__, s_); \
  fflush(stderr); \
  abort(); \
} while (0)

#define ASSRT(cond_) do { \
  if (! (cond_)) { \
    fprintf(stderr, "%s:%d, ERROR: '%s' not true\n", \
      __FILE__, __LINE__, #cond_); \
    abort(); \
  } \
} while (0)


/* See https://github.com/fordsfords/safe_atoi */
#define SAFE_ATOI(a_,r_) do { \
  unsigned long long fs_[9] = {  /* All '1's by variable size. */ \
    0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, 0xffffffffffffffff }; \
  errno = 0; \
  (r_) = fs_[sizeof(r_)]; \
  if ((r_) < 0) { /* Is result a signed value? */ \
    char *in_a_ = a_;  char *temp_ = NULL;  long long llresult_; \
    if (strlen(in_a_) > 2 && *in_a_ == '0' && *(in_a_ + 1) == 'x') { \
      llresult_ = strtoll(in_a_ + 2, &temp_, 16); \
    } else { \
      llresult_ = strtoll(in_a_, &temp_, 10); \
    } \
    if (errno != 0 || temp_ == in_a_ || temp_ == NULL || *temp_ != '\0') { \
      if (errno == 0) { \
        errno = EINVAL; \
      } \
      fprintf(stderr, "%s:%d, Error, invalid number for %s: '%s'\n", \
         __FILE__, __LINE__, #r_, in_a_); \
    } else { /* strtol thinks success; check for overflow. */ \
      (r_) = llresult_; /* "return" value of macro */ \
      if ((r_) != llresult_) { \
        fprintf(stderr, "%s:%d, %s over/under flow: '%s'\n", \
           __FILE__, __LINE__, #r_, in_a_); \
        errno = ERANGE; \
      } \
    } \
  } else { \
    char *in_a_ = a_;  char *temp_ = NULL;  unsigned long long llresult_; \
    if (strlen(in_a_) > 2 && *in_a_ == '0' && *(in_a_ + 1) == 'x') { \
      llresult_ = strtoull(in_a_ + 2, &temp_, 16); \
    } else { \
      llresult_ = strtoull(in_a_, &temp_, 10); \
    } \
    if (errno != 0 || temp_ == in_a_ || temp_ == NULL || *temp_ != '\0') { \
      if (errno == 0) { \
        errno = EINVAL; \
      } \
      fprintf(stderr, "%s:%d, Error, invalid number for %s: '%s'\n", \
         __FILE__, __LINE__, #r_, in_a_); \
    } else { /* strtol thinks success; check for overflow. */ \
      (r_) = llresult_; /* "return" value of macro */ \
      if ((r_) != llresult_) { \
        fprintf(stderr, "%s:%d, %s over/under flow: '%s'\n", \
           __FILE__, __LINE__, #r_, in_a_); \
        errno = ERANGE; \
      } \
    } \
  } \
} while (0)


#ifdef __cplusplus
}
#endif

#endif  /* TGEN_H */
