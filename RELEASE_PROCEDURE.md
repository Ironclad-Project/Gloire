# Release procedure for Gloire

Gloire has as version number its release date as `yyyymmdd`. We are a rolling
release distro, so the version number is just indicative of a snapshot of
the Gloire packages.

## Release steps

Prepare images with:

- `PKGS_TO_INSTALL="*" ./build-support/make[iso,img].sh`.
- Rename the images: `gloire.[img, iso]` -> `gloire.yyyymmdd.[img,iso]`.
- Compress them with `xz < <image> > <image>.xz`.
- Sign them with `gpg -b <image>`.
- Distribute them along with the Ironclad keyring, which should be the signer
keys.

Do this for every architecture, add `-<architecture>-` in between `.img` and
`gloire-yyyymmdd` to mark architecture.

- Make announcements at https://blog.ironclad-os.org and other announcements.
