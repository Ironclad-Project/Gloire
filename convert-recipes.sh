#!/bin/bash

pkg_recipes=($(ls -1 recipes))
host_recipes=($(ls -1 host-recipes))
source_recipes=($(ls -1 source-recipes))
keep_source_recipes="$(mktemp)"

do_recipe_src() {
    source $1

    if [ -z "$from_source" ]; then
        return
    fi

    if [ "$from_source" == "base-files" ]; then
        echo "* keeping $name because it uses base-files sources"
        echo $from_source >>"$keep_source_recipes"
        return
    fi

    if ! [ "$name" = "$from_source" ]; then
        echo "* keeping $name because it uses $from_source sources"
        echo $from_source >>"$keep_source_recipes"
        return
    fi
}

do_recipe() {
    source $1

    if [ -z "$from_source" ]; then
        return
    fi

    if grep -q $from_source "$keep_source_recipes"; then
        return
    fi

    pkg_imagedeps="$imagedeps"
    pkg_hostdeps="$hostdeps"
    pkg_hostrundeps="$hostrundeps"
    pkg_builddeps="$builddeps"
    pkg_deps="$deps"
    pkg_allow_network="$allow_network"

    unset imagedeps hostdeps hostrundeps builddeps deps allow_network
    source source-recipes/$from_source

    result="$(mktemp)"

    cat <<EOF >$result
name=$name
version=$version
revision=$revision
EOF

    if ! [ -z "$source_dir" ]; then
        echo "source_dir=\"$source_dir\"" >>$result
    else
        cat <<EOF >>$result
tarball_url="$tarball_url"
tarball_blake2b="$tarball_blake2b"
EOF
    fi

    [ -z "$imagedeps" ] || echo "source_imagedeps=\"$imagedeps\"" >>$result
    [ -z "$hostdeps" ] || echo "source_hostdeps=\"$hostdeps\"" >>$result
    [ -z "$hostrundeps" ] || echo "source_hostrundeps=\"$hostrundeps\"" >>$result
    [ -z "$allow_network" ] || echo "source_allow_network=\"$allow_network\"" >>$result
    [ -z "$pkg_imagedeps" ] || echo "imagedeps=\"$pkg_imagedeps\"" >>$result
    [ -z "$pkg_hostdeps" ] || echo "hostdeps=\"$pkg_hostdeps\"" >>$result
    [ -z "$pkg_hostrundeps" ] || echo "hostrundeps=\"$pkg_hostrundeps\"" >>$result
    [ -z "$pkg_builddeps" ] || echo "builddeps=\"$pkg_builddeps\"" >>$result
    [ -z "$pkg_deps" ] || echo "deps=\"$pkg_deps\"" >>$result
    [ -z "$pkg_allow_network" ] || echo "allow_network=\"$pkg_allow_network\"" >>$result

    regenerate_source="$(sed -n '/^regenerate() {$/,/^}$/p' source-recipes/$from_source)"
    build_source="$(sed -n '/^build() {$/,/^}$/p' $1)"
    package_source="$(sed -n '/^package() {$/,/^}$/p' $1)"

    cat <<EOF >>$result

$regenerate_source

$build_source

$package_source
EOF

    mv $result $1
}

for recipe in ${host_recipes[@]}; do
    (do_recipe_src host-recipes/$recipe)
done

for recipe in ${pkg_recipes[@]}; do
    (do_recipe_src recipes/$recipe)
done

for recipe in ${host_recipes[@]}; do
    (do_recipe host-recipes/$recipe)
done

for recipe in ${pkg_recipes[@]}; do
    (do_recipe recipes/$recipe)
done

for recipe in ${source_recipes[@]}; do
    if ! grep -q $recipe "$keep_source_recipes"; then
        echo "* removing $recipe because it is not used"
        rm source-recipes/$recipe
    fi
done
