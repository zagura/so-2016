#!/bin/bash

#RECORD_SIZE=(4 512 4096 8192)
RECORD_SIZE=(4 512 4096 8192)
RECORDS_COUNT=(20 100 500 1000)
#RECORDS_COUNT=(100 1000 5000 15000 100000)
FILE="./files/f"
TYPE=(lib sys)
mkdir files
for r in "${RECORD_SIZE[@]}";do
	echo "RECORD_size: $r"
	for c in "${RECORDS_COUNT[@]}";do
		echo "Count : $c"
		./generator.exe $FILE'_'$r'_'$c $r $c
		for t in "${TYPE[@]}";do
			echo "  type: $t";		
			cp $FILE'_'$r'_'$c $FILE'_'$r'_'$c'_'$t
			echo "Differnce `diff $FILE'_'$r'_'$c $FILE'_'$r'_'$c'_'$t`"
			echo "File parameters $FILE $r $s $t"
			./readfile.exe $FILE'_'$r'_'$c'_'$t $r $t
			echo "EXECUTING..."
			echo "Differnce `diff $FILE'_'$r'_'$c $FILE'_'$r'_'$c'_'$t`"
			echo "---------------------------------------------------"
		done
		echo "Differnce `diff $FILE'_'$r'_'$c'_'lib $FILE'_'$r'_'$c'_'sys`"
		echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	done
	echo "=============================================="
	echo "=============================================="
done


