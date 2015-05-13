#! /bin/sh
### BEGIN INIT INFO
# Provides:         desktop
# Required-Start:
# Required-Stop:
# Should-Start:      
# Default-Start:     
# Default-Stop:
# Short-Description: Start X Server with auto login user
### END INIT INFO

# http://karuppuswamy.com/wordpress/2010/09/26/how-to-fix-x-user-not-authorized-to-run-the-x-server-aborting/
# https://wiki.archlinux.org/index.php/Xinitrc
#https://wiki.gentoo.org/wiki/X_without_Display_Manager
#  startx /usr/bin/startxfce4
#  startx -- vt7
# startxfce4 -- vt7
# /sbin/mingetty --autologin username --loginprog=/usr/local/sbin/x11login --noclear tty8 38400
# /bin/su username -l -c /usr/bin/xinit -- VT08
# /etc/X11/Xwrapper.config 
# dpkg-reconfigure x11-common
# http://jeffhoogland.blogspot.ie/2011/12/howto-get-right-to-x-with-no-display.html


/bin/login -f lester tty1 /dev/tty1 2>&1