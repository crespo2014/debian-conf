#! /bin/bash
### BEGIN INIT INFO
# Provides:			modules          
# Required-Start:       bootlogs		
# Required-Stop:	
# Should-Start:      
# Default-Start:	S 1 2 3 4 5  
# Default-Stop:
# Short-Description: 
# Description:       
### END INIT INFO

PATH=/sbin:/bin

case "$1" in
  start|"")
    single=$(grep -wo single /proc/cmdline)
    if [ "$runlevel" == "S" ] && [ "$single" == "" ]; then
    	exit
    fi
	#give some time to display manager
	#[ "$runlevel" == "S" ] || sleep 10
    cat /proc/deferred_initcalls
    udevadm trigger
	# log some bootchart data before close it
	if [ "$runlevel" != "S" ]; then
	 (sleep 30;INIT_PROCESS="yes"; /sbin/bootchartd stop;) &
        fi
	;;
  restart|reload|force-reload)
	;;
  stop)
	# No-op
	;;
  status)
	;;
  *)
	;;
esac

:
