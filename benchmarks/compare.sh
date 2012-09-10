#!/bin/bash

# Run:
# ./compare.sh <shell> "--ion-baseopt#1 --ion-baseopt#2 ..." "--ion-cmpopt#1 --ion-cmpopt#2 ..." <script> <number of executions>

echo "" > /tmp/out1
echo "" > /tmp/out2

for I in `seq 0 $5`; do
    ./$1 -b $2 $4 &>> /tmp/out1
     cat /tmp/ionprofile.out >> /tmp/out1
done

for I in `seq 0 $5`; do
    ./$1 -b $3 $4 &>> /tmp/out2
    cat /tmp/ionprofile.out >> /tmp/out2
done

STAGES="BuildSSA GVN Eliminate_phis Allocate Bounds CP DCEConditionals BCE"

A=`cat /tmp/out1 | grep "runtime" | awk '{ sum+=$3; } END{ print sum; }'`
B=`cat /tmp/out2 | grep "runtime" | awk '{ sum+=$3; } END{ print sum; }'`

printf "%-25s%-15s%-15s%-15s\n" "Step" "First set" "Second set" "Difference" 
echo
printf "%-25s%-15s%-15s%-15s\n" "Total_runtime" $A $B `echo "scale=8; $A - $B" | bc`
printf "%-25s%-15s%-15s%-15s\n" "Average_runtime" `echo "scale=8; $A/$5" | bc` `echo "scale=8; $B/$5" | bc` `echo "scale=8; ($A - $B)/$5" | bc`

for S in `echo $STAGES`; do
    A=`cat /tmp/out1 | grep "$S" | awk '{ sum+=$2; } END{ print sum * 1000; }'`
    B=`cat /tmp/out2 | grep "$S" | awk '{ sum+=$2; } END{ print sum * 1000; }'`
    AOE=`echo "scale=8; $A/$5" | bc`
    BOE=`echo "scale=8; $B/$5" | bc`
    DIF=`echo "scale=8; $AOE - $BOE" | bc`
    printf "%-25s%-15s%-15s%-15s\n" $S $AOE $BOE $DIF
done
