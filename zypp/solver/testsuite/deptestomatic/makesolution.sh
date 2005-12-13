#!/bin/bash
test=$1
base=`basename $1 .xml`
../../testsuite/deptestomatic $test | grep \>\!\> > $base.solution
