#!/bin/sh

set -ex

script_dir="$(dirname "$0")"
test -z "${script_dir}" && script_dir=.

source_dir="$(cd "${script_dir}"/.. && pwd -P)"

cd "${source_dir}"

# Let the user pass their own sudo (or doas).
: "${SUDO:=sudo}"

if [ "$1" = "--erase-cache" ]; then
    $SUDO rm -rf .jinx-cache
fi

$SUDO rm -rf build-x86_64 build-riscv64 sources
