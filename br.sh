#!/bin/bash
set -e
PKGS_TO_INSTALL="" JINX_CONFIG_FILE=jinx-config-riscv64 ./jinx rebuild ironclad
PKGS_TO_INSTALL="" JINX_CONFIG_FILE=jinx-config-riscv64 ./build-support/makeiso.sh
./run.sh
