#!/bin/bash

# ARG1 is path to private key, which is a PEM-encoded RSA key that can be
# generated with `ssh-keygen -t rsa -m PEM -f private.pem -b 4096`

if [ -z "$1" ]; then
   echo 'pass an argument please'
   exit 1
fi

rm -rf pkgs/*.sig2

xbps-rindex --privkey "$1" --sign --signedby "Gloire development team" pkgs
xbps-rindex --privkey "$1" --sign-pkg --signedby "Gloire development team" pkgs/*.xbps

