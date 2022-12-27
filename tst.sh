#!/bin/bash
# tst.sh - build the programs on Linux.

if echo "$OSTYPE" | egrep -i darwin >/dev/null; then :
  LIBS="-pthread -l m"
else :
  LIBS="-pthread -l m -l rt"
fi

gcc -Wall -g -o tgen_test cprt.c tgen.c tgen_test.c $LIBS
if [ $? -ne 0 ]; then echo error in tgen.c; exit 1; fi


echo test1
./tgen_test -t 1 -f 2 2>tgen_test.2 <<__EOF__
# comment
sendt 700 bytes 100 persec 20 sec
bad command
sendt 700 bytes 100 persec 21 sec
__EOF__
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
echo passed

echo test2
./tgen_test -t 2 -f 3 -s "sendt 700 bytes 100 persec 1 sec; repl; sendt 700 bytes 100 persec 2 sec; repl; sendt 700 bytes 100 persec 3 sec" 2>tgen_test.2 <<__EOF__
# comment
sendt 700 bytes 100 persec 20 sec
bad command
sendt 700 bytes 100 persec 21 sec
__EOF__
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if egrep "sendt, 700 100 1000000" tgen_test.2 >/dev/null; then :; else echo failed 2; exit 1; fi
if egrep "sendt, 700 100 20000000" tgen_test.2 >/dev/null; then :; else echo failed 3; exit 1; fi
if egrep "tgen_parse_step: unrecognized input line: 'bad command" tgen_test.2 >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "sendt, 700 100 21000000" tgen_test.2 >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "sendt, 700 100 3000000" tgen_test.2 >/dev/null; then :; else echo failed 5; exit 1; fi
echo passed

echo test3
./tgen_test -t 2 -f 3 -s "
# comment 1

 # comment 3
	
	# comment 5
 	 	
	 	 # comment 6
sendt 700 bytes 100 persec 1 sec
 sendt  700	kbytes 	100	 kpersec 2 msec # asdf
	sendt  700	mbytes 	100	 mpersec 3 usec #
" 2>tgen_test.2  >tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.2`" -ne 3 ]; then echo failed 2; exit 1; fi
if egrep "sendt, 700 100 1000000" tgen_test.2 >/dev/null; then :; else echo failed 3; exit 1; fi
if egrep "sendt, 700000 100000 2000" tgen_test.2 >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "sendt, 700000000 100000000 3" tgen_test.2 >/dev/null; then :; else echo failed 5; exit 1; fi
echo passed

echo test4
./tgen_test -t 2 -f 3 -s "
# comment 1

 # comment 3
	
	# comment 5
 	 	
	 	 # comment 6
sendc 700 bytes 100 persec 1 msgs
 sendc  700	kbytes 	100	 kpersec 2 kmsgs # asdf
	sendc  700	mbytes 	100	 mpersec 3 mmsgs #
" 2>tgen_test.2  >tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.2`" -ne 3 ]; then echo failed 2; exit 1; fi
if egrep "sendc, 700 100 1" tgen_test.2 >/dev/null; then :; else echo failed 3; exit 1; fi
if egrep "sendc, 700000 100000 2000" tgen_test.2 >/dev/null; then :; else echo failed 4; exit 1; fi
if egrep "sendc, 700000000 100000000 3000000" tgen_test.2 >/dev/null; then :; else echo failed 5; exit 1; fi
echo passed

echo test5
./tgen_test -t 2 -f 3 -s "set i 10;label l;sendt 700 bytes 100 persec 1 sec; delay 3 sec" 2>tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.2`" -ne 2 ]; then echo failed 2; exit 1; fi
if egrep "sendt, 700 100 1000000" tgen_test.2 >/dev/null; then :; else echo failed 3; exit 1; fi
if egrep "Variable i = 10" tgen_test.2 >/dev/null; then :; else echo failed 4; exit 1; fi
echo passed

# This is the same as test3() in "tgen_test.c".
echo test6
rm -f time.out
command time -p -o time.out ./tgen_test -t 0 -f 3 -s "
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
" 2>tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.2`" -ne 11 ]; then echo failed 2; exit 1; fi
if egrep "sendt, 700 100 4000000" tgen_test.2 >/dev/null; then :; else echo failed 3; exit 1; fi
# Nine sleeps of 200 ms each = 1.80 seconds. Get the "real" time and
# remove the decimal to get 180 (nominally).
T=`sed -n 's/real \([0-9]*\)\.\([0-9]*\)$/\1\2/p' <time.out`
if [ "$T" -lt 170 -o "$T" -gt 195 ]; then echo failed 4; exit 1; fi
echo passed

# Test3() in "tgen_test.c".
echo test7
rm -f time.out
command time -p -o time.out ./tgen_test -t 3 -f 3 2>tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
if [ "`wc -l <tgen_test.2`" -ne 11 ]; then echo failed 2; exit 1; fi
if egrep "sendt, 700 100 4000000" tgen_test.2 >/dev/null; then :; else echo failed 3; exit 1; fi
# Nine sleeps of 200 ms each = 1.80 seconds. Get the "real" time and
# remove the decimal to get 180 (nominally).
T=`sed -n 's/real \([0-9]*\)\.\([0-9]*\)$/\1\2/p' <time.out`
if [ "$T" -lt 170 -o "$T" -gt 195 ]; then echo failed 4; exit 1; fi
echo passed

echo test8
rm -f time.out
# The set of variable 'z' is for the test assertion inside "my_send()".
command time -p -o time.out ./tgen_test -f 2 -t 2 -s "set z 271828; sendt 700 bytes 100 persec 1 sec" >tgen_test.1 2>tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
SEND_CNT="`egrep "send message" <tgen_test.2 | wc -l`"
if [ $SEND_CNT -lt 99 -o $SEND_CNT -gt 103 ]; then echo failed 2; exit 1; fi
T=`sed -n 's/real \([0-9]*\)\.\([0-9]*\)$/\1\2/p' <time.out`
if [ "$T" -lt 98 -o "$T" -gt 102 ]; then echo failed 3; exit 1; fi
if egrep "sendt len=700 rate=100 duration_usec=1000000, actual rate=100, actual msgs=100" tgen_test.1 >/dev/null; then :; else echo failed 4; exit 1; fi
echo passed

echo test9
rm -f time.out
# The set of variable 'z' is for the test assertion inside "my_send()".
command time -p -o time.out ./tgen_test -f 2 -t 2 -s "set z 271828; sendc 700 bytes 100 persec 101 msgs" >tgen_test.1 2>tgen_test.2
STATUS=$?

# Success status is expected
if [ "$STATUS" -ne 0 ]; then echo failed 1; exit 1; fi
SEND_CNT="`egrep "send message" <tgen_test.2 | wc -l`"
if [ $SEND_CNT -ne 101 ]; then echo failed 2; exit 1; fi
T=`sed -n 's/real \([0-9]*\)\.\([0-9]*\)$/\1\2/p' <time.out`
if [ "$T" -lt 98 -o "$T" -gt 102 ]; then echo failed 3; exit 1; fi
if egrep "sendc len=700 rate=100 num_msgs=101, actual rate=100" tgen_test.1 >/dev/null; then :; else echo failed 4; exit 1; fi
echo passed
