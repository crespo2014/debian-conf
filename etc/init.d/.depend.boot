TARGETS = mountkernfs.sh hostname.sh x11-common udev keyboard-setup mountdevsubfs.sh console-setup hwclock.sh checkroot.sh mountall.sh mountall-bootclean.sh mountnfs.sh mountnfs-bootclean.sh alsa-utils urandom networking bootmisc.sh checkfs.sh checkroot-bootclean.sh kmod kbd mtab.sh hdparm procps udev-mtab
INTERACTIVE = udev keyboard-setup console-setup checkroot.sh checkfs.sh kbd
udev: mountkernfs.sh
keyboard-setup: mountkernfs.sh udev
mountdevsubfs.sh: mountkernfs.sh udev
console-setup: mountall.sh mountall-bootclean.sh mountnfs.sh mountnfs-bootclean.sh kbd
hwclock.sh: mountdevsubfs.sh
checkroot.sh: hwclock.sh mountdevsubfs.sh hostname.sh hdparm keyboard-setup
mountall.sh: checkfs.sh checkroot-bootclean.sh
mountall-bootclean.sh: mountall.sh
mountnfs.sh: mountall.sh mountall-bootclean.sh networking
mountnfs-bootclean.sh: mountall.sh mountall-bootclean.sh mountnfs.sh
alsa-utils: mountall.sh mountall-bootclean.sh mountnfs.sh mountnfs-bootclean.sh
urandom: mountall.sh mountall-bootclean.sh hwclock.sh
networking: mountkernfs.sh mountall.sh mountall-bootclean.sh urandom
bootmisc.sh: mountall.sh mountall-bootclean.sh mountnfs.sh mountnfs-bootclean.sh udev checkroot-bootclean.sh
checkfs.sh: checkroot.sh mtab.sh
checkroot-bootclean.sh: checkroot.sh
kmod: checkroot.sh
kbd: mountall.sh mountall-bootclean.sh mountnfs.sh mountnfs-bootclean.sh
mtab.sh: checkroot.sh
hdparm: mountdevsubfs.sh udev
procps: mountkernfs.sh mountall.sh mountall-bootclean.sh udev
udev-mtab: udev mountall.sh mountall-bootclean.sh
