#!/bin/sh

set -ex

# Let the user pass their own sudo (or doas).
: "${SUDO:=sudo}"

if [ "$1" = "--erase-cache" ]; then
    $SUDO rm -rf .jinx-cache
fi

$SUDO rm -rf gloire.iso sysroot sources builds pkgs host-builds host-pkgs
