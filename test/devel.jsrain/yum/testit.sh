#!/bin/bash
# goes through each testcase in this directory
# see README
for testcase in $1/*.test.xml
do
    # echo "$testcase" > testit.log
    mode="${testcase#*-}"
    mode="${mode%%-*}"
    ref="$testcase.ref"
    err_ref="$testcase.err.ref"

    ./YUMtest "$mode" < "$testcase" > "$testcase.out" 2>"$testcase.err.tmp"
    echo "Exit Code: $?" >> "$testcase.out"

    # remove date and host name from logs
    cut -d \  -f 5- < "$testcase.err.tmp" > "$testcase.err.out"
    rm "$testcase.err.tmp"

    if diff "$ref" "$testcase.out" \
       && diff "$err_ref" "$testcase.err.out" 
    then
        echo "OK: $testcase"
    else
        echo "FAILED: $testcase"
        exit 1
    fi
done


