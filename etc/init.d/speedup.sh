#! /bin/bash
### BEGIN INIT INFO
# Provides:	      init procfs sysfs	         
# Required-Start:	
# Required-Stop:
# Should-Start:      
# Default-Start:     S
# Default-Stop:
# Short-Description: It provides udev dbus 
# Description:  SpeedUp boot speed by doing early initialization of some task
### END INIT INFO

PATH=/sbin:/bin:/usr/bin

. /lib/init/vars.sh
. /lib/init/tmpfs.sh
. /lib/lsb/init-functions
. /lib/init/mount-functions.sh

VERBOSE=no

domount mount_noupdate proc "" /proc proc "-onodev,noexec,nosuid"


if grep -qw safe /proc/cmdline; then
BACKG=0
echo "Safe Mode Initialization ... "
else
BACKG=1
fi

# using W to wait for all background to finish  
# using # to run in background not wait
# using % to disable

if grep -qw single /proc/cmdline; then
SCRIPTS=" \
 hostname.sh \
 mountkernfs.sh \
 deferred_init.sh \
 udev \
 mountdevsubfs.sh \
 hwclock.sh \
 checkroot.sh \
 checkfs.sh \
 checkroot-bootclean.sh \
 mountall.sh \
 mountall-bootclean.sh \
 kmod \
 udev-mtab \
 x11-common \
 mountnfs.sh \
 mountnfs-bootclean.sh \
 bootmisc.sh \
 motd \
 dbus \
 network-manager \
 ssh \
 wicd \
 console-setup \
 single \
 "
else
# Desktop mode do initial task
LOGFILE=/var/log/init.log
touch $LOGFILE
LOGFILE=/dev/kmsg
/root/cinit &>>$LOGFILE
cat /proc/deferred_initcalls &
#mount / -o remount,noatime,nodiratime
#domount mount_noupdate sysfs "" /sys sysfs "-onodev,noexec,nosuid"
#mount_run mount_noupdate
#mount_lock mount_noupdate
#mount_tmp mount_noupdate
#mount_shm mount_noupdate
#mount /home
#mount /mnt/data
/sbin/bootchartd start &>>$LOGFILE
/etc/init.d/early-readahead start &>>$LOGFILE &
#/etc/init.d/hostname.sh start &>>$LOGFILE

/etc/init.d/mountdevsubfs.sh start &>>$LOGFILE &

#embedded udev script
#echo > /sys/kernel/uevent_helper
#start-stop-daemon -b -S --exec /sbin/udevd -- &>>$LOGFILE & 
#udevd --daemon &
#/etc/init.d/x11-common start &>>$LOGFILE

#su -c 'startx' lester &>>$LOGFILE &

#/etc/init.d/nodm start &>>$LOGFILE
/etc/init.d/dbus start &>>$LOGFILE & 
udevadm trigger --action=add &>>$LOGFILE &

SCRIPTS="\
 
" 
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
      if [ "$cmd_end" = "&" ]; then
        if [ -x /etc/init.d/${script::-1} ]; then
          /etc/init.d/${script::-1} start &>/dev/kmsg &
          pid="$pid $!"
        fi
      else
      if [ "$cmd_end" = "#" ]; then
        if [ -x /etc/init.d/${script::-1} ]; then
          /etc/init.d/${script::-1} start &>/dev/kmsg &
        fi
      else
      if [ "$cmd_end" = "%" ]; then
        echo "$script OFF"
      else
        if [ -x /etc/init.d/$script ]; then 
          /etc/init.d/$script start &>/dev/kmsg
        fi
      fi
      fi
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
