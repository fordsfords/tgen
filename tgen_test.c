/* tgen_test.c */

#include <stdio.h>
#include <string.h>
#include "cprt.h"
#include "tgen.h"


struct my_data_s {
  int not_used;
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


void my_send(void *user_data)
{
fprintf(stderr, "my_send\n");
}  /* my_send */


int main(int argc, char **argv)
{
  my_data_t *my_data;
  tgen_t *tgen;

  get_my_options(argc, argv);

  CPRT_ENULL(my_data = (my_data_t *)malloc(sizeof(my_data_t)));

  tgen = tgen_create(o_max_steps, o_flags, &my_data);

  tgen_add_multi_steps(tgen, o_script_str);

  tgen_run(tgen);

  tgen_delete(tgen);

  return 0;
}
