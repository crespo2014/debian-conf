#! /bin/sh
### BEGIN INIT INFO
# Provides:          umountnfs
# Required-Start:
# Required-Stop:     umountfs $network network-manager $desktop
# Should-Stop:       $network $portmap nfs-common
# Default-Start:
# Default-Stop:      0 6
# Short-Description: Unmount all network filesystems except the root fs.
# Description:       Also unmounts all virtual filesystems (proc,
#                    devpts, usbfs, sysfs) that are not mounted at the
#                    top level.
### END INIT INFO
