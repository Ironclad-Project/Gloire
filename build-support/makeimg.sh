#!/bin/sh

set -e

script_dir="$(dirname "$0")"
test -z "${script_dir}" && script_dir=.

source_dir="$(cd "${script_dir}"/.. && pwd -P)"
build_dir="$(pwd -P)"

# Let the user pass their own $SUDO (or doas).
: "${SUDO:=sudo}"

# Set ARCH based on the build directory.
case "$(basename "${build_dir}")" in
    build-x86_64) ARCH=x86_64 ;;
    build-riscv64) ARCH=riscv64 ;;
    *)
        echo "error: The build directory must be called 'build-<architecture>'." 1>&2
        exit 1
        ;;
esac

# Ensure that the Ironclad kernel has been cloned.
if ! [ -d "${source_dir}"/ironclad ]; then
    git clone https://codeberg.org/Ironclad/Ironclad "${source_dir}"/ironclad
    git -C "${source_dir}"/ironclad checkout $(cat "${source_dir}"/.ironclad-commit)
fi

# Create build directory if needed.
mkdir -p "${build_dir}"

# Enter build directory.
cd "${build_dir}"

# If already initialized, get ARCH from .jinx-parameters file.
if [ -f .jinx-parameters ]; then
    if ! [ "${ARCH}" = "$(. ./.jinx-parameters && echo "${JINX_ARCH}")" ]; then
        echo "error: Jinx architecture and build dir derived architecture mismatch. Delete build dir." 1>&2
        exit 1
    fi
fi

# Build the sysroot with jinx, and make sure the packages the particular
# target needs.
set -f
$SUDO rm -rf sysroot

if ! [ -f .jinx-parameters ]; then
    "${source_dir}"/jinx init "${source_dir}" ARCH="${ARCH}"
fi

"${source_dir}"/jinx build-if-needed base $PKGS_TO_INSTALL

$SUDO "${source_dir}"/jinx install "sysroot" base $PKGS_TO_INSTALL

set +f

if ! [ -d host-pkgs/limine ]; then
    "${source_dir}"/jinx host-build limine
fi

if [ "$ARCH" = x86_64 ]; then
    if ! [ -d host-pkgs/memtest86+ ]; then
        "${source_dir}"/jinx host-build memtest86+
    fi
fi

# Prepare the iso and boot directories.
rm -rf mount_dir

# Allocate the image. If a size is passed, we just use that size, else, we try
# to guesstimate calculate a rough size.
# Try to not use fractional sizes (3.X for example) since certain Linux distros
# like debian struggle to use it.
if [ -z "$IMAGE_SIZE" ]; then
    IMAGE_SIZE=4G
fi
rm -f gloire.img
fallocate -l "${IMAGE_SIZE}" gloire.img

# Format and mount the image.
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img mklabel gpt
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img mkpart ESP fat32 2048s 5%
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img mkpart gloire_data ext2 5% 100%
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img set 1 esp on
PATH=$PATH:/usr/sbin:/sbin sgdisk gloire.img -u 1:123e4567-e89b-12d3-a456-426614174001
PATH=$PATH:/usr/sbin:/sbin sgdisk gloire.img -u 2:123e4567-e89b-12d3-a456-426614174000
LOOPBACK_DEV=$($SUDO losetup -f)
$SUDO losetup -Pf gloire.img
$SUDO mkfs.fat -F 32 ${LOOPBACK_DEV}p1
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
$SUDO cp "${source_dir}"/artwork/background.png mount_dir/boot/
$SUDO cp sysroot/usr/share/ironclad/ironclad mount_dir/boot/

# Install the boot binaries required by the target.
case "$ARCH" in
    riscv64)
        $SUDO mkdir -p mount_dir/boot/limine
        $SUDO mkdir -p mount_dir/boot/EFI/BOOT
        $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/limine/
        $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTRISCV64.EFI    mount_dir/boot/EFI/BOOT/
        ;;
    x86_64)
        $SUDO mkdir -p mount_dir/boot/EFI/BOOT
        $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-bios.sys    mount_dir/boot/
        $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-bios-cd.bin mount_dir/boot/
        $SUDO cp host-pkgs/limine/usr/local/share/limine/limine-uefi-cd.bin mount_dir/boot/
        $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTX64.EFI        mount_dir/boot/EFI/BOOT/
        $SUDO cp host-pkgs/limine/usr/local/share/limine/BOOTIA32.EFI       mount_dir/boot/EFI/BOOT/
        $SUDO cp host-pkgs/memtest86+/boot/memtest.bin                      mount_dir/boot/
        ;;
esac

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

if [ "$ARCH" = x86_64 ]; then # Assume its only defined for riscv64.
   cat << 'EOF' >> "$CONFIG_TEMP"

/Memory test (memtest86+)
    protocol: linux
    kernel_path: boot():/memtest.bin
EOF
fi

$SUDO cp "$CONFIG_TEMP" mount_dir/boot/limine.conf
rm "$CONFIG_TEMP"

# Modify the fstab to use our final partitions.
$SUDO sh -c "
cat << 'EOF' >> mount_dir/etc/fstab
PARTUUID=123e4567-e89b-12d3-a456-426614174000 / ext relatime 0 1
PARTUUID=123e4567-e89b-12d3-a456-426614174001 /boot fat relatime 0 1
EOF
"

# Unmount after we are done.
sync
$SUDO umount mount_dir/boot
$SUDO umount mount_dir
$SUDO rm -rf mount_dir
$SUDO losetup -d ${LOOPBACK_DEV}

# Arch-specific image triggers.
if [ "$ARCH" = x86_64 ]; then
    host-pkgs/limine/usr/local/bin/limine bios-install gloire.img
fi

sync
