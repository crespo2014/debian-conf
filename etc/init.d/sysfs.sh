#! /bin/sh
### BEGIN INIT INFO
# Provides:          sysfs 
# Required-Start:
# Required-Stop:
# Should-Start:      glibc
# Default-Start:     S
# Default-Stop:
# Short-Description: Mount sysfs virtual file systems.
# Description:       Mount initial set of virtual filesystems the kernel
### END INIT INFO

PATH=/sbin:/bin
. /lib/init/vars.sh
. /lib/init/tmpfs.sh

. /lib/lsb/init-functions
. /lib/init/mount-functions.sh

# -n means do not write into etc/mtab because is read-only

domount -n -t sysfs sys /sys "-onodev,noexec,nosuid"

