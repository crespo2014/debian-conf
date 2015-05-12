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

function init_1() {
  SCRIPTS="hostname.sh mountkernfs.sh udev checkroot.sh checkfs.sh udev mountdevsubfs.sh muntall.sh console-setup"
  for script in $SCRIPTS
  do
    /etc/init.d/$script start
  done
  cat /proc/deferred_initcalls &
}

function init_2() {
  mount -t proc proc /proc "-onodev,noexec,nosuid"
  mount -t sysfs sys /sys "-onodev,noexec,nosuid"
  cat /proc/deferred_initcalls &> /dev/null &
  SCRIPTS="hostname.sh udev& mountall.sh mountdevsubfs.sh dbus& slim" 
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
}
echo "Starting .... $SCRIPTS"

case "$1" in
  start|"")
  	if grep -qw single /proc/cmdline; then
  	init_1
  	else
  	init_2
  	fi
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
