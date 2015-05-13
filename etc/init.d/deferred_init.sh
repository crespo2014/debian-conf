#! /bin/sh
### BEGIN INIT INFO
# Provides:			modules          
# Required-Start:	
# Required-Stop:
# Should-Start:      
# Default-Start:     
# Default-Stop:
# Short-Description: 
# Description:       
### END INIT INFO

PATH=/sbin:/bin

case "$1" in
  start|"")
	cat /proc/deferred_initcalls
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
