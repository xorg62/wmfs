#!/bin/sh
# Print

TIMING=1
RET=0

statustext()
{
     local DATE=`date`

     wmfs -s "$DATE"

     RET=$?
}

while [[ $RET == "0" ]];
do
     statustext
     sleep $TIMING
done
