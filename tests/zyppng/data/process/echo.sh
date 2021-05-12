#!/bin/bash

#just read from the predefined fd (3) until we get a single q and echo everything else to the predefined write fd (4)

while read -u 3 -r line
do
  if [ "$line" == "q" ]
  then
    printf "Byt Bye" >&2;
    exit 0
  fi

  echo "$line" >&4
  echo "Received line $line" >&2
done
exit 123
