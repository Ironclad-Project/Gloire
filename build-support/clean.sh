#!/bin/sh

set -ex

script_dir="$(dirname "$0")"
test -z "${script_dir}" && script_dir=.

source_dir="$(cd "${script_dir}"/.. && pwd -P)"

cd "${source_dir}"

# Only directories without write or search permission can stop rm. Each gets
# its own chmod, so that find can go into it afterwards.
force_remove() {
    rm -rf "$@" 2>/dev/null && return
    find "$@" -type d ! -perm -700 -exec chmod u+rwx {} \; 2>/dev/null || true
    rm -rf "$@"
}

if [ "$1" = "--erase-cache" ]; then
    force_remove .jinx-cache .jinx-fetched
fi

force_remove build-x86_64 build-riscv64 sources host-sources
