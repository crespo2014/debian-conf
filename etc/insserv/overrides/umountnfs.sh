#! /bin/sh
### BEGIN INIT INFO
# Provides:          umountnfs
# Required-Start:
# Required-Stop:     dbus slim umountfs wicd nodm network-manager networking $desktop $network
# Should-Stop:        
# Default-Start:
# Default-Stop:      0 6
# Short-Description: Unmount all network filesystems except the root fs.
# Description:       Also unmounts all virtual filesystems (proc,
#                    devpts, usbfs, sysfs) that are not mounted at the
#                    top level.
### END INIT INFO
