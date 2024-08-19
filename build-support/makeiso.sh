#!/bin/sh

set -ex

# Build the sysroot with jinx, and make sure the packages the particular
# target needs.
sudo ./jinx sysroot
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    ./jinx host-build limine
else
    ./jinx host-build limine
    ./jinx host-build memtest86+
fi

# Ensure permissions are set.
sudo chown -R root:root sysroot/*
sudo chown -R 1000:1000 sysroot/home/user
sudo chmod 700 sysroot/root
sudo chmod 777 sysroot/tmp

# TODO: Once ready, move to ext4, now its ext2 only.
rm -f gloire.img
fallocate -l 2G gloire.img
sudo parted -s gloire.img mklabel gpt
sudo parted -s gloire.img mkpart ESP fat32 2048s 5%
sudo parted -s gloire.img mkpart gloire_data ext2 5% 100%
sudo parted -s gloire.img set 1 esp on
sudo sgdisk gloire.img -u 2:123e4567-e89b-12d3-a456-426614174000

LOOPBACK_DEV=$(sudo losetup -Pf --show gloire.img)
sudo mkfs.fat ${LOOPBACK_DEV}p1
sudo mkfs.ext2 ${LOOPBACK_DEV}p2
mkdir -p mount_dir
sudo mount ${LOOPBACK_DEV}p2 mount_dir

sudo cp -rp sysroot/* mount_dir/
sudo rm -rf mount_dir/boot
sudo mkdir -p mount_dir/boot
sudo mount ${LOOPBACK_DEV}p1 mount_dir/boot
sudo cp -r sysroot/boot/* mount_dir/boot/

# Prepare the iso and boot directories.
sudo cp -r artwork/background.bmp              mount_dir/boot/
sudo cp -r sysroot/usr/share/ironclad/ironclad mount_dir/boot/

# Install the boot binaries required by the target.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    sudo mkdir -pv mount_dir/boot/EFI/BOOT
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    sudo cp host-pkgs/limine/usr/local/share/limine/BOOT*.EFI             mount_dir/boot/EFI/BOOT/
else
    sudo mkdir -pv mount_dir/boot/EFI/BOOT
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-bios.sys    mount_dir/boot/
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mount_dir/boot/
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    sudo cp host-pkgs/limine/usr/local/share/limine/BOOT*.EFI             mount_dir/boot/EFI/BOOT/
    sudo cp -r host-pkgs/memtest86+/boot/memtest.bin                      mount_dir/boot/
fi

sync
sudo umount -R mount_dir
sudo rm -rf mount_dir
sudo losetup -d ${LOOPBACK_DEV}

# Post installation triggers on the whole image.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    true
else
    host-pkgs/limine/usr/local/bin/limine bios-install gloire.img
fi
