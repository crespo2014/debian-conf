#! /bin/sh
### BEGIN INIT INFO
# Provides:          mountnfs
# Required-Start:    $network network-manager
# Required-Stop:
# Should-Start:      $network $portmap nfs-common  udev-mtab
# Default-Start:     1 2 3 4 5  
# Default-Stop:      0 6
# Short-Description: Wait for network file systems to be mounted
# Description:       Network file systems are mounted by
#                    /etc/network/if-up.d/mountnfs in the background
#                    when interfaces are brought up; this script waits
#                    for them to be mounted before carrying on.
### END INIT INFO
