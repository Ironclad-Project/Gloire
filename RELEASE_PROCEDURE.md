# Release procedure for Gloire

Gloire has as version number its release date as `yyyymmdd`. We are a rolling
release distro, so the version number is just indicative of a snapshot of
the Gloire packages.

## Release packages

The releases have the following packages:

```
base ace-of-penguins slim xorg-server st jwm fastfetch xfe mesa-demos
xorg-xeyes nyancat xorg-xwininfo xorg-xfontsel gloire-install mate-icon-theme
dbus xorg-xinit xf86-input-keyboard xf86-input-mouse xf86-video-fbdev gdb
metalog cronie nano vim
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
