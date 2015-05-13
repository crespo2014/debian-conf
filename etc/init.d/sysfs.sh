#! /bin/sh
### BEGIN INIT INFO
# Provides:         run lock
# Required-Start:
# Required-Stop:
# Should-Start:      
# Default-Start:     S
# Default-Stop:
# Short-Description: Mount proc virtual file systems.
# Description:       Mount initial set of virtual filesystems the kernel
### END INIT INFO

PATH=/sbin:/bin
. /lib/init/vars.sh
. /lib/init/tmpfs.sh

. /lib/lsb/init-functions
. /lib/init/mount-functions.sh

# -n means do not write into etc/mtab because is read-only

#mount -n -t proc proc /proc "-onodev,noexec,nosuid"
#mount -n -t sysfs sys /sys "-onodev,noexec,nosuid"

MNTMODE=mount_noupdate

case "$1" in
    start)
        mount_run "$MNTMODE"
        mount_lock "$MNTMODE"
		mount_shm "$MNTMODE"
		mount_tmp "$MNTMODE"
        #domount "$MNTMODE" sysfs "" /sys sysfs "-onodev,noexec,nosuid"
		#domount "$MNTMODE" proc "" /proc proc "-onodev,noexec,nosuid"
        ;;
    stop)
        ;;
    restart|force-reload)
        ;;
    status)
        ;;
    *)
        ;;
esac


