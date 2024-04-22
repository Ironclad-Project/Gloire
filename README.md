<p align="center">
    <img height="300" alt="Logo of the distro" src="artwork/logo.png"/>
</p>

# Gloire

Gloire is an OS built with the [Ironclad](https://ironclad.cx)
kernel and using GNU tools for the userland, along with some original
applications like `util-ironclad`. This repository holds scripts and tools to
build the OS from the ground up on a Linux-based system.

Gloire is named after the [french ironclad](https://en.wikipedia.org/wiki/French_ironclad_Gloire),
which was the first ocean-going vessel of its kind.

![Gloire running the JWM window manager, a terminal emulator, xeyes, and a game](artwork/screenshot1.png)
*Gloire running the JWM window manager, a terminal emulator, xeyes, and a game*
![Gloire generating an RSA key and running neofetch in its fallback shell](artwork/screenshot2.png)
*Gloire generating an RSA key and running neofetch in its fallback shell.*

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
one can burn your gloire image (uncompressing it first if downloaded) to a
SATA or ATA drive. USB sticks for booting are not supported
(for now, stay posted!).

## Contributing

Gloire accepts contributions for new packages or any other kind of changes
using the pull request system baked into Github.
Please [submit PRs here](https://github.com/streaksu/Gloire/pulls) or read
our documentation on how to do so and some things to keep in mind porting on
[the project's wiki](https://github.com/streaksu/Gloire/wiki).

## Building

The project uses `jinx` as its build system, which is included in the tree.
The instructions to build are:

```bash
./jinx build-all           # Build all packages.
./build-support/makeiso.sh # Create the image.
```

These commands will generate a bootable disk image that can be burned to
storage media or be booted by several emulators.

A list of the tools needed for compilation of the OS are:

- `git` for cloning packages.
- `curl` and `bsdtar` for setting up jinx.
- Common UNIX tools like `bash`, `coreutils`, `grep`, `find`, etc.
- `sgdisk` from the `gptfdisk` package for building the image.
- `qemu` for testing, if wanted.
- `tar` and `lzip` for extracting packages.
- `rsync` for building bootable images.

All of said things can be installed in debian-based systems with

```bash
sudo apt install lzip libarchive-tools git build-essentials
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
