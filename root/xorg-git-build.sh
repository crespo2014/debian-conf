#!/bin/bash

/bin/true apt-get install autoconf make cmake pkg-config  libperl-dev libgtk2.0-dev \
intltool \
libsysfs-dev \
libudev-dev \
llvm \
libpciaccess-dev \
libtool \
bison \
flex \
python-mako \
libxshmfence-dev \
valgrind \
libgcrypt11-dev

# http://www.mesa3d.org/autoconf.html

GOTO=$1

export XORG_PREFIX="/mnt/data/app/x86"
export XORG_CONFIG="--disable-manpages --enable-shared=no --prefix=$XORG_PREFIX --sysconfdir=/etc --localstatedir=/var  "

export ACLOCAL="aclocal -I $XORG_PREFIX/share/aclocal"

# avoid including : at the beginning when teh variable is empty 
if [ "$PKG_CONFIG_PATH" != "" ]; then
LIBRARY_PATH=$LIBRARY_PATH: 
fi

#packed configuration
if [ "$PKG_CONFIG_PATH" != "" ]; then
PKG_CONFIG_PATH=$PKG_CONFIG_PATH: 
fi

if [ "$C_INCLUDE_PATH" != "" ]; then
C_INCLUDE_PATH=$C_INCLUDE_PATH: 
fi

if [ "$CPLUS_INCLUDE_PATH" != "" ]; then
CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH: 
fi
	
PATH=$PATH:$XORG_PREFIX/bin
PKG_CONFIG_PATH=${PKG_CONFIG_PATH}$XORG_PREFIX/lib/pkgconfig:$XORG_PREFIX/share/pkgconfig  
LIBRARY_PATH=${LIBRARY_PATH}$XORG_PREFIX/lib             
C_INCLUDE_PATH=${C_INCLUDE_PATH}$XORG_PREFIX/include         
CPLUS_INCLUDE_PATH=${CPLUS_INCLUDE_PATH}$XORG_PREFIX/include  

export PATH PKG_CONFIG_PATH ACLOCAL LIBRARY_PATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH	


#Get the source code from git and cd into folder
# $1 git url

function extract()
{
  url=$1
  fname=${url##*/}
  [ -d $fname ] || git clone --depth 1 "$1" $fname
  [ $? != 0 ] && echo "Failed to get repo $1" && exit
  cd $fname
  tag=$(git tag | tail -n 1)
  tag=${tag##* }  
  if [ "$tag" != "" ]; then
    echo "Checking out branch $tag"
    git checkout $tag
  else
   git pull
  fi
  [ $? != 0 ] && echo "Git pull $1 failed " && exit 
  return 0
}

# download build and isntall file received as parameter.
# $1 git url
# $2 extra autogen options
# $3 extra make options
# $4 extra commands after installs

function install_autogen()
{
  extract $1
  ./autogen.sh $XORG_CONFIG $2
  #./configure $XORG_CONFIG
  [ "$?" != "0" ] && exit
  make $3
  [ "$?" != "0" ] && exit
  su -c "make install $4"
  [ "$?" != "0" ] && exit
  [ "$4" != "" ] && $4 && [ "$?" != "0" ] && exit
  cd ..	
}

function make_install_cd()
{
  make
  [ "$?" != "0" ] && exit
  make install
  [ "$?" != "0" ] && exit
  cd ..
}

## the freedesktop git server
GFDR="git://git.freedesktop.org/git"

modules="\
$GFDR/xorg/proto/fontsproto \
$GFDR/xorg/proto/x11proto \
$GFDR/xorg/proto/xextproto \
$GFDR/xorg/proto/videoproto \
$GFDR/xorg/proto/renderproto \
$GFDR/xorg/proto/inputproto \
$GFDR/xorg/proto/damageproto \
$GFDR/xorg/proto/xf86vidmodeproto \
$GFDR/xorg/proto/xf86dgaproto \
$GFDR/xorg/proto/xf86driproto \
$GFDR/xorg/proto/xcmiscproto \
$GFDR/xorg/proto/scrnsaverproto \
$GFDR/xorg/proto/bigreqsproto \
$GFDR/xorg/proto/resourceproto \
$GFDR/xorg/proto/compositeproto \
$GFDR/xorg/proto/resourceproto \
$GFDR/xorg/proto/evieproto \
$GFDR/xorg/proto/kbproto \
$GFDR/xorg/proto/fixesproto \
$GFDR/xcb/proto \
$GFDR/xcb/pthread-stubs \
$GFDR/xcb/libxcb \
$GFDR/xorg/lib/libXext \
$GFDR/xorg/lib/libxtrans \
$GFDR/xorg/lib/libX11 \
$GFDR/xorg/lib/libXau \
$GFDR/xorg/lib/libXi \
$GFDR/xorg/lib/libxkbfile \
$GFDR/xorg/lib/libfontenc \
$GFDR/xorg/lib/libXfont \
$GFDR/xorg/lib/libXv \
$GFDR/xorg/lib/libXvMC \
$GFDR/xorg/lib/libXxf86vm \
$GFDR/xorg/lib/libXinerama \
$GFDR/xorg/lib/libXfixes \
$GFDR/xorg/lib/libXdamage \
$GFDR/xorg/proto/dri2proto \
$GFDR/xorg/proto/glproto \
$GFDR/xorg/lib/libpciaccess \
$GFDR/pixman \
$GFDR/xorg/proto/randrproto"

if [ "$GOTO" == "" ]; then

# Setup macros
extract $GFDR/xorg/util/macros
echo "Building macros"
./autogen.sh --prefix=$XORG_PREFIX && make && su -c "make install"
[ "$?" != "0" ] && exit
cd ..

fi

# Build all modules        

if [ "$GOTO" == "" -o "$GOTO" == "G1" ]; then
GOTO=
echo "Building modules"
for line in $modules
do
  extract "$line"
  ./autogen.sh            \
  --prefix=$XORG_PREFIX   \
  --enable-static=yes     \
  --enable-devel-docs=no  \
  --enable-shared=yes && 
  make && 
  su -c "make install"
  [ "$?" != "0" ] && exit
  cd ..
done
fi

#/usr/src/linux-headers-`uname -r` &&
# --with-kernel-source
# make -C /usr/src/linux-headers-`uname -r` && 

if [ "$GOTO" == "" -o "$GOTO" == "G2" ]; then
GOTO=
  extract git://git.freedesktop.org/git/mesa/drm
  ./autogen.sh --prefix=$XORG_PREFIX --disable-manpages --enable-shared=no --disable-freedreno --disable-intel --disable-radeon --disable-vmwgfx --enable-udev &&
  make &&
  su -c "make install"
  [ "$?" != "0" ] && exit  
fi

if [ "$GOTO" == "" -o "$GOTO" == "G3" ]; then
GOTO=
#--disable-static --enable-static
  extract git://git.freedesktop.org/git/mesa/mesa 
/bin/true ./autogen.sh                            \
      --prefix=$NVD                       \
     --enable-texture-float              \
      --enable-gles1                      \
      --enable-gles2                      \
      --enable-glx                        \
      --enable-egl                        \
      --enable-gallium-egl                \
      --enable-gallium-llvm               \
      --enable-shared-glapi               \
      --enable-gbm                        \
      --enable-glx-tls                    \
      --enable-dri                        \
      --enable-osmesa                     \
      --with-egl-platforms=x11,drm        \
      --with-gallium-drivers=nouveau      \
      --with-dri-drivers=nouveau          \
      --enable-vdpau

/bin/true   --enable-static           \
  --disable-share           \
  --disable-static		   \
  --with-driver=dri        \
  
  ./autogen.sh             \
  --prefix=$XORG_PREFIX    \
  --sysconfdir=/etc        \
  --localstatedir=/var     \
  --disable-glut           \
  --disable-manpages       \
  --disable-freedreno      \
  --disable-intel          \
  --disable-radeon         \
  --disable-vmwgfx         \
  --disable-share           \
  --with-state-trackers="egl dri" &&
  make && 
  su -c "make install"
  [ "$?" != "0" ] && exit
  su -c "mkdir -p $XORG_PREFIX/bin"
  #&&  su -c "install -m755 progs/xdemos/{glxinfo,glxgears} $XORG_PREFIX/bin/"
  [ "$?" != "0" ] && exit
  cd ..
fi


# git clone git://people.freedesktop.org/~aplattner/libvdpau
#$ cd libvdpau/
#$./autogen.sh --prefix=$NVD
#$ make
#$ make install

if [ "$GOTO" == "" -o "$GOTO" == "G4" ]; then
GOTO=
  extract git://git.freedesktop.org/git/xorg/xserver
  ./autogen.sh \
  --prefix=$XORG_PREFIX \
  --enable-builtin-fonts \
  --disable-xfbdev  \
  --disable-ipv6 &&
  make && su -c "make install"
  [ "$?" != "0" ] && exit 
  su -c "chown root $XORG_PREFIX/bin/Xorg" &&
  su -c "chmod +s $XORG_PREFIX/bin/Xorg"
  [ "$?" != "0" ] && exit
  cd ..
fi

REPOS="\
$GFDR/xorg/driver/xf86-input-mouse \
$GFDR/xorg/driver/xf86-input-keyboard \
$GFDR/xorg/driver/xf86-video-intel"

if [ "$GOTO" == "" -o "$GOTO" == "G5" ]; then
GOTO=
for url in $REPOS; do
  extract $url
  ./autogen.sh --prefix=$XORG_PREFIX  
  make && su -c "make install"
  [ "$?" != "0" ] && exit 
  cd ..	
done
 
fi

exit

#If you have a non qwerty keyboard, and don't want to install XKeyboardConfig, you might want to add the following flag to the autogen.sh script, when building the X server :
#--with-xkb-path=/usr/share/X11/xkb (You may need to adjust the path depending on your distribution).
#Also the X server will search for the xkbcomp program in /opt/gfx-test/bin. So you will need to create a symbolic link to your distribution xkbcomp :
#
#cd /opt/gfx-test/bin
#ln -s /usr/bin/xkbcomp xkbcomp

#rmmod i915 # assuming you're using Intel
#rmmod drm
#insmod/linux-core/drm.ko
#insmod/linux-core/i915.ko
#export LD_LIBRARY_PATH=$XORG_PREFIX/lib
#startx -- $XORG_PREFIX/bin/Xorg -verbose # make sure you have a ~/.xinitrc with what you want to run
