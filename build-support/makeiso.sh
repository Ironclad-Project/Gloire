#!/bin/sh

set -ex

# Let the user pass their own $SUDO (or doas).
: "${SUDO:=sudo}"

: "${IMAGE_SIZE:=2G}"

# Ensure that the Ironclad kernel has been cloned.
if ! [ -d ironclad ]; then
    git clone https://github.com/SW480C-O25T18/ironclad.git ironclad
    #git -C ironclad checkout $(cat .ironclad-commit)
fi

# Build the sysroot with jinx, and make sure the packages the particular
# target needs.
set -f
$SUDO rm -rf sysroot

# Retry the installation in case of transient errors like 503 TooManyRequests.
until ./jinx install "sysroot" base $PKGS_TO_INSTALL; do
    echo "Package installation failed (likely due to rate limiting). Retrying in 30 seconds..."
    sleep 30
done

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
$SUDO chmod 750 sysroot/root
$SUDO chmod 777 sysroot/tmp
$SUDO chmod 777 sysroot/run
$SUDO chmod 710 sysroot/home/user

# Prepare the iso and boot directories.
rm -rf iso_root
mkdir -p iso_root/boot

# Create and format the initramfs filesystem.
# TODO: Once ready, move to ext4, now its ext2 only.
fallocate -l "${IMAGE_SIZE}" iso_root/boot/gloire.ext
$SUDO mkfs.ext2 iso_root/boot/gloire.ext
mkdir -p mount_dir
$SUDO mount iso_root/boot/gloire.ext mount_dir

# Copy the system root to the initramfs filesystem.
$SUDO cp -rp sysroot/* mount_dir/
$SUDO mkdir -p mount_dir/boot

# Just plop a random seed file from the systems RNG.
$SUDO dd if=/dev/random of=mount_dir/root/seedfile.bin bs=40M count=1 iflag=fullblock

# Copy the bootloader wallpaper and kernel to the ISO root.
cp artwork/background.png              iso_root/boot/
cp sysroot/usr/share/ironclad/ironclad iso_root/boot/

# Install the boot binaries required by the target.
if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    mkdir -p iso_root/boot/limine
    mkdir -p iso_root/EFI/BOOT
    cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin iso_root/boot/limine/
    cp host-pkgs/limine/usr/local/share/limine/BOOTRISCV64.EFI    iso_root/EFI/BOOT/
else
    mkdir -p iso_root/boot/limine
    mkdir -p iso_root/EFI/BOOT
    cp host-pkgs/limine/usr/local/share/limine/limine-bios.sys    iso_root/boot/limine/
    cp host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin iso_root/boot/limine/
    cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin iso_root/boot/limine/
    cp host-pkgs/limine/usr/local/share/limine/BOOTX64.EFI        iso_root/EFI/BOOT/
    cp host-pkgs/limine/usr/local/share/limine/BOOTIA32.EFI       iso_root/EFI/BOOT/
    cp host-pkgs/memtest86+/boot/memtest.bin                      iso_root/boot/
fi

# Generate the config file. Take into account that there may not be a graphical
# option, and that non x86 ports will not have memtest.
CONFIG_TEMP="$(mktemp)"
cat << 'EOF' > "$CONFIG_TEMP"
timeout: 5
wallpaper: boot():/boot/background.png
wallpaper_style: stretched
term_margin: 0

${KERNEL_PATH}=boot():/boot/ironclad
${PROTOCOL}=limine

EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - Live Graphical
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: init=/bin/env root=ramdev1 initargs="runlevel=graphical-multiuser /sbin/init"
    module_path: boot():/boot/gloire.ext

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
/Gloire - Live TTY only
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: init=/bin/env root=ramdev1 initargs="runlevel=console-multiuser /sbin/init"
    module_path: boot():/boot/gloire.ext

/Advanced options for Gloire
EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Live Graphical Debug (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env root=ramdev1 initargs="runlevel=graphical-multiuser /sbin/init" nolocaslr noprogaslr
        module_path: boot():/boot/gloire.ext

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Live TTY Debug (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env root=ramdev1 initargs="runlevel=console-multiuser /sbin/init" nolocaslr noprogaslr
        module_path: boot():/boot/gloire.ext

    //Gloire - Live Emergency shell (nolocaslr, noprogaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/gcon root=ramdev1 nolocaslr noprogaslr
        module_path: boot():/boot/gloire.ext
EOF

if [ -z "$JINX_CONFIG_FILE" ]; then # Assume its only defined for riscv64.
   cat << 'EOF' >> "$CONFIG_TEMP"

/Memory test (memtest86+)
    protocol: linux
    kernel_path: boot():/boot/memtest.bin
EOF
fi

cp "$CONFIG_TEMP" iso_root/boot/limine/limine.conf
rm "$CONFIG_TEMP"

# Unmount after we are done.
sync
$SUDO umount -R mount_dir
$SUDO rm -rf mount_dir

if [ "$JINX_CONFIG_FILE" = "jinx-config-riscv64" ]; then
    xorriso -as mkisofs -R -r -J \
        -hfsplus -apm-block-size 2048 \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o gloire.iso
else
    xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o gloire.iso

    host-pkgs/limine/usr/local/bin/limine bios-install gloire.iso
fi

rm -rf iso_root
