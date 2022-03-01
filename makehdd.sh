#! /bin/sh

set -e

# Setup the disk image.
dd if=/dev/zero bs=1M count=0 seek=64 of=gloire.hdd
parted -s gloire.hdd mklabel gpt
parted -s gloire.hdd mkpart primary 2048s 100%
sudo losetup -Pf --show gloire.hdd > loopback_dev

# Make the FSes with loopback devices and mount them.
mkdir loopback_dir
sudo partprobe `cat loopback_dev`
sudo mkfs.ext2 `cat loopback_dev`p1
sudo mount `cat loopback_dev`p1 loopback_dir
sync

# Make a tar with the sysroot.
(cd ../build/system-root && tar cvf ../initramfs.tar *)

# Copy the config files, tar, and the kernel.
sudo mkdir -pv loopback_dir/boot
sudo cp -r ../sysroot/* loopback_dir/
sudo cp -r ../build/initramfs.tar loopback_dir/boot/
sudo cp -r ../build/system-root/usr/share/ironclad loopback_dir/boot/

# Copy limine binaries.
sudo mkdir -p loopback_dir/boot/EFI/BOOT
sudo cp ../ports/limine/limine.sys loopback_dir/boot/
sudo cp ../ports/limine/BOOTX64.EFI loopback_dir/boot/EFI/BOOT/

# Unmount the drive and delete temporary fiels.
sudo umount loopback_dir
sudo losetup -d `cat loopback_dev`
sudo rm -rf loopback_dev loopback_dir

# Install limine.
../ports/limine/limine-install-linux-x86_64 gloire.hdd
