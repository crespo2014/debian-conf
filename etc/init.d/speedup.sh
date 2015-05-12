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

LOG=/dev/kmsg

function init_1() {
  SCRIPTS="hostname.sh mountkernfs.sh udev keyboard-setup mountdevsubfs.sh hdparm hwclock.sh checkroot.sh checkroot-bootclean.sh kbd console-setup alsa-utils pppd-dns procps udev-mtab urandom x11-common networking rpcbind mountnfs.sh mountnfs-bootclean.sh bootmisc.sh motd bootlogs network-manager ssh single"
  for script in $SCRIPTS
  do
    /etc/init.d/$script start &>>$LOG
  done
  cat /proc/deferred_initcalls &>>$LOG &
}

function init_2() {
  mount -t proc proc /proc "-onodev,noexec,nosuid"
  mount -t sysfs sys /sys "-onodev,noexec,nosuid"
  cat /proc/deferred_initcalls &> /dev/null &
  SCRIPTS="hostname.sh early-readahead udev& mountall.sh x11-common mountdevsubfs.sh dbus& slim W stop-readahead-fedora acct urandom acpid atd cron dbus exim4 motd rsync bootlogs networking network-manager ssh saned rpcbind rc.local rmnologin " 
# using W to wait for all background to finish  
  pid=
  for script in $SCRIPTS
  do
    cmd_end=${script: -1}
	if [ "$script" = "W" ]; then
	  for id in $pid
	  do
	    wait $id
	  done
	  pid=
	else	
      if [ "$cmd_end" != "&" ]; then
	    /etc/init.d/$script start &>>$LOG
	  else
	    /etc/init.d/${script::-1} start &>>$LOG &
	    pid="$pid $!"
      fi 
	fi
  done
}

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
