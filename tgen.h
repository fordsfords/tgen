/* tgen.h - Include file for constructing a network traffic generator.
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
#ifndef TGEN_H
#define TGEN_H

#include "cprt.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TGEN_MAX_LINE 255
#define TGEN_MAX_KEYWORD 15


/* Valid opcodes for steps. */
#define TGEN_OPCODE_SENDT 1
#define TGEN_OPCODE_SENDC 2
#define TGEN_OPCODE_STOP 3
#define TGEN_OPCODE_SET 4
#define TGEN_OPCODE_LOOP 5
#define TGEN_OPCODE_DELAY 6
#define TGEN_OPCODE_REPL 7

struct tgen_step_s {
  int index;
  int opcode;
  int len;
  int rate;
  int duration_usec;
  int num_msgs;
  int variable_index;
  int value;
  int label_index;
};
typedef struct tgen_step_s tgen_step_t;

struct tgen_script_s {
  int labels[26];
  int num_steps;
  int max_steps;
  tgen_step_t *steps;
};


typedef struct tgen_script_s tgen_script_t;
#define TGEN_FLAGS_TST1 0x00000001  /* set during first stage of selftest */

#define TGEN_STATE_STOPPED 0
#define TGEN_STATE_RUNNING 1

struct tgen_s {
  uint32_t flags;
  void *user_data;
  int variables[26];
  int pc;
  int state;  /* TGEN_STATE_... */
  tgen_script_t *script;
};
typedef struct tgen_s tgen_t;


tgen_t *tgen_create(uint32_t flags, void *user_data);
void tgen_delete(tgen_t *tgen);
void *tgen_user_data_get(tgen_t *tgen);
int tgen_variable_get(tgen_t *tgen, char var_id);
void tgen_variable_set(tgen_t *tgen, char var_id, int value);
void tgen_add_step(tgen_t *tgen, char *iline);
void tgen_add_multi_steps(tgen_t *tgen, char *iline);
void tgen_run(tgen_t *tgen);
void tgen_run1(tgen_t *tgen, tgen_step_t *step);

/* Functions the application must provide. */
void my_send(tgen_t *tgen, int len);
void my_variable_change(tgen_t *tgen, char var_id, int value);

#if defined(__cplusplus)
}
#endif

#endif  /* TGEN_H */
