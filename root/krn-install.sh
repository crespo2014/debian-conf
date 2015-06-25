mount /mnt/c2d-projects
cd /mnt/c2d-projects/linux-4.0
#make O=build/311c KCONFIG_CONFIG=`pwd`/../kernel_configuration/311c.config -j4 modules_install
make O=build/311c KCONFIG_CONFIG=`pwd`/../kernel_configuration/311c.config -j4 install
cd
umount /mnt/c2d-projects
#init 6

