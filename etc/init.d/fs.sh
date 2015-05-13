#! /bin/sh
### BEGIN INIT INFO
# Provides:         rootfs
# Required-Start:
# Required-Stop:
# Should-Start:      
# Default-Start:     S
# Default-Stop:
# Short-Description: Mount all file system 
### END INIT INFO

PATH=/sbin:/bin

mount /home
mount /mnt/data

