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
