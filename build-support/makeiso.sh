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

"${source_dir}"/jinx update -b base $PKGS_TO_INSTALL

$SUDO "${source_dir}"/jinx install "sysroot" base $PKGS_TO_INSTALL

set +f

"${source_dir}"/jinx update -b host:limine

if [ "$ARCH" = x86_64 ]; then
    "${source_dir}"/jinx update -b host:memtest86+
fi

# Prepare the iso and boot directories.
$SUDO rm -rf iso_root
mkdir -p iso_root/boot

# Add init system config.
$SUDO mkdir sysroot/etc/epoch
$SUDO sh -c "
cat << 'EOF' >> sysroot/etc/epoch/epoch.conf
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

ObjectID=powerd
   ObjectDescription=power management daemon
   ObjectStartCommand=/usr/bin/powerd
   ObjectStopCommand=PID
   ObjectStartPriority=4
   ObjectStopPriority=4
   ObjectEnabled=true
   ObjectRunlevels=graphical-multiuser console-multiuser
   ObjectOptions=SERVICE

ObjectID=gcon
   ObjectDescription=gcon
   ObjectStartCommand=/usr/bin/gcon
   ObjectStopCommand=PID
   ObjectStartPriority=5
   ObjectStopPriority=5
   ObjectEnabled=true
   ObjectRunlevels=console-multiuser
   ObjectOptions=FORK

ObjectID=killall5_soft
   ObjectDescription=Terminating all processes
   ObjectStopCommand=killall5 -15 && sleep 5
   ObjectStartPriority=0
   ObjectStopPriority=6
   ObjectEnabled=true
   ObjectOptions=HALTONLY RAWDESCRIPTION

ObjectID=killall5
   ObjectDescription=Killing all processes
   ObjectStopCommand=killall5 -9 && sleep 5
   ObjectStartPriority=0
   ObjectStopPriority=7
   ObjectEnabled=true
   ObjectOptions=HALTONLY RAWDESCRIPTION
EOF
"

# Add /etc/issue and /etc/motd for flare and valuable info.
$SUDO sh -c "
cat << 'EOF' >> sysroot/etc/issue
                  &#BGPPPPPG#&
               B5?77!!?YJJ7!7YBB&
            &G5YJ77!7JYYYYYBPJ&PY#
          #PYYYYYY?!?YYYYY7?7JP5JJ
         B?YYYYYY7!!7JYYYYJ!!?JJJ5
  &&    B7?J?77?7!!!!!77777!7Y5YYBBPGGG&
 G77?YBB!!!!!!!!!!!!!JYJ??7JYJJY# PYPPG&    Welcome to [1;35mGloire[0m's
 J777JB?!7JJ???!!!7?JYYYYYPJ!7JB            Live ISO!
 GYYG #JJJJJ??7!!!JYYY5PGB&GB&
    #Y!?GB5YYJY5PG###&
    GJJP

Gloire is an OS built with the [1mIronclad[0m kernel and using [1;31mGNU[0m tools for the userland.
It is 100% freedom-respecting free/libre software, and distributed following
the GNU Free System Distribution Guidelines.

Gloire and Ironclad come with ABSOLUTELY NO WARRANTY, to the extent permitted
by applicable law.

Available users for login in this image are 'user' and 'root', both have no
passwords.
EOF
"

$SUDO sh -c "
cat << 'EOF' >> sysroot/etc/motd
Please report any issues at <https://codeberg.org/Ironclad/Gloire/issues>, or
check the differences between a Linux and Ironclad userland at
<https://codeberg.org/Ironclad/Gloire/wiki/Differences-with-GNU-Linux>.

Thanks in advance, and [1;34mhave a nice time around[0m!
EOF
"

$SUDO sh -c "
cat << 'EOF' >> sysroot/etc/hostname
gloirelive
EOF
"

# Copy the system root to the initramfs filesystem.
cd sysroot
$SUDO tar --sort=name --format=ustar -czf ../gloire-root.tar.gz *
cd ..

# Copy the bootloader wallpaper and kernel to the ISO root.
cp "${source_dir}"/artwork/background.png iso_root/boot/
cp sysroot/usr/share/ironclad/ironclad iso_root/boot/
mv gloire-root.tar.gz iso_root/boot/

# Install the boot binaries required by the target.
rm -rf limine-tmp
mkdir limine-tmp
( cd limine-tmp && tar -xf $(ls -1 ../host-pkgs/limine-*.xbps | sort -Vr | head -1) )
case "$ARCH" in
    riscv64)
        $SUDO mkdir -p iso_root/boot/limine
        $SUDO mkdir -p iso_root/EFI/BOOT
        $SUDO cp limine-tmp/usr/local/share/limine/limine-uefi-cd.bin iso_root/boot/limine/
        $SUDO cp limine-tmp/usr/local/share/limine/BOOTRISCV64.EFI    iso_root/EFI/BOOT/
        ;;
    x86_64)
        rm -rf memtest-tmp
        mkdir memtest-tmp
        ( cd memtest-tmp && tar -xf $(ls -1 ../host-pkgs/memtest86+-*.xbps | sort -Vr | head -1) )
        $SUDO mkdir -p iso_root/boot/limine
        $SUDO mkdir -p iso_root/EFI/BOOT
        $SUDO cp limine-tmp/usr/local/share/limine/limine-bios.sys    iso_root/boot/limine/
        $SUDO cp limine-tmp/usr/local/share/limine/limine-bios-cd.bin iso_root/boot/limine/
        $SUDO cp limine-tmp/usr/local/share/limine/limine-uefi-cd.bin iso_root/boot/limine/
        $SUDO cp limine-tmp/usr/local/share/limine/BOOTX64.EFI        iso_root/EFI/BOOT/
        $SUDO cp limine-tmp/usr/local/share/limine/BOOTIA32.EFI       iso_root/EFI/BOOT/
        $SUDO cp memtest-tmp/boot/memtest.bin                         iso_root/boot/
        ;;
esac

# Generate the config file. Take into account that there may not be a graphical
# option, and that non x86 ports will not have memtest.
CONFIG_TEMP="$(mktemp)"
cat << 'EOF' > "$CONFIG_TEMP"
timeout: 5
wallpaper: boot():/boot/background.png
wallpaper_style: stretched

${KERNEL_PATH}=boot():/boot/ironclad
${PROTOCOL}=limine

/Gloire - Live TTY only
    protocol: ${PROTOCOL}
    path: ${KERNEL_PATH}
    cmdline: init=/bin/env root=ramdev1 initargs="runlevel=console-multiuser /sbin/init"
    module_path: $boot():/boot/gloire-root.tar.gz

/Advanced options for Gloire
    //Gloire - Live TTY Debug (noaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/env root=ramdev1 initargs="runlevel=console-multiuser /sbin/init" noaslr
        module_path: $boot():/boot/gloire-root.tar.gz

    //Gloire - Live Emergency shell (noaslr)
        protocol: ${PROTOCOL}
        path: ${KERNEL_PATH}
        cmdline: init=/bin/gcon root=ramdev1 noaslr
        module_path: $boot():/boot/gloire-root.tar.gz
EOF

if [ "$ARCH" = x86_64 ]; then # Assume its only defined for riscv64.
   cat << 'EOF' >> "$CONFIG_TEMP"

/Memory test (memtest86+)
    protocol: linux
    kernel_path: boot():/memtest.bin
EOF
fi
cp "$CONFIG_TEMP" iso_root/boot/limine.conf
rm "$CONFIG_TEMP"

# Make the ISO.
if [ -z "$IMAGE_NAME" ]; then
    IMAGE_NAME=gloire.iso
fi

if [ "$ARCH" = riscv64 ]; then
    xorriso -as mkisofs -R -r -J \
        -hfsplus -apm-block-size 2048 \
        --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o "$IMAGE_NAME"
else
    xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o "$IMAGE_NAME"

    limine-tmp/usr/local/bin/limine bios-install "$IMAGE_NAME"
fi

rm -rf limine-tmp memtest-tmp

sync
