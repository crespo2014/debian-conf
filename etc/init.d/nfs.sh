#!/bin/sh
### BEGIN INIT INFO
# Provides:          nfs
# Required-Start:    
# Required-Stop:     sendsigs $desktop $network networking wicd network-manager dbus udev 
# Default-Start:     
# Default-Stop:      0 1 6
# Short-Description: Unmount network file system
# Description:       
### END INIT INFO

# Author: Lester <foobar@baz.org>

DESC="Description of the service"
DAEMON=/usr/sbin/daemonexecutablename

PATH=/sbin:/usr/sbin:/bin:/usr/bin

do_stop () {
  DIRS=
  while read -r DEV MTPT FSTYPE REST
  do
    case "$FSTYPE" in
      nfs|nfs4|smbfs|ncp|ncpfs|cifs|coda|ocfs2|gfs|ceph)
	DIRS="$MTPT $DIRS"
	;;
    esac
  done < /proc/mounts
  [ "$DIRS" != "" ] && umount -f -l $DIRS
}

case "$1" in
  start|status)
	# No-op
	;;
  restart|reload|force-reload)
	echo "Error: argument '$1' not supported" >&2
	exit 3
	;;
  stop|"")
	do_stop
	;;
  *)
	echo "Usage: nfs.sh [start|stop]" >&2
	exit 3
	;;
esac

:
