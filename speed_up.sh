#!/bin/bash

REP="${1}"
THREADS="${2}"

for (( i=1; i<=$REP; i++ )); do
	(>&2 echo "$i")
	RUNTIME=$( { time ./brandes ${THREADS} wiki.in wiki.out; } 2>&1 )
	IFS=$'\n' read -rd '' -a TIMES <<<"$RUNTIME"
	REAL=`echo "${TIMES[0]}" | cut -f2`
	USER=`echo "${TIMES[1]}" | cut -f2`
	SYS=`echo "${TIMES[2]}" | cut -f2`
	echo "$REAL $USER $SYS"
done
