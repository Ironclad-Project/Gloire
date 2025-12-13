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

"${source_dir}"/jinx update base $PKGS_TO_INSTALL

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
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img mkpart ESP fat32 2048s 64MiB
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img set 1 esp on
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img mkpart bios_boot 64MiB 65MiB
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img set 2 bios_grub on
PATH=$PATH:/usr/sbin:/sbin parted -s gloire.img mkpart gloire_data ext2 65MiB 100%
PATH=$PATH:/usr/sbin:/sbin sgdisk gloire.img -u 1:123e4567-e89b-12d3-a456-426614174001
PATH=$PATH:/usr/sbin:/sbin sgdisk gloire.img -u 3:123e4567-e89b-12d3-a456-426614174000
LOOPBACK_DEV=$($SUDO losetup -f)
$SUDO losetup -Pf gloire.img
$SUDO mkfs.fat -F 32 ${LOOPBACK_DEV}p1
$SUDO mkfs.ext2 ${LOOPBACK_DEV}p3
mkdir -p mount_dir
$SUDO mount ${LOOPBACK_DEV}p3 mount_dir

# Copy the system root to the initramfs filesystem.
$SUDO cp -rp sysroot/* mount_dir/
$SUDO rm -rf mount_dir/boot
$SUDO mkdir -p mount_dir/boot
$SUDO mount ${LOOPBACK_DEV}p1 mount_dir/boot

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
    cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000  initargs="runlevel=console-multiuser /sbin/init" noaslr

/Advanced options for Gloire
EOF

if [ -f sysroot/usr/bin/slim ]; then
    cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Live Graphical Debug (noaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000  initargs="runlevel=graphical-multiuser /sbin/init" noaslr

EOF
fi

cat << 'EOF' >> "$CONFIG_TEMP"
    //Gloire - Live TTY Debug (noaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env rootuuid=123e4567-e89b-12d3-a456-426614174000  initargs="runlevel=console-multiuser /sbin/init" noaslr

    //Gloire - Live Emergency shell (noaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/gcon rootuuid=123e4567-e89b-12d3-a456-426614174000  noaslr
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

# Add init system config.
$SUDO mkdir mount_dir/etc/epoch
$SUDO sh -c "
cat << 'EOF' >> mount_dir/etc/epoch/epoch.conf
# https://universe2.us/epochconfig.html

Hostname=FILE /etc/hostname
DefaultRunlevel=graphical-multiuser
DisableCAD=false
EnableLogging=true
BlankLogOnBoot=true

ObjectID=mounting
   ObjectDescription=Mounting /etc/fstab partitions
   ObjectStartCommand=mount -a
   ObjectStopCommand=NONE
   ObjectStartPriority=1
   ObjectStopPriority=0
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=RAWDESCRIPTION

ObjectID=clear_tmp
   ObjectDescription=Clearing /tmp and /run
   ObjectStartCommand=rm -rvf /tmp/* &>/dev/null && rm -rvf /run/* &>/dev/null
   ObjectStopCommand=NULL
   ObjectStartPriority=2
   ObjectStopPriority=0
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=RAWDESCRIPTION

ObjectID=xbps_reconf
   ObjectDescription=Running first boot configuration (if needed)
   ObjectStartCommand=gloire-first-boot
   ObjectStopCommand=NONE
   ObjectStartPriority=3
   ObjectStopPriority=0
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=RAWDESCRIPTION

ObjectID=metalog
   ObjectDescription=metalog
   ObjectStartCommand=/usr/bin/metalog -N -B
   ObjectStopCommand=PID
   ObjectStartPriority=4
   ObjectStopPriority=2
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=SERVICE

ObjectID=crond
   ObjectDescription=crond
   ObjectStartCommand=/usr/bin/crond
   ObjectStopCommand=PIDFILE /var/run/crond.pid
   ObjectStartPriority=5
   ObjectStopPriority=3
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=SERVICE

ObjectID=powerd
   ObjectDescription=power management daemon
   ObjectStartCommand=/usr/bin/powerd
   ObjectStopCommand=PID
   ObjectStartPriority=6
   ObjectStopPriority=4
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=SERVICE

ObjectID=slim
   ObjectDescription=SLiM
   ObjectStartCommand=/usr/bin/slim
   ObjectStopCommand=PID
   ObjectStartPriority=7
   ObjectStopPriority=5
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser
   ObjectOptions=SERVICE

ObjectID=dbus
   ObjectDescription=dbus
   ObjectStartCommand=mkdir /run/dbus && /usr/bin/dbus-daemon --system
   ObjectStopCommand=PID
   ObjectStartPriority=8
   ObjectStopPriority=6
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser
   ObjectOptions=SERVICE

ObjectID=gcon
   ObjectDescription=gcon
   ObjectStartCommand=/usr/bin/gcon
   ObjectStopCommand=PID
   ObjectStartPriority=7
   ObjectStopPriority=5
   ObjectEnabled=true
   ObjectRunlevels=console-multiuser
   ObjectOptions=FORK

ObjectID=killall5_soft
   ObjectDescription=Terminating all processes
   ObjectStopCommand=killall5 -15 && sleep 5
   ObjectStartPriority=0
   ObjectStopPriority=8
   ObjectEnabled=true
   ObjectOptions=HALTONLY RAWDESCRIPTION

ObjectID=killall5
   ObjectDescription=Killing all processes
   ObjectStopCommand=killall5 -9 && sleep 5
   ObjectStartPriority=0
   ObjectStopPriority=9
   ObjectEnabled=true
   ObjectOptions=HALTONLY RAWDESCRIPTION
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
