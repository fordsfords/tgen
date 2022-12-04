#!/bin/sh
# tst.sh - build the programs on Linux.

LIBS="-pthread -l m -l rt"

gcc -Wall -g -o tgen_test cprt.c tgen.c tgen_test.c $LIBS
if [ $? -ne 0 ]; then echo error in tgen.c; exit 1; fi


echo test1
./tgen_test -m 3 -s "
# comment 1

 # comment 3
	
	# comment 5
 	 	
	 	 # comment 6
send 700 bytes 100 persec 1 sec
 send  700	kbytes 	100	 kpersec 2 msec # asdf
	send  700	mbytes 	100	 mpersec 3 usec #
	send  700	mbytes 	100	 mpersec 4 usec #" >tgen_test.out 2>&1
STATUS=$?

# Failed status is expected
if [ "$STATUS" -ne 1 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.out`" -ne 1 ]; then echo failed 2; exit 1; fi
if egrep "ERROR: 'tgen->script->num_steps < tgen->script->max_steps' not true" tgen_test.out >/dev/null; then :; else echo failed 3; exit 1; fi
echo passed

echo test2
./tgen_test -m 99 -s "send 700 bytes 100 persec 1 sec; repl; send 700 bytes 100 persec 2 sec; repl; send 700 bytes 100 persec 3 sec" >tgen_test.out 2>&1 <<__EOF__
# comment
send 700 bytes 100 persec 20 sec
bad command
send 700 bytes 100 persec 21 sec
__EOF__
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if egrep "my_send, 700 100 1000000" tgen_test.out >/dev/null; then :; else echo failed 2; exit 1; fi
if egrep "my_send, 700 100 20000000" tgen_test.out >/dev/null; then :; else echo failed 3; exit 1; fi
if egrep "tgen_parse_step: unrecognized input line: 'bad command" tgen_test.out >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "my_send, 700 100 21000000" tgen_test.out >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "my_send, 700 100 3000000" tgen_test.out >/dev/null; then :; else echo failed 5; exit 1; fi
echo passed

echo test3
./tgen_test -m 3 -s "
# comment 1

 # comment 3
	
	# comment 5
 	 	
	 	 # comment 6
send 700 bytes 100 persec 1 sec
 send  700	kbytes 	100	 kpersec 2 msec # asdf
	send  700	mbytes 	100	 mpersec 3 usec #
" >tgen_test.out 2>&1  >tgen_test.out
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.out`" -ne 3 ]; then echo failed 2; exit 1; fi
if egrep "my_send, 700 100 1000000" tgen_test.out >/dev/null; then :; else echo failed 3; exit 1; fi
if egrep "my_send, 700000 100000 2000" tgen_test.out >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "my_send, 700000000 100000000 3" tgen_test.out >/dev/null; then :; else echo failed 5; exit 1; fi
echo passed

echo test4
./tgen_test -m 99 -s "set i 10;label l;send 700 bytes 100 persec 1 sec;stop;send 700 bytes 100 persec 9 sec" >tgen_test.out 2>&1
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.out`" -ne 1 ]; then echo failed 2; exit 1; fi
if egrep "my_send, 700 100 1000000" tgen_test.out >/dev/null; then :; else echo failed 3; exit 1; fi
echo passed

echo test5
rm -f time.out
time -p -o time.out ./tgen_test -m 99 -s "
send 700 bytes 100 persec 1 sec
set i 3
label l
  set j 2
  label m
    send 700 bytes 100 persec 2 sec
    delay 200 msec
  loop m j
  send 700 bytes 100 persec 3 sec
  delay 200 msec
loop l i
send 700 bytes 100 persec 4 sec
" >tgen_test.out 2>&1
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.out`" -ne 11 ]; then echo failed 2; exit 1; fi
if egrep "my_send, 700 100 4000000" tgen_test.out >/dev/null; then :; else echo failed 3; exit 1; fi
T=`sed -n 's/real \([0-9]*\)\.\([0-9]*\)$/\1\2/p' <time.out`
if [ "$T" -lt 170 -o "$T" -gt 195 ]; then echo failed 4; exit 1; fi
echo passed
