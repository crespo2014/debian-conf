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

# using W to wait for all background to finish  
if grep -qw single /proc/cmdline; then
SCRIPTS="hostname.sh \
 mountkernfs.sh \
 udev \
 procps \
 keyboard-setup \
 mountdevsubfs.sh \
 hdparm \
 hwclock.sh \
 checkroot.sh \
 checkfs.sh \
 checkroot-bootclean.sh \
 mountall.sh \
 mountall-bootclean.sh \
 kbd \
 console-setup \
 alsa-utils \
 pppd-dns \
 udev-mtab \
 urandom \
 x11-common \
 networking \
 rpcbind \
 mountnfs.sh \
 mountnfs-bootclean.sh \
 bootmisc.sh \
 motd \
 bootlogs \
 dbus \
 network-manager \
 ssh \
 single \
 deferred_init.sh& "
else
SCRIPTS="hostname.sh \
 procfs.sh \
 deferred_init.sh& \
 early-readahead \
 udev& \
 mountall.sh& \
 W \
 later-readahead \
 mtab.sh& \
 mountdevsubfs.sh& \
 hwclock.sh& \
 hdparam& \
 deferred_init.sh& \
 x11-common& \
 dbus \
 slim \
 stop-readahead-fedora \
 acct \
 urandom \
 acpid \
 atd \
 cron \
 dbus \
 exim4 \
 motd rsync \
 bootlogs \
 networking \
 network-manager \
 ssh \
 saned \
 rpcbind \
 rc.local \
 rmnologin \
 bootchart \
 bootchart-done" 
fi

function init() {  
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
	init
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
