#!/bin/bash
line="start"
while [ -n "$line" ]
do
  read -e -t 1 line < $1 
  #echo $line
done
