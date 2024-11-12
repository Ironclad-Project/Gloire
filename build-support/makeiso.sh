#!/bin/sh

set -ex

# Build the sysroot with jinx, and make sure the packages the particular
# target needs.
set -f
sudo rm -rf sysroot
./jinx install "sysroot" base $PKGS_TO_INSTALL
set +f
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    if ! [ -d host-pkgs/limine ]; then
        ./jinx host-build limine
    fi
else
    if ! [ -d host-pkgs/limine ]; then
        ./jinx host-build limine
    fi
    if ! [ -d host-pkgs/memtest86+ ]; then
        ./jinx host-build memtest86+
    fi
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
sudo sgdisk gloire.img -u 1:f2ac3998-bd9d-4193-bb75-efcd9e90dd96
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

# Prepare the iso and boot directories.
sudo cp -r artwork/background.bmp              mount_dir/boot/
sudo cp -r sysroot/usr/share/ironclad/ironclad mount_dir/boot/

# Install the boot binaries required by the target.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    sudo mkdir -pv mount_dir/boot/EFI/BOOT
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    sudo cp host-pkgs/limine/usr/local/share/limine/BOOTRISCV64.EFI       mount_dir/boot/EFI/BOOT/
else
    sudo mkdir -pv mount_dir/boot/EFI/BOOT
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-bios.sys    mount_dir/boot/
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mount_dir/boot/
    sudo cp -r host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    sudo cp host-pkgs/limine/usr/local/share/limine/BOOTX64.EFI           mount_dir/boot/EFI/BOOT/
    sudo cp host-pkgs/limine/usr/local/share/limine/BOOTIA32.EFI          mount_dir/boot/EFI/BOOT/
    sudo cp -r host-pkgs/memtest86+/boot/memtest.bin                      mount_dir/boot/
fi

# Generate the config file. Take into account that there may not be a graphical
# option, and that non x86 ports will not have memtest.
CONFIG_TEMP="$(mktemp)"
cat << 'EOF' > "$CONFIG_TEMP"
timeout: 5
wallpaper: boot():/background.bmp
wallpaper_style: stretched

${KERNEL_PATH}=boot():/ironclad
${PROTOCOL}=limine
${ROOTUUID}=123e4567-e89b-12d3-a456-426614174000
${BASECMDLINE}=init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000

EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - Graphical
    kernel_path: ${KERNEL_PATH}
    protocol: ${PROTOCOL}
    cmdline: ${BASECMDLINE} initargs="runlevel=graphical-multiuser /sbin/init"

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - TTY only
    kernel_path: ${KERNEL_PATH}
    protocol: ${PROTOCOL}
    cmdline: ${BASECMDLINE} initargs="runlevel=console-multiuser /sbin/init"

/Advanced options for Gloire
EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Graphical Debug (nolocaslr, noprogaslr)
        kernel_path: ${KERNEL_PATH}
        protocol: ${PROTOCOL}
        cmdline: ${BASECMDLINE} initargs="runlevel=graphical-multiuser /sbin/init" nolocaslr noprogaslr

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - TTY Debug (nolocaslr, noprogaslr)
        kernel_path: ${KERNEL_PATH}
        protocol: ${PROTOCOL}
        cmdline: ${BASECMDLINE} initargs="runlevel=console-multiuser /sbin/init" nolocaslr noprogaslr

    //Gloire - Emergency shell (nolocaslr, noprogaslr)
        kernel_path: ${KERNEL_PATH}
        protocol: ${PROTOCOL}
        cmdline: rootuuid=${ROOTUUID} init=/bin/gcon nolocaslr noprogaslr
EOF

if [ -z "$JINX_CONFIG_FILE" ]; then # Assume its only defined for riscv64.
   cat << 'EOF' >> "$CONFIG_TEMP"

/Memory test (memtest86+)
    protocol: linux
    kernel_path: boot():/memtest.bin
EOF
fi

sudo cp "$CONFIG_TEMP" mount_dir/boot/limine.conf

# Unmount after we are done.
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
