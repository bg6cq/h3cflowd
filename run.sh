#!/bin/bash
#
# run in rc.local
# screen -d -m /usr/src/h3cflowd/run.sh  &

cd /usr/src/h3cflowd

while true; do
	date
	echo starting h3cflowd
	date >> run.log
	echo starting h3cflowd >> run.log
	./h3cflowd | tee -a run.stdout
	echo h3cflowd exit >> run.log
done
