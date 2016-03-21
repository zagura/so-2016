#!/bin/bash

Ns=(10000 40000 100000 200000)

for n in "${Ns[@]}"
do
	./fork $n >> result.txt
	./vfork $n >> result.txt
	./clone $n >> result.txt
	./vclone $n >> result.txt
done
