/* tgen_test.c */

#include <stdio.h>
#include <string.h>
#include "cprt.h"
#include "tgen.h"


/* Options */
int o_max_steps = 99;
char *o_script_str = NULL;

void usage()
{
  printf("Usage: tgen_test -s script_string [-m max_steps]\n");
  exit(0);
}  /* usage */

void get_my_options(int argc, char **argv)
{
  int opt;

  while ((opt = cprt_getopt(argc, argv, "m:s:")) != EOF) {
    switch (opt) {
      case 'm': CPRT_ATOI(cprt_optarg, o_max_steps); break;
      case 's': o_script_str = CPRT_STRDUP(cprt_optarg); break;
      default: usage();
    }  /* switch */
  }  /* while */

  if (o_script_str == NULL) {
    usage();
  }
}  /* get_my_options */


void my_send(int len, int rate, int duration_usec)
{
fprintf(stderr, "my_send, %d %d %d\n", len, rate, duration_usec);
}  /* my_send */


int main(int argc, char **argv)
{
  tgen_t *tgen;

  get_my_options(argc, argv);

  tgen = tgen_create(o_max_steps);

  tgen_add_multi_steps(tgen, o_script_str);

  tgen_run(tgen);

  tgen_delete(tgen);

  return 0;
}
