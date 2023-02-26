#!/bin/sh

set -ex

# Build the sysroot with jinx, build limine and memtest86+
rm -rf sysroot
./jinx sysroot
./jinx host-build limine
./jinx host-build memtest86+

# TODO: Once ready, move to ext4, now its ext2 only.
rm -rf initramfs mount_contents
mkdir mount_contents
fallocate -l 400M initramfs
sudo mkfs.ext2 initramfs
sudo losetup /dev/loop9 initramfs
sudo mount /dev/loop9 mount_contents
sudo cp -r sysroot/* mount_contents/
sudo umount mount_contents
sudo losetup -D /dev/loop9

# Prepare the iso and boot directories.
rm -rf iso_root
mkdir -pv iso_root/boot
cp -r artwork/background.bmp iso_root/boot/
cp -r initramfs              iso_root/boot/
cp -r sysroot/boot/*         iso_root/boot/

# Install the limine and memtest86+ binaries.
cp -r host-pkgs/limine/usr/local/share/limine/limine.sys        iso_root/boot/
cp -r host-pkgs/limine/usr/local/share/limine/limine-cd.bin     iso_root/boot/
cp -r host-pkgs/limine/usr/local/share/limine/limine-cd-efi.bin iso_root/boot/
cp -r host-pkgs/memtest86+/boot/memtest.bin                     iso_root/boot/

# Create the disk image.
xorriso -as mkisofs -b boot/limine-cd.bin -no-emul-boot -boot-load-size 4 \
-boot-info-table --efi-boot boot/limine-cd-efi.bin -efi-boot-part         \
--efi-boot-image --protective-msdos-label iso_root -o gloire.iso

# Install limine.
host-pkgs/limine/usr/local/bin/limine-deploy gloire.iso
