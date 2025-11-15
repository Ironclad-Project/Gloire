#! /bin/sh

for f in pkgs/*; do
    PKGPATH="$(realpath "$f")"
    TMPDIR="$(mktemp -d)"
    (
        cd "$TMPDIR"
        PRINT_PKG=0
        zstdcat < "$PKGPATH" | tar -xf -
        for ff in $(find ./usr/bin ./usr/lib ./usr/libexec -name '*.so*' -type f 2>/dev/null); do
            if ! readelf --dynamic "$ff" >/dev/null 2>&1; then
                continue
            fi
            if ! readelf --dynamic "$ff" | grep -q 'SONAME'; then
                echo "$ff has no soname"
                PRINT_PKG=1
            fi
        done
        if [ "$PRINT_PKG" = 1 ]; then
            echo "$f"
        fi
    )
    rm -rf "$TMPDIR"
done
