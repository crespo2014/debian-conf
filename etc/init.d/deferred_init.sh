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
    (sleep 5;cat /proc/deferred_initcalls;udevadm trigger;sleep 15;INIT_PROCESS="yes"; /sbin/bootchartd stop;) &
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
