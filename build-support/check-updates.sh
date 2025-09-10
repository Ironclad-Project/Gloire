#! /bin/sh

set -e

base_dir="$(pwd -P)"

for pkg in "$@"; do
    unset repology_id
    unset repology_srcname
    unset repology_status
    unset skip_pkg_check

    . "${pkg}"

    printf "checking $name ..."

    if [ "$skip_pkg_check" = "yes" ]; then
        printf ' \033[0;37;100mskipped\033[0m\n'
        continue
    fi

    if [ -z "$repology_id" ]; then
        name_to_check="$name"
    else
        name_to_check="$repology_id"
    fi

    if [ -z "$repology_srcname" ]; then
        srcname_to_check=""
    else
        srcname_to_check=".srcname==\"$repology_srcname\" and"
    fi

    if [ -z "$repology_status" ]; then
        status_to_check=""
    else
        status_to_check=".status == \"$repology_status\" and"
    fi
    sleep .5
    repology_response="$(curl -s -A '' https://repology.org/api/v1/project/$name_to_check)"
    checked_vers=$(echo "$repology_response" | jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' (.repo=="arch" or .repo=="nix_unstable" or .repo=="chimera")).version' | sort -Vr | head -n 1)
    if [ -z "$checked_vers" ]; then
        checked_vers=$(echo "$repology_response" | jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' .repo=="alpine_edge").version' | sort -Vr | head -n 1)
    fi
    if [ -z "$checked_vers" ]; then
        checked_vers=$(echo "$repology_response" | jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' .repo=="debian_unstable").version' | sort -Vr | head -n 1)
    fi
    if [ -z "$checked_vers" ]; then
        checked_vers=$(echo "$repology_response" | jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' .repo=="aur").version' | sort -Vr | head -n 1)
    fi
    if [ -z "$checked_vers" ]; then
        printf " \033[0;97;101mis not checkable\033[0m\n"
        continue
    fi

    checked_vers="$(echo "$checked_vers" | sed 's/\"//g')"

    if ! [ "$version" = "$checked_vers" ]; then
        if ! [ "$(printf "$checked_vers\n$version\n" | sort -Vr | head -n 1)" = "$version" ]; then
            printf " \033[0;97;42mneeds update $version -> $checked_vers\033[0m\n"
        else
            printf " \033[0;97;44mmore up-to-date than detected ($version vs $checked_vers)\033[0m\n"
        fi
        continue
    fi

    printf "\33[2K\r"
done
