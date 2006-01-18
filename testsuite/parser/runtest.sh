#!/bin/bash
# goes through each testcase in this directory
# see README
testcase=$1;
mode="${testcase#*-}"
mode="${mode%%-*}"

./YUMtest "$mode" < "$testcase"
echo "Exit Code: $?"
