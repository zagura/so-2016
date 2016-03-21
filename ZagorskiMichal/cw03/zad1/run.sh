#!/bin/bash

Ns=(10000 20000 50000 120000)

for n in "${Ns[@]}"
do
	./fork $n >> result.txt
	./vfork $n >> result.txt
	./clone $n >> result.txt
	./vclone $n >> result.txt
done
