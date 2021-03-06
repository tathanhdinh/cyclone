#!/bin/sh
# usage: test.sh n prog arg [inputfile]
#   where n is the number of times to run each file
#         prog1 is the program to run
#         arg is the argument to run it with
#         inputfile is the optional input file to redirect to stdin

#TIME="/usr/bin/time --format=\"%e\""
TMP=/tmp/test$$
TMP2=/tmp/testinput$$

# Args

if [ $# -lt 3 ]; then
cat<<EOF
usage: $0 n prog arg [inputfile]
  where n is the number of times to run each file
        prog1 is the program to run
        arg is the argument to run it with
        inputfile is the optional input file to redirect to stdin
EOF
exit 1

else
  N=$1
  CMD=$2
  ARG=$3
  if [ $# -ge 4 ]; then
    if [ "${CMD%regexmatch}" = "${CMD}" ]; then
      INPUTFILE=$4
      ./catn $ARG $INPUTFILE > $TMP2
      INPUTFILE=$TMP2
    else
      INPUTFILE=$4
    fi
  fi
fi

# Run it

NAME=`echo $CMD | awk '{ print $1; }'`
NAME=${NAME%.exe}
echo -n "${NAME#./} "

i=0
rm -f $TMP
while [ "$i" != "$N" ]; do
  if [ -n "$INPUTFILE" ]; then
    $TIME $CMD $ARG 2>> $TMP 1> /dev/null <$INPUTFILE
  else
    $TIME $CMD $ARG 2>> $TMP 1> /dev/null
  fi
  i=`expr $i + 1`
done
cat $TMP | awk '{ printf("%s ",$1); }'
echo
rm -f $TMP
rm -f $TMP2
