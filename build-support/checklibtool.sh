#! /bin/sh

set -e

for f in pkgs/*; do tar -tvf $f | grep ' .*\.la$' && echo $f; done
