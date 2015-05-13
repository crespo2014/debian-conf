#! /bin/sh
### BEGIN INIT INFO
# Provides:         rootfs
# Required-Start:
# Required-Stop:
# Should-Start:      
# Default-Start:     
# Default-Stop:
# Short-Description: Mount all file system 
### END INIT INFO

#PATH=/sbin:/bin
. /lib/init/vars.sh
. /lib/init/tmpfs.sh

. /lib/lsb/init-functions
. /lib/init/mount-functions.sh

MNTMODE=mount_noupdate

case "$1" in
    start)
#      mount_run mount_noupdate
#      mount_lock mount_noupdate
#      mount_shm mount_noupdate
#      mount_tmp mount_noupdate
      mount /home
      mount /mnt/data
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

