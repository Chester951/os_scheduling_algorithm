#!/bin/bash

# sudo ./scheduler < $1 > output2.txt
for t in RR SJF PSJF FIFO ; do
    for i in {1..5} ; do
        echo "----------------------"$t"_$i----------------------------"
        sudo ./scheduler < "Test/"$t"_$i.txt" > "Output/schd_"$t"_$i.txt"
    done
done