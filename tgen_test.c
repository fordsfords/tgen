/* tgen_test.c - Test program for the tgen module.
 * See https://github.com/fordsfords/tgen
 */
/*
# This code and its documentation is Copyright 2022-2022 Steven Ford
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
#include "cprt.h"
#include "tgen.h"


struct my_data_s {
  int test_int;
};
typedef struct my_data_s my_data_t;


/* Options */
int o_flags = 0;
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

  while ((opt = cprt_getopt(argc, argv, "f:m:s:")) != EOF) {
    switch (opt) {
      case 'f': CPRT_ATOI(cprt_optarg, o_flags); break;
      case 'm': CPRT_ATOI(cprt_optarg, o_max_steps); break;
      case 's': o_script_str = CPRT_STRDUP(cprt_optarg); break;
      default: usage();
    }  /* switch */
  }  /* while */

  if (o_script_str == NULL) {
    usage();
  }
}  /* get_my_options */


void my_send(int len, tgen_t *tgen)
{
  my_data_t *my_data = (my_data_t *)tgen_user_data_get(tgen);
  CPRT_ASSERT(my_data->test_int == 314159);
  CPRT_ASSERT(tgen_variable_get(tgen, 'z') == 271828);
  fprintf(stderr, "send message %d\n", len);
}  /* my_send */


int main(int argc, char **argv)
{
  my_data_t my_data;
  tgen_t *tgen;

  get_my_options(argc, argv);

  my_data.test_int = 314159;

  tgen = tgen_create(o_max_steps, o_flags, &my_data);

  tgen_add_multi_steps(tgen, o_script_str);

  tgen_run(tgen);

  tgen_delete(tgen);

  return 0;
}
