#!/bin/sh
#WMFS status.sh example file
#Will be executed if put in ~/.config/wmfs/
#Timing adjustable in wmfsrc (misc -> status_timing)

statustext()
{
     wmfs -s "`date`"
}

while true; do statustext; sleep 10; done
