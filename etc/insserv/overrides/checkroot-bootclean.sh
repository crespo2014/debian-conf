#! /bin/sh
### BEGIN INIT INFO
# Provides:          checkroot-bootclean
# Required-Start:    
# Required-Stop:
# Default-Start:     
# Default-Stop:
# X-Start-Before:    bootmisc
# Short-Description: bootclean after checkroot.
# Description:       Clean temporary filesystems after
#                    the root filesystem has been mounted.
#                    At this point, directories which may be
#                    masked by future mounts may be cleaned.
### END INIT INFO
