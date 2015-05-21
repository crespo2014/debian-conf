#!/bin/bash
#
# http://www.linuxfromscratch.org/blfs/view/7.7/x/x7driver.html
#
# apt-get install autoconf make cmake
[ -d xorg-src ] || mkdir xorg-src
cd xorg-src
export XORG_PREFIX="/opt/X11"
export XORG_CONFIG="--prefix=$XORG_PREFIX --sysconfdir=/etc \
    --localstatedir=/var "
#--disable-static
	
PATH=$PATH:$XORG_PREFIX/bin
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$XORG_PREFIX/lib/pkgconfig:$XORG_PREFIX/share/pkgconfig  
LIBRARY_PATH=$LIBRARY_PATH:$XORG_PREFIX/lib             
C_INCLUDE_PATH=$C_INCLUDE_PATH:$XORG_PREFIX/include         
CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$XORG_PREFIX/include         

ACLOCAL='aclocal -I $XORG_PREFIX/share/aclocal'
export PATH PKG_CONFIG_PATH ACLOCAL LIBRARY_PATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH	

if [ 1 = 1 ]; then	
cat > /etc/profile.d/xorg.sh << "EOF"
XORG_PREFIX="/opt/X11"
XORG_CONFIG="--prefix=$XORG_PREFIX --sysconfdir=/etc --localstatedir=/var --disable-static"
export XORG_PREFIX XORG_CONFIG
EOF
cat >> /etc/profile.d/xorg.sh << "EOF"

PATH=$PATH:$XORG_PREFIX/bin
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$XORG_PREFIX/lib/pkgconfig:$XORG_PREFIX/share/pkgconfig  
LIBRARY_PATH=$LIBRARY_PATH:$XORG_PREFIX/lib             
C_INCLUDE_PATH=$C_INCLUDE_PATH:$XORG_PREFIX/include         
CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$XORG_PREFIX/include         

ACLOCAL='aclocal -I $XORG_PREFIX/share/aclocal'

export PATH PKG_CONFIG_PATH ACLOCAL LIBRARY_PATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH
EOF
chmod 644 /etc/profile.d/xorg.sh	

echo "$XORG_PREFIX/lib" >> /etc/ld.so.conf
sed "s@/usr/X11R6@$XORG_PREFIX@g" -i /etc/man_db.conf
ln -sf $XORG_PREFIX/share/X11 /usr/share/X11
ln -sf $XORG_PREFIX /usr/X11R6
install -v -m755 -d $XORG_PREFIX &&
install -v -m755 -d $XORG_PREFIX/lib &&
ln -sf lib $XORG_PREFIX/lib64
fi

#extract the required file and got to folder
function extract()
{
  url=$1
  fname=${url##*/}
  name=${fname%-*}
  [ -d $name ] || mkdir ${name}
  [ -f $fname ] || wget $url
  [ $? = 0 ] && [ -f $fname ] && tar --strip-components=1 -xf $fname -C $name
  cd $name
}
# download build and isntall file received as parameter.
function install()
{
  url=$1
  fname=${url##*/}
  name=${fname%-*}
  [ -d $name ] || mkdir ${name}
  [ -f $fname ] || wget $url
  [ $? = 0 ] && [ -f $fname ] && tar --strip-components=1 -xf $fname -C $name
  if [ $? = 0 ]; then
    pushd $name
    ./configure $XORG_CONFIG
	make
    make install
    popd	
  fi
	
}

install "http://xorg.freedesktop.org/releases/individual/util/util-macros-1.19.0.tar.bz2"

http_root="http://xorg.freedesktop.org/releases/individual/proto/"
files="bigreqsproto-1.1.2.tar.bz2  \
 compositeproto-0.4.2.tar.bz2  \
 damageproto-1.2.1.tar.bz2  \
 dmxproto-2.3.1.tar.bz2  \
 dri2proto-2.8.tar.bz2  \
 dri3proto-1.0.tar.bz2  \
 fixesproto-5.0.tar.bz2  \
 fontsproto-2.1.3.tar.bz2  \
 glproto-1.4.17.tar.bz2 \
 inputproto-2.3.1.tar.bz2  \
 kbproto-1.0.6.tar.bz2  \
 presentproto-1.0.tar.bz2  \
 randrproto-1.4.0.tar.bz2  \
 recordproto-1.14.2.tar.bz2  \
 renderproto-0.11.1.tar.bz2  \
 resourceproto-1.2.0.tar.bz2  \
 scrnsaverproto-1.2.2.tar.bz2  \
 videoproto-2.3.2.tar.bz2  \
 xcmiscproto-1.2.2.tar.bz2  \
 xextproto-7.3.0.tar.bz2  \
 xf86bigfontproto-1.2.0.tar.bz2  \
 xf86dgaproto-2.1.tar.bz2  \
 xf86driproto-2.1.1.tar.bz2  \
 xf86vidmodeproto-2.3.1.tar.bz2  \
 xineramaproto-1.2.1.tar.bz2  \
 xproto-7.0.27.tar.bz2 " 
for line in $files
do
  install "$http_root/$line"
done

files="http://xorg.freedesktop.org/releases/individual/lib/libXau-1.0.8.tar.bz2 \
 http://xorg.freedesktop.org/releases/individual/lib/libXdmcp-1.1.1.tar.bz2 \
 http://xcb.freedesktop.org/dist/xcb-proto-1.11.tar.bz2 "
 
extract "http://xcb.freedesktop.org/dist/libxcb-1.11.tar.bz2"
sed -i "s/pthread-stubs//" configure &&
./configure $XORG_CONFIG    \
            --enable-xinput \
            --docdir='${datadir}'/doc/libxcb-1.11 &&
make
cd ..

http_root=http://xorg.freedesktop.org/releases/individual/lib/
files="\
 xtrans-1.3.5.tar.bz2       \
 libX11-1.6.2.tar.bz2       \
 libXext-1.3.3.tar.bz2      \
 libFS-1.0.6.tar.bz2        \
 libICE-1.0.9.tar.bz2       \
 libSM-1.2.2.tar.bz2        \
 libXScrnSaver-1.2.2.tar.bz2\
 libXt-1.1.4.tar.bz2        \
 libXmu-1.1.2.tar.bz2       \
 libXpm-3.5.11.tar.bz2      \
 libXaw-1.0.12.tar.bz2      \
 libXfixes-5.0.1.tar.bz2    \
 libXcomposite-0.4.4.tar.bz2\
 libXrender-0.9.8.tar.bz2   \
 libXcursor-1.1.14.tar.bz2  \
 libXdamage-1.1.4.tar.bz2   \
 libfontenc-1.1.2.tar.bz2   \
 libXfont-1.5.0.tar.bz2     \
 libXft-2.3.2.tar.bz2       \
 libXi-1.7.4.tar.bz2        \
 libXinerama-1.1.3.tar.bz2  \
 libXrandr-1.4.2.tar.bz2    \
 libXres-1.0.7.tar.bz2      \
 libXtst-1.2.2.tar.bz2      \
 libXv-1.0.10.tar.bz2       \
 libXvMC-1.0.8.tar.bz2      \
 libXxf86dga-1.1.4.tar.bz2  \
 libXxf86vm-1.1.3.tar.bz2   \
 libdmx-1.1.3.tar.bz2       \
 libpciaccess-0.13.3.tar.bz2\
 libxkbfile-1.0.8.tar.bz2   \
 libxshmfence-1.2.tar.bz2   "
for line in $files
do
  extract "$http_root/$line"
  packagedir=(dirname `pwd`)
  case $packagedir in
    libXfont* )
      ./configure $XORG_CONFIG --disable-devel-docs
    ;;
    libXt* )
      ./configure $XORG_CONFIG \
                  --with-appdefaultdir=/etc/X11/app-defaults
    ;;
    * )
      ./configure $XORG_CONFIG
    ;;
  esac
  make
  make check 2>&1 | tee ../$packagedir-make_check.log
  make install
  /sbin/ldconfig
  cd ..
done 

install http://xcb.freedesktop.org/dist/xcb-util-0.4.0.tar.bz2
install http://xcb.freedesktop.org/dist/xcb-util-image-0.4.0.tar.bz2
install http://xcb.freedesktop.org/dist/xcb-util-keysyms-0.4.0.tar.bz2
install http://xcb.freedesktop.org/dist/xcb-util-renderutil-0.3.9.tar.bz2
install http://xcb.freedesktop.org/dist/xcb-util-wm-0.4.1.tar.bz2

#wget http://www.linuxfromscratch.org/patches/blfs/7.7/MesaLib-10.4.5-add_xdemos-1.patch 
extract ftp://ftp.freedesktop.org/pub/mesa/10.4.5/MesaLib-10.4.5.tar.bz2
#patch -Np1 -i ../MesaLib-10.4.5-add_xdemos-1.patch

autoreconf -f -i &&
./configure CFLAGS='-O2' CXXFLAGS='-O2'    \
            --prefix=$XORG_PREFIX          \
            --sysconfdir=/etc              \
            --enable-texture-float         \
            --enable-gles1                 \
            --enable-gles2                 \
            --enable-osmesa                \
            --enable-xa                    \
            --enable-gbm                   \
            --enable-glx-tls               \
            --with-egl-platforms="drm,x11" \
            --with-gallium-drivers="nouveau,r300,r600,radeonsi,svga,swrast" &&
make
make install
make -C xdemos DEMOS_PREFIX=$XORG_PREFIX
make -C xdemos DEMOS_PREFIX=$XORG_PREFIX install

install http://xorg.freedesktop.org/archive/individual/data/xbitmaps-1.1.1.tar.bz2

http_root=http://xorg.freedesktop.org/releases/individual/app/
files="\
 bdftopcf-1.0.5.tar.bz2     \
 iceauth-1.0.7.tar.bz2      \
 luit-1.1.1.tar.bz2         \
 mkfontdir-1.0.7.tar.bz2    \
 mkfontscale-1.1.2.tar.bz2  \
 sessreg-1.1.0.tar.bz2      \
 setxkbmap-1.3.0.tar.bz2    \
 smproxy-1.0.5.tar.bz2      \
 x11perf-1.5.4.tar.bz2      \
 xauth-1.0.9.tar.bz2        \
 xbacklight-1.2.1.tar.bz2   \
 xcmsdb-1.0.4.tar.bz2       \
 xcursorgen-1.0.6.tar.bz2   \
 xdpyinfo-1.3.1.tar.bz2     \
 xdriinfo-1.0.4.tar.bz2     \
 xev-1.2.1.tar.bz2          \
 xgamma-1.0.5.tar.bz2       \
 xhost-1.0.6.tar.bz2        \
 xinput-1.6.1.tar.bz2       \
 xkbcomp-1.3.0.tar.bz2      \
 xkbevd-1.1.3.tar.bz2       \
 xkbutils-1.0.4.tar.bz2     \
 xkill-1.0.4.tar.bz2        \
 xlsatoms-1.1.1.tar.bz2     \
 xlsclients-1.1.3.tar.bz2   \
 xmessage-1.0.4.tar.bz2     \
 xmodmap-1.0.8.tar.bz2      \
 xpr-1.0.4.tar.bz2          \
 xprop-1.2.2.tar.bz2        \
 xrandr-1.4.3.tar.bz2       \
 xrdb-1.1.0.tar.bz2         \
 xrefresh-1.0.5.tar.bz2     \
 xset-1.2.3.tar.bz2         \
 xsetroot-1.1.1.tar.bz2     \
 xvinfo-1.1.2.tar.bz2       \
 xwd-1.0.6.tar.bz2          \
 xwininfo-1.1.3.tar.bz2     \
 xwud-1.0.4.tar.bz2         "
for line in $files
do
  extract "$http_root/$line"
  packagedir=(dirname `pwd`)
  case $packagedir in
    luit-[0-9]* )
      line1="#ifdef _XOPEN_SOURCE"
      line2="#  undef _XOPEN_SOURCE"
      line3="#  define _XOPEN_SOURCE 600"
      line4="#endif"
 
      sed -i -e "s@#ifdef HAVE_CONFIG_H@$line1\n$line2\n$line3\n$line4\n\n&@" sys.c
      unset line1 line2 line3 line4
    ;;
  esac
  ./configure $XORG_CONFIG
  make
  make install  
  cd ..
done  

install http://xorg.freedesktop.org/archive/individual/data/xcursor-themes-1.0.4.tar.bz2

http_root="http://xorg.freedesktop.org/releases/individual/font/"
files="\
 font-util-1.3.0.tar.bz2                        \
 encodings-1.0.4.tar.bz2                        \
 font-adobe-100dpi-1.0.3.tar.bz2                \
 font-adobe-75dpi-1.0.3.tar.bz2                 \
 font-adobe-utopia-100dpi-1.0.4.tar.bz2         \
 font-adobe-utopia-75dpi-1.0.4.tar.bz2          \
 font-adobe-utopia-type1-1.0.4.tar.bz2          \
 font-alias-1.0.3.tar.bz2                       \
 font-arabic-misc-1.0.3.tar.bz2                 \
 font-bh-100dpi-1.0.3.tar.bz2                   \
 font-bh-75dpi-1.0.3.tar.bz2                    \
 font-bh-lucidatypewriter-100dpi-1.0.3.tar.bz2  \
 font-bh-lucidatypewriter-75dpi-1.0.3.tar.bz2   \
 font-bh-ttf-1.0.3.tar.bz2                      \
 font-bh-type1-1.0.3.tar.bz2                    \
 font-bitstream-100dpi-1.0.3.tar.bz2            \
 font-bitstream-75dpi-1.0.3.tar.bz2             \
 font-bitstream-type1-1.0.3.tar.bz2             \
 font-cronyx-cyrillic-1.0.3.tar.bz2             \
 font-cursor-misc-1.0.3.tar.bz2                 \
 font-daewoo-misc-1.0.3.tar.bz2                 \
 font-dec-misc-1.0.3.tar.bz2                    \
 font-ibm-type1-1.0.3.tar.bz2                   \
 font-isas-misc-1.0.3.tar.bz2                   \
 font-jis-misc-1.0.3.tar.bz2                    \
 font-micro-misc-1.0.3.tar.bz2                  \
 font-misc-cyrillic-1.0.3.tar.bz2               \
 font-misc-ethiopic-1.0.3.tar.bz2               \
 font-misc-meltho-1.0.3.tar.bz2                 \
 font-misc-misc-1.1.2.tar.bz2                   \
 font-mutt-misc-1.0.3.tar.bz2                   \
 font-schumacher-misc-1.1.2.tar.bz2             \
 font-screen-cyrillic-1.0.4.tar.bz2             \
 font-sony-misc-1.0.3.tar.bz2                   \
 font-sun-misc-1.0.3.tar.bz2                    \
 font-winitzki-cyrillic-1.0.3.tar.bz2           \
 font-xfree86-type1-1.0.4.tar.bz2"
for line in $files
do
  install "$http_root/$line"
done

install -v -d -m755 /usr/share/fonts                               
ln -svfn $XORG_PREFIX/share/fonts/X11/OTF /usr/share/fonts/X11-OTF 
ln -svfn $XORG_PREFIX/share/fonts/X11/TTF /usr/share/fonts/X11-TTF

extract  http://xorg.freedesktop.org/archive/individual/data/xkeyboard-config/xkeyboard-config-2.14.tar.bz2
./configure $XORG_CONFIG --with-xkb-rules-symlink=xorg
make
make install
cd ..

extract  http://xorg.freedesktop.org/archive/individual/xserver/xorg-server-1.17.1.tar.bz2
./configure $XORG_CONFIG            \
           --enable-glamor          \
           --enable-install-setuid  \
           --enable-suid-wrapper    \
           --disable-systemd-logind \
           --with-xkb-output=/var/lib/xkb
make

make install
mkdir -pv /etc/X11/xorg.conf.d
cat >> /etc/sysconfig/createfiles << "EOF"
/tmp/.ICE-unix dir 1777 root root
/tmp/.X11-unix dir 1777 root root
EOF

#Device Drivers  --->
#  Input device support --->
#    <*> Generic input layer (needed for...) [CONFIG_INPUT]
#    <*>   Event interface                   [CONFIG_INPUT_EVDEV]
#    [*]   Miscellaneous devices  --->       [CONFIG_INPUT_MISC]
#      <*>    User level driver support      [CONFIG_INPUT_UINPUT]
install http://www.freedesktop.org/software/libevdev/libevdev-1.3.2.tar.xz
install http://xorg.freedesktop.org/archive/individual/driver/xf86-input-evdev-2.9.1.tar.bz2
install http://xorg.freedesktop.org/archive/individual/driver/xf86-input-synaptics-1.8.1.tar.bz2

wget http://www.linuxfromscratch.org/patches/blfs/7.7/xf86-input-vmmouse-13.0.0-build_fix-1.patch
extract http://xorg.freedesktop.org/archive/individual/driver/xf86-input-vmmouse-13.0.0.tar.bz2
patch -Np1 -i ../xf86-input-vmmouse-13.0.0-build_fix-1.patch &&
sed -i -e '/__i386__/a iopl(3);' tools/vmmouse_detect.c      &&
./configure $XORG_CONFIG               \
            --without-hal-fdi-dir      \
            --without-hal-callouts-dir \
            --with-udev-rules-dir=/lib/udev/rules.d &&
make
make install
cd ..

#Device Drivers  --->
#  HID support  --->
#    <*/M> HID bus support                                      [CONFIG_HID]
#            Special HID drivers --->
#              <*/M> Wacom Intuos/Graphire tablet support (USB) [CONFIG_HID_WACOM]
extract  http://downloads.sourceforge.net/linuxwacom/xf86-input-wacom-0.28.0.tar.bz2
./configure $XORG_CONFIG --with-systemd-unit-dir=no
make
make install
cd ..

install http://people.freedesktop.org/~aplattner/vdpau/libvdpau-0.9.tar.gz

extract  https://github.com/i-rinat/libvdpau-va-gl/releases/download/v0.3.4/libvdpau-va-gl-0.3.4.tar.gz
mkdir build &&
cd    build &&

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$XORG_PREFIX .. &&
make
make install
echo "export VDPAU_DRIVER=va_gl" >> /etc/profile.d/xorg.sh


