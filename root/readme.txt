X server tips

X -display <name>
-bg <color> bagkground color

X -configure  #test current configuration
X -config <>  use a specific config file

startx
prepare environment and arguments then call xinit <client data> -- < server data > 

Better to call xinit with all arguments
xinit http://cgit.freedesktop.org/xorg/app/xinit/tree/xinit.c

xinit /root/.xinitrc -- /etc/X11/xinit/xserverrc :0 -auth /tmp/.xauth
xinit  --  :0 -auth /tmp/.xauth

e4rat

path CMakeList.txt to use boost 3
path filename to .string(); 

apt-get install libboost-all-dev

cmake   \
libaudit \
libaudit1 \
libboost-system-dev \
libboost-signals1.55-dev\
libboost-filesystem1.55-dev \
libboost-system-dev \
libboost-signals1.55-dev \
libboost-filesystem1.55-dev\
libboost-regex1.55-dev\
e2fsck-static e2fsprogs \
e2fslibs-dev\
libaudit-dev\
libauparse-dev libauparse0


cmake -DBOOST_FILESYSTEM_VERSION=3

make BOOST_FILESYSTEM_VERSION=3