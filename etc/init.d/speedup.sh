#! /bin/sh
### BEGIN INIT INFO
# Provides:		modules proc sysfs         
# Required-Start:	mountkernfs
# Required-Stop:
# Should-Start:      
# Default-Start:     S
# Default-Stop:
# Short-Description: 
# Description:  SpeedUp boot speed by doing early initialization of some task
### END INIT INFO

PATH=/sbin:/bin

. /lib/init/vars.sh
. /lib/lsb/init-functions
# read env
if grep -qw single /proc/cmdline; then
RUNLEVEL=1
else
RUNLEVEL=2
fi

case "$1" in
  start|"")
	cat /proc/deferred_initcalls > /dev/null &
	if [ "$RUNLEVEL" = 2 ]; then
	mount /home
	/etc/init.d/early-readahead start &
	/etc/init.d/slim start &
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
