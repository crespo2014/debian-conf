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
if grep -qw single /proc/cmdline; then
SCRIPTS=" \
 hostname.sh \
 mountkernfs.sh \
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
 kmod \
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
# Desktop mode do initial task
domount mount_noupdate sysfs "" /sys sysfs "-onodev,noexec,nosuid"
mount_run mount_noupdate
mount_lock mount_noupdate
mount_tmp mount_noupdate
mount_shm mount_noupdate

#mount /home
#mount /mnt/data

SCRIPTS="\
 hostname.sh& \
 fs.sh& \
 early-readahead \
 later-readahead \
 udev# \
 deferred_init.sh# \
 mountdevsubfs.sh# \
 hdparm# \
 kbd# \
 x11-common& \
 W \
 procps# \
 dbus# \
 slim \
 urandom \
 hwclock.sh \
 networking \
 network-manager \
 stop-readahead-fedora \
 acct \
 acpid \
 atd \
 cron \
 motd \
 wicd \
 ntp \
 ssh \
 saned \
 rpcbind \
 rc.local \
 rmnologin \
 bootchart \
 bootchart-done#" 
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
        [ -x /etc/init.d/${script::-1} ] && /etc/init.d/${script::-1} start &>/dev/kmsg &
        pid="$pid $!"
      else
      if [ "$cmd_end" = "#" ]; then
        [ -x /etc/init.d/${script::-1} ] && /etc/init.d/${script::-1} start &>/dev/kmsg &
      else
        [ -x /etc/init.d/$script ] && /etc/init.d/$script start &>/dev/kmsg      
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
