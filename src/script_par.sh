#!/bin/bash

echo "$1 $2 $3 $4 $5">>log_par.txt
	
	rm -rf $3
    mkdir $3

 for((j=$6;j<=$7;j=j+$8)); do

 	echo "parallel degree $j"
	for((i=0; i<$9; i=i+1)); do
	./$1 $2 $3 $4 $5 $j>>log_test.txt
	rm -rf $3
    mkdir $3
	done
done



