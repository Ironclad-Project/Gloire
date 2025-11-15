#! /bin/sh

set -e

for f in pkgs/*; do tar -tvf $f | grep ' .*\.a$' && echo $f; done
