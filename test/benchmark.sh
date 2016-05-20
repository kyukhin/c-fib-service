#!/bin/bash

if [ !  ` which ab` ]; then
    echo "Please install ab tool"
fi

echo "HOST: localhost:$1"

# Concurrency level
C=4

echo "Benchmarking concurrency=$C fib_num=$2"

for i in {1..500}
do
    F=$(($RANDOM % $2 + 1))
    ab  -n  $C -c $C http://0.0.0.0:$1/fib/$F 2>&1 >/dev/null
    if [ $? -ne 0 ] ; then
	echo "ERROR: benchmarking is invalid."
    fi
    echo -n "*"
done
echo
