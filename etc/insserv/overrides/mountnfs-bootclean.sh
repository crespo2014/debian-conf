#! /bin/sh
### BEGIN INIT INFO
# Provides:          mountnfs-bootclean
# Required-Start:    $local_fs mountnfs
# Required-Stop:
# Default-Start:     1 2 3 4 5
# Default-Stop:      0 6
# X-Start-Before:    bootmisc
# Short-Description: bootclean after mountnfs.
# Description:       Clean temporary filesystems after
#                    network filesystems have been mounted.
### END INIT INFO

