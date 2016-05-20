#!/bin/bash

if [ !  ` which ab` ]; then
    echo "Please install ab tool"
fi

echo "HOST: localhost:$1"

# Number of request in ab invocation
I=10
# Concurrency level
C=10

echo "Benchmarking iterations=$I concurrency=$C fib_num=$2"

T=`
time for i in {1..2}
do
    F=($RANDOM % $2 + 1)
    ab -n $I -c $C http://0.0.0.0:$1/fib/$F 2>&1 1>/dev/null
    if [ $? -ne 0 ] ; then
	echo "ERROR: benchmarking is invalid."
    fi
done`
