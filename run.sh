#!/bin/bash
#
# run in rc.local
# screen -d -m /usr/src/h3cflowd/run.sh  &

cd /usr/src/h3cflowd

while true; do
	date
	echo starting h3cflowd
	./h3cflowd
done
