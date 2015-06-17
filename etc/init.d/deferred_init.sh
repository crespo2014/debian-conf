#! /bin/bash
### BEGIN INIT INFO
# Provides:			modules          
# Required-Start:       $all		
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
    cat /proc/deferred_initcalls
    udevadm trigger
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
