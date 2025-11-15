#! /bin/sh

for f in pkgs/*; do
    PKGPATH="$(realpath "$f")"
    TMPDIR="$(mktemp -d)"
    (
        cd "$TMPDIR"
        PRINT_PKG=0
        zstdcat < "$PKGPATH" | tar -xf -
        for ff in $(find . -type f); do
            stuff="$(strings "$ff" | grep '/sysroot' | grep -v Assertion | grep -v '\--enable-languages')"
            if [ -z "$stuff" ]; then
                continue
            fi
            echo "    $ff"
            PRINT_PKG=1
        done
        if [ "$PRINT_PKG" = 1 ]; then
            echo "$f"
        fi
    )
    rm -rf "$TMPDIR"
done
