#! /bin/sh

set -e

for f in pkgs/*; do
    PKGPATH="$(realpath "$f")"
    TMPDIR="$(mktemp -d)"
    (
        cd "$TMPDIR"
        PRINT_PKG=0
        zstdcat < "$PKGPATH" | tar -xf -
        for ff in $(find ./usr/bin ./usr/lib ./usr/libexec -type f 2>/dev/null); do
            if ! readelf -d "$ff" >/dev/null 2>&1; then
                continue
            fi
            if readelf -d "$ff" | grep NEEDED | grep 'libc\.so\.6'; then
                PRINT_PKG=1
            fi
        done
        if [ "$PRINT_PKG" = 1 ]; then
            echo "$f"
        fi
    )
    rm -rf "$TMPDIR"
done
