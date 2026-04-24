# Release procedure for Gloire

Gloire has as version number its release date as `yyyymmdd`. We are a rolling
release distro, so the version number is just indicative of a snapshot of
the Gloire packages.

## Release packages

Gloire distributes nightlies and full images. They contain extra packages on
top of `base` because base is meant to be as minimal as possible, for its use
in containers, or extremely slim installs, but these minimal package sets are
sometimes not practical for testing or daily use.

### Nightlies

Nightlies feature a reduced package set to speed up building and testing cycles,
which is their main purpose.

They have the following packages:

```
base pv openssh
```

### Full releases

The releases have the following packages:

```
base slim xorg-server fastfetch mesa-demos xorg-xeyes xorg-xwininfo xorg-xfontsel gloire-install dbus xorg-xinit xf86-input-keyboard xf86-input-mouse xf86-video-fbdev metalog cronie nano vim sed file gawk tar mate ttf-dejavu pv openssh
```

## Release steps

Prepare images with:

- `PKGS_TO_INSTALL="<the release packages>" ./build-support/makeimg.sh`.
- Rename the images: `gloire.img` -> `gloire.yyyymmdd.img`.
- Compress them with `xz < <image> > <image>.xz`.
- Sign them with `gpg -b <image>`.
- Distribute them along with the Ironclad keyring, which should be the signer
keys.

Do this for every architecture, add `-<architecture>-` in between `.img` and
`gloire-yyyymmdd` to mark architecture.

- Make announcements at https://blog.ironclad-os.org and other announcements.
