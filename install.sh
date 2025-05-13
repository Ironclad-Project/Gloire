#!/bin/bash
set -e  # exit on error

# -------------------------------
# Helper: Detect package manager
# -------------------------------
if command -v apt-get >/dev/null 2>&1; then
    PM="apt"
elif command -v pacman >/dev/null 2>&1; then
    PM="pacman"
elif command -v dnf >/dev/null 2>&1; then
    PM="dnf"
else
    echo "No supported package manager found (apt-get, pacman, or dnf). Please install dependencies manually."
    exit 1
fi

# ------------------------------------------------------------
# Install required packages (attempt arch-agnostic installation)
# ------------------------------------------------------------
echo "Installing required packages..."

if [ "$PM" = "apt" ]; then
    sudo apt-get update
    sudo apt-get install -y \
        lzip git build-essential rsync xorriso curl \
        libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev ninja-build flex bison \
        git-email \
        libaio-dev libbluetooth-dev libcapstone-dev libbrlapi-dev libbz2-dev \
        libcap-ng-dev libcurl4-gnutls-dev libgtk-3-dev \
        libibverbs-dev libjpeg8-dev libncurses5-dev libnuma-dev \
        librbd-dev librdmacm-dev \
        libsasl2-dev libsdl2-dev libseccomp-dev libsnappy-dev libssh-dev \
        libvde-dev libvdeplug-dev libvte-2.91-dev libxen-dev liblzo2-dev \
        valgrind xfslibs-dev \
        libnfs-dev libiscsi-dev code

elif [ "$PM" = "pacman" ]; then
    sudo pacman -Sy --needed \
        lzip git rsync xorriso curl \
        base-devel glib2 dtc pixman zlib ninja flex bison \
        # git-email is usually included with git on Arch; if not, install manually
        libaio bluez capstone brltty bzip2 libcap-ng gtk3 \
        libibverbs libjpeg-turbo ncurses numactl \
        # librbd and librdmacm may be part of ceph or rdma-core packages:
        ceph rdma-core \
        cyrus-sasl sdl2 libseccomp snappy libssh \
        vde vde2 vte3 xen lzo valgrind xfsprogs \
        libnfs libiscsi code

elif [ "$PM" = "dnf" ]; then
    sudo dnf install -y \
        lzip git @development-tools rsync xorriso curl \
        glib2-devel dtc pixman-devel zlib-devel ninja-build flex bison \
        git-email \
        libaio-devel bluez-libs-devel capstone-devel brlapi-devel bzip2-devel \
        libcap-ng-devel libcurl-devel gtk3-devel \
        libibverbs-devel libjpeg-turbo-devel ncurses-devel numactl-devel \
        librbd-devel librdmacm-devel \
        cyrus-sasl-devel SDL2-devel libseccomp-devel snappy-devel libssh-devel \
        vde-devel vte291-devel xen-devel lzo-devel valgrind xfsprogs-devel \
        libnfs-devel libiscsi-devel code
fi

# ----------------------------------------
# Clone and build QEMU from GitLab repository
# ----------------------------------------
echo "Cloning QEMU repository..."
git clone https://gitlab.com/qemu-project/qemu.git
cd qemu

echo "Configuring QEMU..."
./configure

echo "Building QEMU (this may take a while)..."
make -j"$(nproc)"

# ---------------------------------------------------------------------
# Add current directory (the QEMU build directory) to PATH and re-source terminal
# ---------------------------------------------------------------------
echo "Adding $(pwd) to PATH in ~/.bashrc..."
if ! grep -q "$(pwd)" ~/.bashrc; then
    echo "export PATH=\$PATH:$(pwd)" >> ~/.bashrc
fi
# Source the updated bashrc for the current session (works for bash; if using another shell adjust accordingly)
source ~/.bashrc

# ---------------------------------------------------------------------
# Download firmware file (or use dd to create a sparse file if needed)
# ---------------------------------------------------------------------
# Set a path for the firmware file (adjust as desired)
FIRMWARE_PATH="$(pwd)/../ovmf-code-riscv64.fd"
echo "Downloading firmware to ${FIRMWARE_PATH}..."
if [ ! -f "$FIRMWARE_PATH" ]; then
    curl -L -o "$FIRMWARE_PATH" "https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-riscv64.fd"
fi

# Ensure the firmware file is 32MB (creating a sparse file if necessary)
echo "Ensuring firmware file is padded to 32MB..."
dd if=/dev/zero of="$FIRMWARE_PATH" bs=1 count=0 seek=33554432

# --------------------------------------------------
# Disable apparmor restriction on unprivileged userns
# --------------------------------------------------
echo "Disabling AppArmor unprivileged userns restrictions..."
sudo sysctl kernel.apparmor_restrict_unprivileged_userns=0
sudo sh -c 'echo "kernel.apparmor_restrict_unprivileged_userns = 0" > /etc/sysctl.d/99-userns.conf'

# --------------------------------------------------
# Install VS code adaCore extension
# --------------------------------------------------

code --install-extension adacore.ada

# --------------------------------------------------
# Return to top-level (assumed to be parent of qemu directory)
# --------------------------------------------------
cd ..

# --------------------------------------------------
# Build the ISO using the provided script and environment variables.
# Adjust PKGS_TO_INSTALL and JINX_CONFIG_FILE as needed.
# --------------------------------------------------
echo "Building ISO image..."
PKGS_TO_INSTALL="*" JINX_CONFIG_FILE=jinx-config-riscv64 ./build-support/makeiso.sh

# --------------------------------------------------
# Launch QEMU with the built ISO and firmware
# --------------------------------------------------
echo "Launching QEMU..."
qemu-system-riscv64 -M virt -cpu rv64 -smp 4 -device ramfb -device qemu-xhci \
    -device usb-kbd -device usb-mouse -drive if=pflash,unit=0,format=raw,file="$FIRMWARE_PATH" \
    -hda gloire.iso -serial stdio -m 4G

