#! /bin/sh

for f in pkgs/*; do
    PKGPATH="$(realpath "$f")"
    TMPDIR="$(mktemp -d)"
    (
        cd "$TMPDIR"
        PRINT_PKG=0
        zstdcat < "$PKGPATH" | tar -xf -
        for ff in $(find . -type f); do
            stuff="$(strings "$ff" | grep '/sysroot' | sort | uniq)"
            if [ -z "$stuff" ]; then
                continue
            fi
            printf "\e[44;97m------- vvv ------- $ff ------- vvv -------\e[m\n"
            echo "$stuff"
            PRINT_PKG=1
        done
        if [ "$PRINT_PKG" = 1 ]; then
            printf "\e[46;97m------- ^^^ ------- $f ------- ^^^ -------\e[m\n"
        fi
    )
    rm -rf "$TMPDIR"
done
