#!/bin/sh
#WMFS status.sh example file

TIMING=10

statustext()
{
     echo "status default `date`" > /tmp/wmfs-$DISPLAY.fifo
}

while true;
do
    statustext
    sleep $TIMING
done
