/* tgen.c - Traffic generator framework.
# For documentation, see https://github.com/fordsfords/tgen
#
# This code and its documentation is Copyright 2002-2021 Steven Ford
# and licensed "public domain" style under Creative Commons "CC0":
#   http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/tgen
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>
#ifdef _WIN32
  #include <winsock2.h>
  #define SLEEP(s) Sleep((s)*1000)
#else
  #include <stdlib.h>
  #include <unistd.h>
  #define SLEEP(s) sleep(s)
#endif

#include "nstm.h"
#include "tgen.h"

#define NS_PER_SEC UINT64_C(1000000000)

/* Globals. */
nstm_t *nstm;

/* Command-line options and their defaults. */
int o_loops = 1;


char usage_str[] = "Usage: tgen [-h] [-l loops]";

void usage(char *msg) {
  if (msg) fprintf(stderr, "%s\n", msg);
  fprintf(stderr, "%s\n", usage_str);
  exit(1);
}

void help() {
  fprintf(stderr, "%s\n", usage_str);
  fprintf(stderr, "where:\n"
                  "  -h : print help\n"
                  "  -l loops : repeat the test set loops times [1]\n");
  exit(0);
}


int send_at_rate(uint64_t sends_per_sec, uint64_t send_duration)
{
  uint64_t cur_ns = nstm_get(nstm);
  uint64_t start_ns = cur_ns;
  uint64_t end_ns = start_ns + send_duration;
  uint64_t num_sends = 0;

  do {
    uint64_t ns_so_far = cur_ns - start_ns;
    /* The +1 is because we want to send, pause, send, pause, etc. */
    uint64_t should_have_sent = (ns_so_far * sends_per_sec) / NS_PER_SEC + 1;

    while (num_sends < should_have_sent) {
      /* insert "send" code here */
      num_sends++;
    }

    cur_ns = nstm_get(nstm);
  } while (cur_ns < end_ns);
  printf("??? num_sends=%"PRIu64"\n", num_sends);

  return 0;
}  /* send_at_rate */


int send_at_rate_histo(uint64_t sends_per_sec, uint64_t send_duration)
{
  uint64_t cur_ns = nstm_get(nstm);
  uint64_t prev_ns;
  uint64_t start_ns = cur_ns;
  uint64_t end_ns = start_ns + send_duration;
  uint64_t num_sends = 0;

  #define HISTO_SZ 10000
  int histo[HISTO_SZ];
  int i;
  int max_sends = 0;
  uint64_t max_diff = 0;
  uint64_t min_diff = -1;
  uint64_t diff;
  for (i=0; i<HISTO_SZ; i++) histo[i] = 0;

  do {
    uint64_t ns_so_far = cur_ns - start_ns;
    /* The +1 is because we want to send, pause, send, pause, etc. */
    uint64_t should_have_sent = (ns_so_far * sends_per_sec) / NS_PER_SEC + 1;

    if ((should_have_sent - num_sends) > max_sends) {
      max_sends = should_have_sent - num_sends;
    }
    while (num_sends < should_have_sent) {
      /* insert "send" code here */
      num_sends++;
    }

    prev_ns = cur_ns;
    cur_ns = nstm_get(nstm);

    diff = cur_ns - prev_ns;
    if (diff > max_diff) max_diff = diff;
    if (diff < min_diff) min_diff = diff;
    diff /= 10;  /* scale */
    if (diff >= HISTO_SZ) diff = HISTO_SZ - 1;
    histo[diff]++;
  } while (cur_ns < end_ns);

  for (i=0; i<10000; i++) {
    if (histo[i] > 0) {
      printf("histo[%d]=%d\n", i, histo[i]);
    }
  }
  printf("max_sends=%d, max_diff=%"PRIu64", min_diff=%"PRIu64"\n",
      max_sends, max_diff, min_diff);
  printf("??? num_sends=%"PRIu64"\n", num_sends);

  return 0;
}  /* send_at_rate_histo */


int t1(uint64_t a, uint64_t b)
{
  printf("??? t1, a=%"PRIu64", b=%"PRIu64"\n", a, b);
  return send_at_rate(a, b);;
}  /* t1 */


int t2(uint64_t a, uint64_t b)
{
  printf("??? t2, a=%"PRIu64", b=%"PRIu64"\n", a, b);
  return send_at_rate_histo(a, b);;
}  /* t2 */


int run_test(char *run_str)
{
  uint64_t p1, p2 /*, p3, p4, p5, p6, p7, p8, p9 */;
  int null_ofs;

  null_ofs = 0;
  (void)sscanf(run_str, "t1(%"SCNu64",%"SCNu64")%n",
      &p1, &p2, &null_ofs);
  if (null_ofs != 0 && run_str[null_ofs] == '\0') {
    return t1(p1, p2);
  }

  null_ofs = 0;
  (void)sscanf(run_str, "t2(%"SCNu64",%"SCNu64")%n",
      &p1, &p2, &null_ofs);
  if (null_ofs != 0 && run_str[null_ofs] == '\0') {
    return t2(p1, p2);
  }

  fprintf(stderr, "Unrecognized test: '%s'\n", run_str);
  return -1;
}  /* run_test */


int main(int argc, char **argv)
{
  int opt;
  int i;
  char **run_strings;
  int num_run_strings;

#ifdef _WIN32
  /* windows-specific code */
  WSADATA wsadata;
  int wsStat = WSAStartup(MAKEWORD(2,2), &wsadata);
  if (wsStat != 0) {printf("line %d: wsStat=%d\n",__LINE__,wsStat);exit(1);}
#endif

  nstm = nstm_create(NSTM_CLOCK_BEST);

  while ((opt = getopt(argc, argv, "hl:")) != EOF) {
    switch (opt) {
      case 'l':
        SAFE_ATOI(optarg, o_loops);
        break;
      case 'h':
        help();
        break;
      default:
        usage(NULL);
    }  /* switch opt */
  }  /* while getopt */

  run_strings = (char **)malloc(sizeof(char **) * argc);

  num_run_strings = 0;
  while (optind < argc) {
    run_strings[num_run_strings] = strdup(argv[optind]);
fprintf(stderr, "run_strings[%d]='%s'\n", num_run_strings, run_strings[num_run_strings]);
    num_run_strings++;
    optind++;
  }

  for (i = 0; i < o_loops; i++) {
    int run_num;
    for (run_num = 0; run_num < num_run_strings; run_num++) {
      if (run_test(run_strings[run_num]) != 0) {
        fprintf(stderr, "Abnormal exit\n");
        exit(1);
      }
    }
  }

#ifdef _WIN32
  WSACleanup();
#endif

  return 0;
}  /* main */
