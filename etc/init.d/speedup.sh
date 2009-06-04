#! /bin/bash
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

LEVEL2="hostname.sh& udev& mountall.sh mountdevsubfs.sh dbus& slim" 
LEVEL1="hostname.sh& mountkernfs.sh udev checkroot.sh checkfs.sh udev mountdevsubfs.sh console-setup" 

mount -t proc proc /proc "-onodev,noexec,nosuid"

cat /proc/deferred_initcalls &> /dev/null &

# read env
if grep -qw single /proc/cmdline; then
SCRIPTS=$LEVEL1
else
SCRIPTS=$LEVEL2
fi

echo "Starting .... $SCRIPTS"

case "$1" in
  start|"")
	mount -t sysfs sys /sys "-onodev,noexec,nosuid"
	for script in $SCRIPTS
	do
	cmd=${script::-1}
	cmd_end=${script: -1}
	if [ "$cmd_end" != "&" ]; then
	  cmd=$cmd$cmd_end
	  cmd_end=
	fi
	/etc/init.d/$cmd start $cmd_end
	done
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
