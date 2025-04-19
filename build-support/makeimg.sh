#!/bin/sh

set -ex

# Let the user pass their own $SUDO (or doas).
: "${SUDO:=sudo}"

# Ensure that the Ironclad kernel has been cloned.
if ! [ -d ironclad ]; then
    git clone https://codeberg.org/Ironclad/Ironclad ironclad
    git -C ironclad checkout $(cat .ironclad-commit)
fi

# Build the sysroot with jinx, and make sure the packages the particular
# target needs.
set -f
$SUDO rm -rf sysroot

# Retry the installation in case of transient errors like 503 TooManyRequests.
until ./jinx build-if-needed base $PKGS_TO_INSTALL; do
    echo "Package installation failed (likely due to rate limiting). Retrying in 30 seconds..."
    sleep 30
done

$SUDO ./jinx install "sysroot" base $PKGS_TO_INSTALL

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

# Prepare the iso and boot directories.
rm -rf mount_dir

# Allocate the image. If a size is passed, we just use that size, else, we try
# to guesstimate calculate a rough size.
if [ -z "$IMAGE_SIZE" ]; then
    if [ -f sysroot/usr/bin/slim ]; then
        IMAGE_SIZE=3G
    else
        IMAGE_SIZE=750M
    fi
fi
rm -f gloire.img
fallocate -l "${IMAGE_SIZE}" gloire.img

# Format and mount the image.
$SUDO parted -s gloire.img mklabel gpt
$SUDO parted -s gloire.img mkpart ESP fat32 2048s 5%
$SUDO parted -s gloire.img mkpart gloire_data ext2 5% 100%
$SUDO parted -s gloire.img set 1 esp on
$SUDO sgdisk gloire.img -u 2:123e4567-e89b-12d3-a456-426614174000
LOOPBACK_DEV=$($SUDO losetup -Pf --show gloire.img)
$SUDO mkfs.fat ${LOOPBACK_DEV}p1
$SUDO mkfs.ext2 ${LOOPBACK_DEV}p2
mkdir -p mount_dir
$SUDO mount ${LOOPBACK_DEV}p2 mount_dir

# Copy the system root to the initramfs filesystem.
$SUDO cp -rp sysroot/* mount_dir/
$SUDO rm -rf mount_dir/boot
$SUDO mkdir -p mount_dir/boot
$SUDO mount ${LOOPBACK_DEV}p1 mount_dir/boot

# Just plop a random seed file from the systems RNG.
$SUDO dd if=/dev/random of=mount_dir/root/seedfile.bin bs=40M count=1 iflag=fullblock

# Copy the bootloader wallpaper and kernel to the ISO root.
$SUDO cp artwork/background.png              mount_dir/boot/
$SUDO cp sysroot/usr/share/ironclad/ironclad mount_dir/boot/

# Install the boot binaries required by the target.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    mkdir -p mount_dir/boot/limine
    mkdir -p mount_dir/boot/EFI/BOOT
    cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/limine/
    cp host-pkgs/limine/usr/local/share/limine/BOOTRISCV64.EFI    mount_dir/EFI/BOOT/
else
    $SUDO mkdir -p mount_dir/boot/EFI/BOOT
    $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-bios.sys    mount_dir/boot/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mount_dir/boot/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTX64.EFI        mount_dir/boot/EFI/BOOT/
    $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTIA32.EFI       mount_dir/boot/EFI/BOOT/
    $SUDO cp host-pkgs/memtest86+/boot/memtest.bin                      mount_dir/boot/
fi

# Generate the config file. Take into account that there may not be a graphical
# option, and that non x86 ports will not have memtest.
CONFIG_TEMP="$(mktemp)"
cat << 'EOF' > "$CONFIG_TEMP"
timeout: 5
wallpaper: boot():/background.png
wallpaper_style: stretched
term_margin: 0

${KERNEL_PATH}=boot():/ironclad
${PROTOCOL}=limine

EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - Live Graphical
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000 initargs="runlevel=graphical-multiuser /sbin/init"

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - Live TTY only
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000  initargs="runlevel=console-multiuser /sbin/init"

/Advanced options for Gloire
EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Live Graphical Debug (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000  initargs="runlevel=graphical-multiuser /sbin/init" nolocaslr noprogaslr

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Live TTY Debug (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000  initargs="runlevel=console-multiuser /sbin/init" nolocaslr noprogaslr

    //Gloire - Live Emergency shell (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/gcon rootuuid=123e4567-e89b-12d3-a456-426614174000  nolocaslr noprogaslr
EOF

if [ -z "$JINX_CONFIG_FILE" ]; then # Assume its only defined for riscv64.
   cat << 'EOF' >> "$CONFIG_TEMP"

/Memory test (memtest86+)
    protocol: linux
    kernel_path: boot():/boot/memtest.bin
EOF
fi

sudo cp "$CONFIG_TEMP" mount_dir/boot/limine.conf
rm "$CONFIG_TEMP"

# Unmount after we are done.
sync
$SUDO umount -R mount_dir
$SUDO rm -rf mount_dir
$SUDO losetup -d ${LOOPBACK_DEV}

host-pkgs/limine/usr/local/bin/limine bios-install gloire.img
