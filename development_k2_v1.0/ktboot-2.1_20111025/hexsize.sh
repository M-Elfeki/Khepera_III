#!/bin/bash

IsNumeric () {
  case "$1" in
    *[!0-9,.]*) return 1;; 	# invalid
    *) return 0;; 		# valid
  esac
}
#trick to manage group name (or user name) with a space
dec1=`ls -l $1 | awk '{print $5'}`
dec2=`ls -l $1 | awk '{print $6'}`

IsNumeric "$dec1"
if [ "$?" -eq "0" ]
then
hex1=`bc <<!
obase=16; $dec1 
!
`

hex1=`echo $hex1 | sed -e 's: :	:g'`
else
hex1=`bc <<!
obase=16; $dec2
!
` 

hex1=`echo $hex1 | sed -e 's: :	:g'`
fi

echo $hex1 
