<p align="center">
    <img height="300" alt="Logo of the distro" src="artwork/logo.png"/>
</p>

# Gloire

Gloire is an OS built with the [Ironclad](https://ironclad.cx)
kernel and using GNU tools for the userland, along with some original
applications like `gwm`. This repository holds scripts and tools to build the
OS from the ground up.

Gloire is named after the [french ironclad](https://en.wikipedia.org/wiki/French_ironclad_Gloire),
which was the first ocean-going vessel of its kind.

![Gloire running its window manager, GWM](artwork/screenshot1.png)
*Gloire running its window manager, GWM.*
![Gloire generating an RSA key and running neofetch](artwork/screenshot2.png)
*Gloire generating an RSA key and running neofetch.*

## Downloading

One can grab a pre-built Gloire image [here](https://github.com/streaksu/Gloire/releases).

## Running

### On virtual machines

One can run either the downloaded disk image (uncompressing it first) or a
built image with a command as such:

```bash
qemu-system-x86_64 -enable-kvm -m 2G -M q35 -hda gloire.img
```

Where `gloire.img` is your image of choice.

### On physical hardware

Gloire should run fine on any x86 machine, be it UEFI or BIOS. For running it,
one can burn your gloire image (uncompressing it first if downloaded) to either
a USB drive featuring HDD emulation, a SATA drive, or an ATA drive. USB without
HDD emulation is not supported as of yet.

## Building

The project uses `jinx` as its build system, which is included in the tree.
The instructions to build are:

```bash
./jinx build-all           # Build all packages.
./build-support/makeiso.sh # Create the image.
```

This commands will generate a bootable disk image that can be burned to
USB mass-storage media or be booted by several emulators.

A list of the tools needed for compilation of the OS are:

- `git` for cloning packages.
- `curl` and `bsdtar` for setting up jinx.
- Common UNIX tools like `bash`, `coreutils`, `grep`, `find`, etc.
- `sgdisk` from the `gptfdisk` package for building the image.
- `qemu` for testing, if wanted.

All of said things can be installed in debian-based systems with

```bash
sudo apt install libarchive-tools git build-essentials
```

## Licensing

A list of the licenses used by the software ported to Gloire is:

- [GPLv3 (Or-Later and Only)](https://www.gnu.org/licenses/gpl-3.0.html)
- [GPLv2 (Or-Later and Only)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)
- [The Less License](https://github.com/gwsw/less/blob/master/LICENSE)
- [MIT License](https://opensource.org/licenses/MIT)
- [BSD 2-Clause](https://opensource.org/licenses/BSD-2-Clause)

## Thanks to

- [Mintsuki](https://github.com/mintsuki) for the limine bootloader and `jinx`.
- [The managarm project](https://github.com/managarm) for help with some
of the recipes and `mlibc`.
