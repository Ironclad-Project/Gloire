<p align="center">
    <img height="300" alt="Logo of the distro" src="logo.png"/>
</p>

Gloire is an OS built with the
[Ironclad](https://github.com/streaksu/ironclad) kernel, and using GNU tools for
the userland. This repository holds scripts and tools to build the OS from the
ground.

Gloire is named after the french ironclad, which was the first ocean-going
ironclad.

## Building

The project uses `xbstrap`, which can be adquired easily from pip:

```bash
pip install xbstrap
```

From there, one can do the following to build the project:

```bash
make build && cd build # Prepare a directory and switch to it.
xbstrap init ..        # Arm xbstrap.
xbstrap install --all  # Tell xbstrap to install all the packages.
```

A bootable ISO image can be generated running `xbstrap run make-iso` script,
and it can be booted using a machine or QEMU with a command like
```bash
qemu-system-x86_64 -enable-kvm -cpu-host -m 2G -smp 4 -hda gloire.hdd
```

A list of the tools needed for compilation of everything are:

- `autoconf` 2.69 and `automake`.
- An Ada compiler, preferably `gcc`.
- A standard linker and GAS assembler.
- `xorriso` and QEMU for testing.
