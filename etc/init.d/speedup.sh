#! /bin/sh
### BEGIN INIT INFO
# Provides:		modules proc sysfs desktop         
# Required-Start:	
# Required-Stop:
# Should-Start:      
# Default-Start:     S
# Default-Stop:
# Short-Description: 
# Description:  SpeedUp boot speed by doing early initialization of some task
### END INIT INFO

PATH=/sbin:/bin:/usr/bin

# read env
if grep -qw single /proc/cmdline; then
RUNLEVEL=1
else
RUNLEVEL=2
fi

case "$1" in
  start|"")
	/etc/init.d/hostname.sh start &
	if [ "$RUNLEVEL" = 2 ]; then
	mount -t proc "" /proc "-onodev,noexec,nosuid"
	/etc/init.d/early-readahead start &
	mount -t sysfs "" /sys "-onodev,noexec,nosuid"
	/etc/init.d/udev start &
	/etc/init.d/mountall.sh 
	/etc/init.d/mountdevsubfs.sh start 
	/etc/init.d/dbus start &
	/etc/init.d/slim start
	else
	/etc/init.d/mountkernfs.sh start
	/etc/init.d/checkroot.sh start
	/etc/init.d/checkfs.sh start
	fi
	cat /proc/deferred_initcalls &> /dev/null &
	exit 0
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
