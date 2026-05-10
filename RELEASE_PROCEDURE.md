# Release procedure for Gloire

Gloire has as version number its release date as `yyyymmdd`. We are a rolling
release distro, so the version number is just indicative of a snapshot of
the Gloire packages.

Gloire distributes monthly images. They contain extra packages on
top of `base` because base is meant to be as minimal as possible, for its use
in containers, or extremely slim installs, but these minimal package sets are
sometimes not practical for testing or daily use.

The monthlies have the following packages:

```
base pv openssh nano vim gloire-install fastfetch less
```

## Release steps

Prepare images with:

- `PKGS_TO_INSTALL="<the release packages>" IMAGE_NAME="gloire-yyyymmdd.iso" ./build-support/makeiso.sh`.
- Sign them with `gpg -b <image>`.
- Distribute them along with the Ironclad keyring, which should be the signer
keys.

Do this for every architecture, add `-<architecture>-` in between `.iso` and
`gloire-yyyymmdd` to mark architecture.

Make a tag named `v<yyyymmdd>` and release on said tag with the following
message:

-------------------------------------------------------------------------------

These images can be burned to a DVD, mounted as an ISO file, or be directly
written to a USB flash drive. It is intended for new installations only; an
existing Gloire system can always be updated with `xbps-install -Su`.

To burn the image to a device on a Linux system, one can use:

```bash
sh -c "cat <desired image> > /dev/<desired device>"
```

Otherwise they can be booted directly from QEMU with a command like:

```bash
# riscv
qemu-system-riscv64 -M virt,acpi=on -cpu rv64,svpbmt=on -device ramfb \
  -device qemu-xhci -m 4G -device usb-kbd -device usb-mouse -serial stdio \
  -drive if=pflash,unit=0,format=raw,file=<firmware image> \
  -drive id=disk,file=gloire.img,if=none -device ahci,id=ahci \
  -device ide-hd,drive=disk,bus=ahci.0

# x86_64
qemu-system-x86_64 -enable-kvm -cpu host -m 8G -M q35 -drive format=raw,file=gloire-<yyyymmdd>-x86_64.iso -serial stdio
```
To check the signature against the `.iso` file, one can use:

```bash
# gpg --import ironclad-keyring.gpg if you dont have the project's keyring.
gpg --verify <desired image>.sig
```

One can download `ironclad-keyring.gpg` as part of this release's files.
