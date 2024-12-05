#!/bin/sh

set -ex

# Let the user pass their own $SUDO (or doas).
: "${SUDO:=sudo}"

# Ensure that the Ironclad kernel has been cloned.
if ! [ -d ironclad ]; then
    git clone https://git.savannah.nongnu.org/git/ironclad.git ironclad
    git -C ironclad checkout $(cat .ironclad-commit)
fi

# Build the sysroot with jinx, and make sure the packages the particular
# target needs.
set -f
$SUDO rm -rf sysroot
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
$SUDO chown -R root:root sysroot/*
$SUDO chown -R 1000:1000 sysroot/home/user
$SUDO chmod 700 sysroot/root
$SUDO chmod 777 sysroot/tmp

# TODO: Once ready, move to ext4, now its ext2 only.
rm -f gloire.img
fallocate -l 2G gloire.img
$SUDO parted -s gloire.img mklabel gpt
$SUDO parted -s gloire.img mkpart ESP fat32 2048s 5%
$SUDO parted -s gloire.img mkpart gloire_data ext2 5% 100%
$SUDO parted -s gloire.img set 1 esp on
$SUDO sgdisk gloire.img -u 1:f2ac3998-bd9d-4193-bb75-efcd9e90dd96
$SUDO sgdisk gloire.img -u 2:123e4567-e89b-12d3-a456-426614174000

LOOPBACK_DEV=$($SUDO losetup -Pf --show gloire.img)
$SUDO mkfs.fat -F 32 ${LOOPBACK_DEV}p1
$SUDO mkfs.ext2 ${LOOPBACK_DEV}p2
mkdir -p mount_dir
$SUDO mount ${LOOPBACK_DEV}p2 mount_dir

$SUDO cp -rp sysroot/* mount_dir/
$SUDO rm -rf mount_dir/boot
$SUDO mkdir -p mount_dir/boot
$SUDO mount ${LOOPBACK_DEV}p1 mount_dir/boot

# Prepare the iso and boot directories.
$SUDO cp -r artwork/background.bmp              mount_dir/boot/
$SUDO cp -r sysroot/usr/share/ironclad/ironclad mount_dir/boot/

# Install the boot binaries required by the target.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    $SUDO mkdir -pv mount_dir/boot/EFI/BOOT
    $SUDO cp -r host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTRISCV64.EFI       mount_dir/boot/EFI/BOOT/
else
    $SUDO mkdir -pv mount_dir/boot/EFI/BOOT
    $SUDO cp -r host-pkgs/limine/usr/local/share/limine/limine-bios.sys    mount_dir/boot/
    $SUDO cp -r host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mount_dir/boot/
    $SUDO cp -r host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTX64.EFI           mount_dir/boot/EFI/BOOT/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTIA32.EFI          mount_dir/boot/EFI/BOOT/
    $SUDO cp -r host-pkgs/memtest86+/boot/memtest.bin                      mount_dir/boot/
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
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: ${BASECMDLINE} initargs="runlevel=graphical-multiuser /sbin/init"

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - TTY only
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: ${BASECMDLINE} initargs="runlevel=console-multiuser /sbin/init"

/Advanced options for Gloire
EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Graphical Debug (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: ${BASECMDLINE} initargs="runlevel=graphical-multiuser /sbin/init" nolocaslr noprogaslr

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - TTY Debug (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: ${BASECMDLINE} initargs="runlevel=console-multiuser /sbin/init" nolocaslr noprogaslr

    //Gloire - Emergency shell (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: rootuuid=${ROOTUUID} init=/bin/gcon nolocaslr noprogaslr
EOF

if [ -z "$JINX_CONFIG_FILE" ]; then # Assume its only defined for riscv64.
   cat << 'EOF' >> "$CONFIG_TEMP"

/Memory test (memtest86+)
    protocol: linux
    kernel_path: boot():/memtest.bin
EOF
fi

$SUDO mv "$CONFIG_TEMP" mount_dir/boot/limine.conf

# Unmount after we are done.
sync
$SUDO umount -R mount_dir
$SUDO rm -rf mount_dir
$SUDO losetup -d ${LOOPBACK_DEV}

# Post installation triggers on the whole image.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    :
else
    host-pkgs/limine/usr/local/bin/limine bios-install gloire.img
fi
