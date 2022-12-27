# tgen - Traffic generator

Utility functions to assist in the creation of a traffic generator.

# Table of contents

- [tgen - Traffic generator](#tgen---traffic-generator)
- [Introduction](#introduction)
  - [Repository](#repository)
- [Quick Start](#quick-start)
- [Scripting Language](#scripting-language)
- [Embedded API](#embedded-api)
- [Sending Messages](#sending-messages)
- [Variables, Labels, and Looping](#variables-labels-and-looping)
  - [Special Variables](#special-variables)
- [REPL](#repl)
- [Instruction Set](#instruction-set)
  - [Comment](#comment)
  - [Sendt](#sendt)
  - [Sendc](#sendc)
  - [Set](#set)
  - [Label](#label)
  - [Loop](#loop)
  - [Delay](#delay)
  - [Repl](#repl)
- [TODO](#todo)
- [License](#license)
<sup>(table of contents from https://luciopaiva.com/markdown-toc/)</sup>

# Introduction

This is a "little language" processing module and API designed to assist
in the construction of a network traffic generator.

The tgen module is not actually a network traffic generator.
It is an API to make it easier to write a flexiable,
easy-to-use traffic generator.
It is an "orchestrator".
I could imagine this being repurposed to create a
signal generator, a data sampler,
or anything else that requires operations be performed on a time base.

## Repository

See https://github.com/fordsfords/tgen for code and documentation.

# Quick Start

1. Download the repository to a Linux system.
1. Run the "tst.sh" script.
This builds and runs a self-test.

After that, you'll to integrate your network messaging code
by writing your own "main" program.
You supply the following functions:
* "my_send()" - send a single network message.
* "my_variable_change()" -
called if the user sets the value of a scripting variable.

Your main program calls:
* tgen_create() - create an instance of the tgen interpreter.
* tgen_add_multi_steps() - save script instructions to the interpreter.
* tgen_run() - start the interpreter running.
It will return when the script completes.
* tgen_delete() - graceful shutdown and cleanup of tgen instance.

A real traffic generator based on tgen is at:
https://github.com/UltraMessaging/um_tgen
Use that as a guide for your own.

# Scripting Language

The tgen module implements a "little scripting language".
The scripts written in that language are very simple.

The format of a script is a series of instructions separated by
either newlines or semi-colons.
Fields within an instruction are separated by one or more
whitespace (spaces and tabs are considered equivelant).
Indention is allowed.

Here's an example usage:
````
./tgen_test -t 0 -s "
  delay 200 msec # let topic resolution happen.
  sendc 700 bytes 2 persec 10 msgs
  delay 2 sec   # linger to allow NAK/retransmits to complete."
````
Note that the "-s" option (script) has a multi-line value.
This same script could be written without comments and with semi-colons instead of newlines:
````
./tgen_test -t 0 -s "delay 200 msec; sendc 700 bytes 2 persec 10 msgs; delay 2 sec"
````

Each instruction consists of a keyword,
followed by zero or more values and keywords.
The field format and ordering for a given instruction is fixed.
For example, this is a valid "sendt" instruction:
````
sendt 700 bytes 50 kpersec 3 sec
````
These are invalid:
````
sendt 50 kpersec 700 bytes 3 sec  # Field order is wrong.
sendt 700 bytes 50 kpersec        # 2 fields missing at end.
SendT 700 Bytes 50 KPerSec 3 Sec  # Upper-case not allowed.
````

Numeric fields may be specified in hexidecimal by prefixing
with "0x".

# Embedded API

Little languages are usually ... little.
They usually lack the rich expressiveness of a regular language.
For example, the tgen scripting language doesn't have support for subroutines.

Rather than adding more and more language features,
tgen offers its basic functional operations as an API,
allowing you to write your "script" in C/C++.
This would be compiled directly into your traffic generator tool,
"embedding the script".

For example, the script:
````
sendt 700 bytes 100 persec 1 sec
set i 3
label l
  set j 2
  label m
    sendt 700 bytes 100 persec 2 sec
    delay 200 msec
  loop m j
  sendt 700 bytes 100 persec 3 sec
  delay 200 msec
loop l i
sendt 700 bytes 100 persec 4 sec
````
could be written in C as:
````
  my_data_t my_data;
  tgen_t *tgen;
  int i, j;

  my_data.test_int = 314159;

  tgen = tgen_create(o_flags, &my_data);

  tgen_run_sendt(tgen, 700, 100, 1000000);  /* 1 sec. */
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 2; j++) {
      tgen_run_sendt(tgen, 700, 100, 2000000);  /* 2 sec. */
      tgen_run_delay(tgen, 200000);  /* 200 msec. */
    }
    tgen_run_sendt(tgen, 700, 100, 3000000);  /* 3 sec. */
    tgen_run_delay(tgen, 200000);  /* 200 msec. */
  }
  tgen_run_sendt(tgen, 700, 100, 4000000);  /* 4 sec. */

  tgen_delete(tgen);
````

One downside to embedding scripts is that you need the full source package to
make changes to the script so that it can be re-built.
This makes it more difficult to use the tool in an ad-hoc testing or
exploratory manner.
I would guess that embedding scripts make the most sense for
well-defined QA/acceptance testing.

See "Instruction Set" (below) for API details.

# Sending Messages

Sending messages is the basic function of a traffic generator.
The tgen module has two instructions:
* sendt - send messages for a specified period of time.
* sendc - send a specific number of messages.

Both allow specifying a sending rate.
It uses a busy-looping algorithm to achieve
even spacing between messages.
For example:
````
sendt 700 bytes 50 kpersec 3 sec
````
This will send 700-byte messages at very close to
50,000 messages per second for 3 seconds.
You will see that the messages are separated by
almost exactly 20 microseconds.

Note that the tool does not initialize the
message contents.
It's just whatever malloc returned.

# Variables, Labels, and Looping

The tgen scripting language supports 26 general-purpose integer variables ('a' - 'z').
They are most commonly used with the "loop" instruction,
or to communicate with the application as "special variables" (see below).

Here's an example of a loop instruction:
````
./tgen_test -t 0 -s "
  delay 200 msec # let topic resolution happen.
  set i 10
  label a
    sendt 700 bytes 50 kpersec 4 sec
    sendt 700 bytes 999 mpersec 100 msec
  loop a i  # Decrement i, loop to a while i > 0.
  delay 4 sec   # linger to allow NAK/retransmits to complete."
````
The two "sendt" instructions send 4 seconds at 50,000 msgs/sec followed by a 100 ms burst at maximum send rate.
This is repeated 10 times.

Note that the label ('a' - 'z') are a separate name space from variable ('a' - 'z').
I.e. you can have a label 'a' and an unrelated variable 'a'.

## Special Variables

I wanted an easy way for a tgen script to interact with the traffic generator
application code.
For example, in the Ultra Messaging product,
you can set a "loss" value that discards a specified percentage of
outgoing datagrams.
But this is specific to the UM product, so I didn't want to add a "loss" primitive.
Instead, I added a general-purpose callback: 
````
  void my_variable_change(tgen_t *tgen, char var_id, int value);
````
This application callback is invoked any time any variable is changed.
The intent is that the application will perform some action when a specific
variable is changed.

For example:
````
void my_variable_change(tgen_t *tgen, char var_id, int value)
{
  switch (var_id) {
    /* Variable "l" controls LBT-RM loss rate. */
    case 'l': lbm_set_lbtrm_src_loss_rate(tgen_variable_get(tgen, var_id)); break;
  }
}  /* my_variable_change */
````

This allows a script to change the loss rate during execution of the tgen script.
For example:
````
  set l 100 # set loss to 100%
  sendc 700 bytes 999 mpersec 1 msgs  # this message dropped.
  set l 0 # set loss to 0%
  sendc 700 bytes 999 mpersec 1 msgs  # this message is delivered..
````

# REPL

A [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop)
is a "Read-Eval-Print Loop",
allowing the user to interactively enter commands.

Here's an example of using the 'repl' instruction:

````
./tgen_test -t 0 -s "repl"
repl?
````
The "tgen_test" command has created the tgen instance
and is now waiting for you to enter a command.
For example, type a "sendc" command to send a message:
````
repl? sendc 700 bytes 1 persec 1 msgs
repl?
````

To exit the REPL loop, type "ctrl-d" (signals EOF on standard input).
This continues the script to the next instruction.

However, note that the REPL is purely interactive.
The "label" and "loop" instructions don't work.

# Instruction Set

## Comment
````
[instruction] # comment
````

Example:
````
# comment 1
delay 1 sec # comment 2
````

## Sendt

Send a set of messages at a requested rate for a specified
period of time.
````
sendt N {bytes|kbytes|mbytes} R {persec|kpersec|mpersec} T {sec|msec|usec}
````
where:
* N - size of message ('kbytes' = 1,000 bytes, 'mbytes' = 1,000,000 bytes).
* R - send rate ('kpersec' = 1,000 per sec, 'mpersec' = 1,000,000 per sec).
* T - time sending ('msec' = milliseconds, 'usec' = microseconds).

Example:
````
sendt 10 kbytes 30 kpersec 200 msec
````
Send messages of 10,000 bytes each at a rate of 30,000 messages/sec for 200 milliseconds.

API:
````
void tgen_run_sendt(tgen_t *tgen, int len, int rate, int duration_usec);
````

Note that the tool does not initialize the
message contents.
It's just whatever malloc returned.

Note that sendt will make its best effort to send at the requested rate.
If you specify a rate that UM cannot support (like 999 mpersec),
it will simply send without any delay between sends.

## Sendc

Send a set of messages at a requested rate for a specified
number of messages.
````
sendc N {bytes|kbytes|mbytes} R {persec|kpersec|mpersec} C {msgs|kmsgs|mmsgs}
````
where:
* N - size of message ('kbytes' = 1,000 bytes, 'mbytes' = 1,000,000 bytes).
* R - send rate ('kpersec' = 1,000 per sec, 'mpersec' = 1,000,000 per sec).
* C - message count ('kmsgs' = 1,000 messages, 'mmsgs' = 1,000,000 messages)).

Example:
````
sendc 10 kbytes 30 kpersec 200 kmsgs
````
Send messages of 10,000 bytes each at a rate of 30,000 messages/sec for 200,000 messages.

API:
````
void tgen_run_sendc(tgen_t *tgen, int len, int rate, int duration_usec);
````

Note that the tool does not initialize the
message contents.
It's just whatever malloc returned.

Note that sendc will make its best effort to send at the requested rate.
If you specify a rate that UM cannot support (like 999 mpersec),
it will simply send without any delay between sends.

## Set

Set a variable to a value.
````
set ID VAL
````
where:
* ID - variable identifier 'a' .. 'z' (26 variables total).
* VAL - numeric integer value.

Example:
````
set b 256
set c 0x1F
````
Sets variable b to 256 and variable c to 31.

API:
````
void tgen_run_set(tgen_t *tgen, int variable_index, int value);
````

## Label

Define a label in the script.
````
label ID
````
where:
* ID - label identifier 'a' .. 'z' (26 labels total).

Example:
````
label b
````

No API available (doesn't make sense for embedded use).

## Loop

Decrement a variable and loop if positive.
````
loop LAB_ID VAR_ID
````
where:
* LAB_ID - label to branch to 'a' .. 'z'.
* VAR_ID - variable to decrement.

Example:
````
set i 5
label a
  set j 3
  label b
    delay 1 msec
  loop b j
loop a i
````
Nested loops. The delay is executed 5 * 3 = 15 times.

No API available (doesn't make sense for embedded use).

## Delay

Pause for a period of time.
Uses busy looping to get high accuracy.
````
delay T {sec|msec|usec}
````
where:
* T - time sleeping ('msec' = milliseconds, 'usec' = microseconds).

Example:
````
delay 10 msec
````

API:
````
void tgen_run_delay(tgen_t *tgen, int duration_usec);
````

## Repl

Enter Read-Eval-Print Loop.
Read instructions from stdin and
execute them until EOF.
````
repl
````

Example:
````
repl
````
Usually used interactively,
with EOF supplied by typing "ctrl-d".

API:
````
void tgen_run_repl(tgen_t *tgen);
````

# TODO

I want to be careful not to bloat this module.
Little languages have a tendeny to outgrow their original designs,
making them hard to use and maintain.
*COUGH*perl*COUGH*

That said...

* It might be nice to support setting the contents
of the messages being sent.
It should probably allow inclusion of a changing
value with each message.
Possibly even "verifiable" messages (per the
UM example apps).

* It might be nice to support multiple sources (like lbmmsrc or lbmstrm).

* It might be nice to support file inclusion for scripts.

* It might be nice to supply instruction arguments via
variables.
I.e. instead of "delay 5 msec", maybe something like "delay i msec"
which uses the value in variable 'i'.

* It might be nice to support multi-character variable names
and labels.
Allows better self-documenting scripts.

* It might be nice to be able to orchestrate multiple publishers
on different hosts.
E.g. be able to burst multiple publishers at the same time.

* It might be nice to support if/then/else, for loops, while loops, functions, parameters,
arrays, strings, floating point variables, macros, structures, etc, etc, etc.

**OR NOT**

It probably doesn't make sense to do many (if any) of those.
Instead, a user should just write their script in C,
calling the "..._run" functions as needed.
That gives you the full richness and speed of C.

It's hard to justify putting too much effort into
making a "little language" rich.
Olin Shivers makes a good case against it in the introductory sections of
https://3e8.org/pub/scheme/doc/Universal%20Scripting%20Framework%20(Lambda%20as%20little%20language).pdf

# License

I want there to be NO barriers to using this code,
so I am releasing it to the public domain.
But "public domain" does not have an internationally agreed upon definition,
so I use CC0:

Copyright 2022-2022 Steven Ford http://geeky-boy.com and licensed
"public domain" style under
[CC0](http://creativecommons.org/publicdomain/zero/1.0/):
![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png "CC0")

To the extent possible under law, the contributors to this project have
waived all copyright and related or neighboring rights to this work.
In other words, you can use this code for any purpose without any
restrictions.  This work is published from: United States.  The project home
is https://github.com/fordsfords/cprt

To contact me, Steve Ford, project owner, you can find my email address
at http://geeky-boy.com.  Can't see it?  Keep looking.
