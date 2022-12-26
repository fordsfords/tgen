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
char *o_script_str = NULL;
int o_test_num = -1;

void usage(int exit_status)
{
  printf("Usage: tgen_test [-h] [-s script_string] [-t test_num]\n");
  exit(exit_status);
}  /* usage */

void get_my_options(int argc, char **argv)
{
  int opt;

  while ((opt = cprt_getopt(argc, argv, "hf:s:t:")) != EOF) {
    switch (opt) {
      case 'h': usage(0);
      case 'f': CPRT_ATOI(cprt_optarg, o_flags); break;
      case 's': o_script_str = CPRT_STRDUP(cprt_optarg); break;
      case 't': CPRT_ATOI(cprt_optarg, o_test_num); break;
      default: usage(1);
    }  /* switch */
  }  /* while */

  if (o_test_num == -1) {
    fprintf(stderr, "Test number ('-t test_num') is required.\n");
    usage(1);
  }

  if (o_test_num != 1 && o_script_str == NULL) {
    fprintf(stderr, "test %d requires script\n", o_test_num);
    usage(1);
  }
}  /* get_my_options */


void my_send(tgen_t *tgen, int len)
{
  my_data_t *my_data = (my_data_t *)tgen_user_data_get(tgen);
  if (o_test_num > 0) {
    CPRT_ASSERT(my_data->test_int == 314159);
    CPRT_ASSERT(tgen_variable_get(tgen, 'z') == 271828);
  }
  fprintf(stderr, "send message %d\n", len);
}  /* my_send */


void my_variable_change(tgen_t *tgen, char var_id, int value)
{
  CPRT_ASSERT(value == tgen_variable_get(tgen, var_id));
  fprintf(stderr, "Variable %c = %d\n", var_id, value);
}  /* my_variable_change */


void test0()
{
  my_data_t my_data;
  tgen_t *tgen;

  my_data.test_int = 314159;

  tgen = tgen_create(o_flags, &my_data);

  tgen_add_multi_steps(tgen, o_script_str);

  tgen_run(tgen);

  tgen_delete(tgen);
}  /* test0 */


void test1()
{
  tgen_t *tgen;
  int initial_max_steps;
  int i;

  tgen = tgen_create(o_flags, NULL);

  initial_max_steps = tgen->script->max_steps;
  for (i = 0; i < initial_max_steps; i++) {
    tgen_add_step(tgen, "sendc 1 bytes 999999 mpersec 1 msgs");
  }
  CPRT_ASSERT(initial_max_steps == tgen->script->max_steps);
  tgen_add_step(tgen, "sendc 1 bytes 999999 mpersec 1 msgs");
  CPRT_ASSERT(2 * initial_max_steps == tgen->script->max_steps);

  tgen_delete(tgen);
}  /* test1 */


void test2()
{
  my_data_t my_data;
  tgen_t *tgen;

  my_data.test_int = 314159;

  tgen = tgen_create(o_flags, &my_data);

  tgen_add_multi_steps(tgen, o_script_str);

  tgen_run(tgen);

  tgen_delete(tgen);
}  /* test2 */


int main(int argc, char **argv)
{
  get_my_options(argc, argv);

  switch (o_test_num) {
    case 0: test0(); break;
    case 1: test1(); break;
    case 2: test2(); break;

    default: fprintf(stderr, "unknown test %d\n", o_test_num); exit(1);
  }

  return 0;
}  /* main */
