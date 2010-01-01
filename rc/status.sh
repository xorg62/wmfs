#!/bin/sh
#WMFS status.sh example file
#Will be executed if put in ~/.config/wmfs/
#Timing adjustable in wmfsrc (misc -> status_timing)

statustext()
{
     local DATE=`date`

     wmfs -s "$DATE"
}

statustext
