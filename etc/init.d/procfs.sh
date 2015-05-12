#! /bin/sh
### BEGIN INIT INFO
# Provides:          procfs sysfs run lock
# Required-Start:
# Required-Stop:
# Should-Start:      glibc
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

mount_run
mount_lock

# -n means do not write into etc/mtab because is read-only

mount -n -t proc proc /proc "-onodev,noexec,nosuid"
domount -n -t sysfs sys /sys "-onodev,noexec,nosuid"
