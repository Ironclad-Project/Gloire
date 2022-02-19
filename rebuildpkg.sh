#!/usr/bin/env bash

set -e

BASE_DIR="$(pwd)"

if [ -z "$1" ]; then
    echo "Usage: $0 <package dir> [<package name>] [--tool]"
    exit 1
fi

set -x

if [ -z "$2" ]; then
    PKG_NAME="$1"
else
    if [ "$2" = "--tool" ]; then
        IS_TOOL="-tool"
        PKG_NAME="$1"
    else
        PKG_NAME="$2"
    fi
fi

if [ "$3" = "--tool" ]; then
    IS_TOOL="-tool"
fi

[ -z "$IS_TOOL" ] && rm -rf "$BASE_DIR"/build/pkg-builds/$1
[ -z "$IS_TOOL" ] || rm -rf "$BASE_DIR"/build/tool-builds/$PKG_NAME

if [ -d ports/$1 ]; then
    ( cd ports/$1
    [ -f "$BASE_DIR"/patches/$1/0001-Ironclad-specific-changes.patch ] && (
        git reset HEAD~1
    )
    git checkout .
    git clean -ffd
    touch checkedout.xbstrap fetched.xbstrap
    cd ../../build
    xbstrap patch $1 )
else
    ( cd build
    xbstrap fetch $1
    xbstrap checkout $1
    xbstrap patch $1 )
fi

[ -d ports/$1-workdir ] || (
    cp -r "$BASE_DIR"/ports/$1 "$BASE_DIR"/ports/$1-workdir
    rm -f "$BASE_DIR"/ports/$1-workdir/*.xbstrap
)

cd ports/$1-workdir
[ -f "$BASE_DIR"/patches/$1/0001-Ironclad-specific-changes.patch ] && (
    git reset HEAD~1
    ( cd ../$1 && git reset HEAD~1 )
)
( cd ../$1 && git checkout . && git clean -ffd && touch checkedout.xbstrap fetched.xbstrap )
git add .
git commit --allow-empty -m "Ironclad specific changes"
git format-patch -1
if [ -s 0001-Ironclad-specific-changes.patch ]; then
    mkdir -p "$BASE_DIR"/patches/$1
    mv 0001-Ironclad-specific-changes.patch "$BASE_DIR"/patches/$1/
else
    rm 0001-Ironclad-specific-changes.patch
    git reset HEAD~1
fi

cd "$BASE_DIR"/build
xbstrap patch $1
xbstrap regenerate $1

xbstrap install$IS_TOOL -u $PKG_NAME
