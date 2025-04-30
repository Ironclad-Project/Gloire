#!/bin/sh

# Option A (debugging turned on):
# set -ex
# set -euo pipefail

# Option B (quiet, but still safe):
set -euo pipefail

# Let the user pass their own $SUDO (or doas).
: "${SUDO:=sudo}"

if [ "$#" -ne 1 ]; then
    echo "Error: incorrect number of arguments"
    echo "Usage: $0 [x86|riscv]"
    exit 1
fi

ENVIRONMENT="$1"

case "$ENVIRONMENT" in
    x86)
        export PKGS_TO_INSTALL=""
        unset JINX_CONFIG_FILE
        ./jinx rebuild ironclad
        ./build-support/makeiso.sh
        ;;
    riscv)
        export PKGS_TO_INSTALL="" JINX_CONFIG_FILE=jinx-config-riscv64
        ./jinx rebuild ironclad
        ./build-support/makeiso.sh
        ;;
    *)
        echo "Error: Unrecognized parameter '$ENVIRONMENT'. Use 'x86' or 'riscv'."
        exit 1
        ;;
esac
