#! /bin/bash
### BEGIN INIT INFO
# Provides:	      init procfs sysfs	         
# Required-Start:	
# Required-Stop:
# Should-Start:      
# Default-Start:     S
# Default-Stop:
# Short-Description: 
# Description:  SpeedUp boot speed by doing early initialization of some task
### END INIT INFO

PATH=/sbin:/bin:/usr/bin
#LOG=/dev/kmsg &>/dev/msg  &>>file
LOG=

/etc/init.d/hostname.sh start 
/etc/init.d/mountkernfs.sh start 

if grep -qw safe /proc/cmdline; then
BACKG=0
echo "Safe Mode Initialization ... "
else
BACKG=1
fi

# using W to wait for all background to finish  
if grep -qw single /proc/cmdline; then
SCRIPTS=" \
 deferred_init.sh \
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
 wicd \
 single \
 "
else
SCRIPTS="\
 early-readahead \
 udev& \
 later-readahead& \
 mountdevsubfs.sh \
 hdparm& \
 mountall.sh& \
 kbd& \
 console-setup& \
 urandom \
 x11-common \
 procps& \
 dbus& \
 W \
 slim& \
 W \
 deferred_init.sh \
 hwclock.sh \
 stop-readahead-fedora \
 acct \
 urandom \
 acpid \
 atd \
 cron \
 dbus \
 exim4 \
 motd \
 rsync \
 bootlogs \
 networking \
 network-manager \
 wicd \
 stop-readahead-fedora \
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
    echo $script 
    cmd_end=${script: -1}
	if [ "$script" = "W" ]; then
	  for id in $pid
	  do
	    wait $id
	  done
	  pid=
	else	
      if [ "$cmd_end" != "&" ] || [ "$BACKG" = "0" ] ; then
	    [ -x /etc/init.d/$script ] && /etc/init.d/$script start $LOG
	  else
	    [ -x /etc/init.d/${script::-1} ] && /etc/init.d/${script::-1} start $LOG &
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
