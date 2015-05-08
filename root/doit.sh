mount -t cifs //192.168.56.6/data /mnt/data -o username=lester,password=lester123
cd /mnt/data/users/lester/projects/linux-4.0
make O=build/311c KCONFIG_CONFIG=`pwd`/../kernel_configuration/311c.config -j4 install
cd
umount /mnt/projects
init 6

