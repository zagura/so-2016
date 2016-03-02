#!/bin/bash

<<MANUAL
Simple script for archiving one chosen task.
It should be placed in the directory which contains directory named like NAME variable.

Scirpt takes one argument which is the number of exercise in two digts format, like: 
01, 02, etc.

Requires directory structurre explained at coures site.

For use at 'Systemy Operacyjne' (Operating systems) at I@IET AGH UST in Krakow.
Script by Michal Zagorski.
MANUAL

# Author of files
NAME="ZagorskiMichal" # Archive's name first part

# Options for tar program
OP="-cvzf"
#extension of file
EXT=".tar.gz"
CW=cw$1

# Case with no arguments
if [ $# -eq 0 ]
  then
	echo "Wrong argument of script!"
	echo "Read manual inside this file"
elif [ $# -eq 1 ]
  then	
	#full command
	tar $OP $NAME-$CW$EXT $NAME/$CW
fi

