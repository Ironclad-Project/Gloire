#!/bin/bash

# ARG1 is path to a private key, which is a PEM-encoded RSA key that can be
# generated with `ssh-keygen -t rsa -m PEM -f private.pem -b 4096`

if [ -z "$1" ]; then
   echo 'pass an argument please'
   exit 1
fi

keysize="4096"
fingerprint="$(openssl rsa -in "$1" -pubout | \
	ssh-keygen -i -f /dev/stdin -m PKCS8 | \
	cut -d ' ' -f 2 | base64 -d | openssl md5 -c | cut -d ' ' -f 2)"
cat >"${fingerprint}.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>public-key</key>
	<data>$(openssl rsa -in "$1" -pubout | base64 -w 0)</data>
	<key>public-key-size</key>
	<integer>$keysize</integer>
	<key>signature-by</key>
	<string>Gloire development team</string>
</dict>
</plist>
EOF
