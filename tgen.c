/* tgen.c - Module for constructing a network traffic generator.
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
#include "cprt.h"  /* See https://github.com/fordsfords/cprt */
#include "tgen.h"


/* Return multiplication factor. */
int tgen_convert_byte_multiplier(char *in_str)
{
  if (strcmp(in_str, "bytes") == 0) return 1;
  if (strcmp(in_str, "kbytes") == 0) return 1000;
  if (strcmp(in_str, "mbytes") == 0) return 1000000;

  fprintf(stderr, "Error: invalid byte multiplier '%s'\n", in_str);
  CPRT_ERR_EXIT;
}  /* tgen_convert_byte_multiplier */


/* Return multiplication factor. */
int tgen_convert_rate_multiplier(char *in_str)
{
  if (strcmp(in_str, "persec") == 0) return 1;
  if (strcmp(in_str, "kpersec") == 0) return 1000;
  if (strcmp(in_str, "mpersec") == 0) return 1000000;

  fprintf(stderr, "Error: invalid rate multiplier '%s'\n", in_str);
  CPRT_ERR_EXIT;
}  /* tgen_convert_rate_multiplier */


/* Return multiplication factor to give usec. */
int tgen_convert_duration_multiplier(char *in_str)
{
  if (strcmp(in_str, "usec") == 0) return 1;
  if (strcmp(in_str, "msec") == 0) return 1000;
  if (strcmp(in_str, "sec") == 0) return 1000000;

  fprintf(stderr, "Error: invalid duration multiplier '%s'\n", in_str);
  CPRT_ERR_EXIT;
}  /* tgen_convert_duration_multiplier */


/* Return multiplication factor to give num msgs. */
int tgen_convert_msgs_multiplier(char *in_str)
{
  if (strcmp(in_str, "msgs") == 0) return 1;
  if (strcmp(in_str, "kmsgs") == 0) return 1000;
  if (strcmp(in_str, "mmsgs") == 0) return 1000000;

  fprintf(stderr, "Error: invalid msgs multiplier '%s'\n", in_str);
  CPRT_ERR_EXIT;
}  /* tgen_convert_msgs_multiplier */


/* Return variable index 0-25 (a-z). */
int tgen_convert_variable(char *in_str)
{
  if (strlen(in_str) != 1 || in_str[0] < 'a' || in_str[0] > 'z') {
    fprintf(stderr, "Error: invalid variable variable_name '%s'\n", in_str);
    CPRT_ERR_EXIT;
  }

  return (in_str[0] - 'a');
}  /* tgen_convert_variable */


/*
 * The tgen_parse_*() functions return -1 for error,
 * 0 for success but no steps parsed (e.g. comment),
 * 1 for step parsed.
 */


int tgen_parse_comment(char *iline, tgen_step_t *step)
{
  int null_ofs = 0;

  (void)sscanf(iline, " %n", &null_ofs);
  if (iline[null_ofs] == '\0' || iline[null_ofs] == '#') return 0;

  return -1;
}  /* tgen_parse_comment */


int tgen_parse_sendt(char *iline, tgen_step_t *step)
{
  char byte_multiplier[TGEN_MAX_KEYWORD+1];
  char rate_multiplier[TGEN_MAX_KEYWORD+1];
  char duration_multiplier[TGEN_MAX_KEYWORD+1];
  int null_ofs = 0;

  (void)sscanf(iline, " sendt"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %n",
      &step->len, byte_multiplier,
      &step->rate, rate_multiplier,
      &step->duration_usec, duration_multiplier,
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->len *= tgen_convert_byte_multiplier(byte_multiplier);

  step->rate *= tgen_convert_rate_multiplier(rate_multiplier);

  step->duration_usec *= tgen_convert_duration_multiplier(duration_multiplier);

  step->opcode = TGEN_OPCODE_SENDT;

  return 1;
}  /* tgen_parse_sendt */


int tgen_parse_sendc(char *iline, tgen_step_t *step)
{
  char byte_multiplier[TGEN_MAX_KEYWORD+1];
  char rate_multiplier[TGEN_MAX_KEYWORD+1];
  char msgs_multiplier[TGEN_MAX_KEYWORD+1];
  int null_ofs = 0;

  (void)sscanf(iline, " sendc"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %n",
      &step->len, byte_multiplier,
      &step->rate, rate_multiplier,
      &step->num_msgs, msgs_multiplier,
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->len *= tgen_convert_byte_multiplier(byte_multiplier);

  step->rate *= tgen_convert_rate_multiplier(rate_multiplier);

  step->num_msgs *= tgen_convert_msgs_multiplier(msgs_multiplier);

  step->opcode = TGEN_OPCODE_SENDC;

  return 1;
}  /* tgen_parse_sendc */


int tgen_parse_stop(char *iline, tgen_step_t *step)
{
  int null_ofs = 0;

  (void)sscanf(iline, " stop"
      " %n",
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->opcode = TGEN_OPCODE_STOP;

  return 1;
}  /* tgen_parse_stop */


int tgen_parse_set(char *iline, tgen_step_t *step)
{
  char variable_name[TGEN_MAX_KEYWORD+1];
  int null_ofs = 0;

  (void)sscanf(iline, " set"
      " %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %9u"
      " %n",
      variable_name,
      &step->value,
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->variable_index = tgen_convert_variable(variable_name);

  step->opcode = TGEN_OPCODE_SET;

  return 1;
}  /* tgen_parse_set */


int tgen_parse_loop(char *iline, tgen_step_t *step)
{
  char label_name[TGEN_MAX_KEYWORD+1];
  char variable_name[TGEN_MAX_KEYWORD+1];
  int null_ofs = 0;

  (void)sscanf(iline, " loop"
      " %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %n",
      label_name,
      variable_name,
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->label_index = tgen_convert_variable(label_name);
  step->variable_index = tgen_convert_variable(variable_name);

  step->opcode = TGEN_OPCODE_LOOP;

  return 1;
}  /* tgen_parse_loop */


int tgen_parse_label(tgen_t *tgen, char *iline, tgen_step_t *step)
{
  char variable_name[TGEN_MAX_KEYWORD+1];
  int label_index;
  int null_ofs = 0;

  (void)sscanf(iline, " label"
      " %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %n",
      variable_name,
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  label_index = tgen_convert_variable(variable_name);
  tgen->script->labels[label_index] = step->index;

  return 0;
}  /* tgen_parse_label */


int tgen_parse_delay(char *iline, tgen_step_t *step)
{
  char duration_multiplier[TGEN_MAX_KEYWORD+1];
  int null_ofs = 0;

  (void)sscanf(iline, " delay"
      " %9u %" CPRT_STRDEF(TGEN_MAX_KEYWORD) "[A-Za-z]"
      " %n",
      &step->duration_usec, duration_multiplier,
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->duration_usec *= tgen_convert_duration_multiplier(duration_multiplier);

  step->opcode = TGEN_OPCODE_DELAY;

  return 1;
}  /* tgen_parse_delay */


int tgen_parse_repl(char *iline, tgen_step_t *step)
{
  int null_ofs = 0;

  (void)sscanf(iline, " repl"
      " %n",
      &null_ofs);
  if (iline[null_ofs] != '\0' && iline[null_ofs] != '#') {
    return -1;
  }

  step->opcode = TGEN_OPCODE_REPL;

  return 1;
}  /* tgen_parse_repl */


int tgen_parse_step(tgen_t *tgen, char *iline, tgen_step_t *step)
{
  int stat;

  if ((stat = tgen_parse_comment(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_sendt(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_sendc(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_stop(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_set(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_label(tgen, iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_loop(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_delay(iline, step)) >= 0) return stat;
  if ((stat = tgen_parse_repl(iline, step)) >= 0) return stat;

  fprintf(stderr, "tgen_parse_step: unrecognized input line: '%s'\n", iline);
  return -1;
}  /* tgen_parse_step */


/*
 * Run-time functions.
 */


void tgen_run_sendt(tgen_t *tgen, tgen_step_t *step)
{
  my_sendt(step->len, step->rate, step->duration_usec);
}  /* tgen_run_stop */


void tgen_run_sendc(tgen_t *tgen, tgen_step_t *step)
{
  my_sendc(step->len, step->rate, step->num_msgs);
}  /* tgen_run_stop */

void tgen_run_stop(tgen_t *tgen, tgen_step_t *step)
{
  tgen->state = TGEN_STATE_STOPPED;
}  /* tgen_run_stop */


void tgen_run_set(tgen_t *tgen, tgen_step_t *step)
{
  tgen->variables[step->variable_index] = step->value;
}  /* tgen_run_set */


void tgen_run_loop(tgen_t *tgen, tgen_step_t *step)
{
  if (tgen->script->labels[step->label_index] == -1) {
    fprintf(stderr, "tgen_run_loop: unknown label: %c\n", ('a' + step->label_index));
    CPRT_ERR_EXIT;
  }
  int variable_index = step->variable_index;
  if (tgen->variables[variable_index] > 0) {
    tgen->variables[variable_index] --;
  }

  if (tgen->variables[variable_index] > 0) {
    tgen->pc = tgen->script->labels[step->label_index];
  }
}  /* tgen_run_loop */


void tgen_run_delay(tgen_t *tgen, tgen_step_t *step)
{
  usleep(step->duration_usec);
}  /* tgen_run_set */


void tgen_run_repl(tgen_t *tgen, tgen_step_t *step)
{
  char iline[TGEN_MAX_LINE+1];
  tgen_step_t my_step;

  printf("repl? "); fflush(stdout);
  while (fgets(iline, TGEN_MAX_LINE, stdin)) {
    if (tgen_parse_step(tgen, iline, &my_step) > 0) {
      tgen_run1(tgen, &my_step);
    }
    printf("repl? "); fflush(stdout);
  }
}  /* tgen_run_set */


void tgen_run1(tgen_t *tgen, tgen_step_t *step)
{
  switch (step->opcode) {
  case TGEN_OPCODE_SENDT: tgen_run_sendt(tgen, step); break;
  case TGEN_OPCODE_SENDC: tgen_run_sendc(tgen, step); break;
  case TGEN_OPCODE_STOP: tgen_run_stop(tgen, step); break;
  case TGEN_OPCODE_SET: tgen_run_set(tgen, step); break;
  case TGEN_OPCODE_LOOP: tgen_run_loop(tgen, step); break;
  case TGEN_OPCODE_DELAY: tgen_run_delay(tgen, step); break;
  case TGEN_OPCODE_REPL: tgen_run_repl(tgen, step); break;
  default:
    fprintf(stderr, "tgen_run1: unknown opcode: %d\n", step->opcode);
    CPRT_ERR_EXIT;
  }  /* switch */
}  /* tgen_run1 */


void tgen_run(tgen_t *tgen)
{
  tgen->state = TGEN_STATE_RUNNING;
  while (tgen->state == TGEN_STATE_RUNNING) {
    if (tgen->pc >= tgen->script->num_steps) {
      tgen->state = TGEN_STATE_STOPPED;
    }
    else {
      tgen_step_t *step = &tgen->script->steps[tgen->pc];
      tgen->pc++;

      tgen_run1(tgen, step);
    }
  }
}  /* tgen_run */


/*
 * APIs
 */

tgen_t *tgen_create(int max_steps)
{
  tgen_t *tgen;
  tgen_script_t *script;
  tgen_step_t *steps;
  int i;

  CPRT_ENULL(tgen = malloc(sizeof(tgen_t)));

  CPRT_ENULL(script = malloc(sizeof(tgen_script_t)));

  CPRT_ENULL(steps = malloc(max_steps * sizeof(tgen_script_t)));

  for (i = 0; i < max_steps; i++) {
    steps[i].index = i;
  }

  for (i = 0; i < 26; i++) {
    script->labels[i] = -1;
  }
  script->num_steps = 0;
  script->max_steps = max_steps;
  script->steps = steps;

  for (i = 0; i < 26; i++) {
    tgen->variables[i] = 0;
  }
  tgen->pc = 0;
  tgen->script = script;
  tgen->state = TGEN_STATE_STOPPED;

  return tgen;
}  /* tgen_create */


void tgen_delete(tgen_t *tgen)
{
  free(tgen->script->steps);
  free(tgen->script);
  free(tgen);
}  /* tgen_delete */


void tgen_add_step(tgen_t *tgen, char *iline)
{
  int status;

  CPRT_ASSERT(tgen->script->num_steps < tgen->script->max_steps);

  status = tgen_parse_step(tgen, iline, &tgen->script->steps[tgen->script->num_steps]);
  if (status > 0) {
    tgen->script->num_steps ++;
  }
  else if (status < 0) {
    CPRT_ERR_EXIT;
  }
}  /* tgen_add_step */


void tgen_add_multi_steps(tgen_t *tgen, char *iline)
{
  char *local_buffer = CPRT_STRDUP(iline);
  char *strtok_state;
  char *token;

  token = CPRT_STRTOK(local_buffer, ";\n", &strtok_state);
  while (token != NULL) {
    tgen_add_step(tgen, token);

    token = CPRT_STRTOK(NULL, ";\n", &strtok_state);
  }

  free(local_buffer);
}  /* tgen_add_multi_steps */
