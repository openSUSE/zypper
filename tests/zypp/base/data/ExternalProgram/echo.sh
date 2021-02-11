#!/bin/bash

#just read from stdin and write to stdout until we get a single q

while read -r line
do
  if [ "$line" == "q" ]
  then
    exit 0
  fi

  echo "$line"
done
