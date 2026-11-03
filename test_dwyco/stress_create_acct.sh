#!/bin/bash
i=0
while [ $i -lt 500 ]
do
	./create_account /tmp/dwy$i &
	i=`expr $i + 1`
done
wait

