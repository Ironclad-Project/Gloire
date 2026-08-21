#! /bin/sh

set -e

script_dir="$(dirname "$0")"
test -z "${script_dir}" && script_dir=.

base_dir="$(cd "${script_dir}"/.. && pwd -P)"

for pkg in ../recipes/*; do
    unset repology_id
    unset repology_srcname
    unset repology_status
    unset repology_version
    unset skip_pkg_check

    . "${pkg}"/recipe

    pkg=${pkg##*/}
    printf "checking ${pkg} ..."

    if [ "$skip_pkg_check" = "yes" ]; then
        printf ' \033[0;37;100mskipped\033[0m\n'
        continue
    fi

    if [ -z "$repology_id" ]; then
        name_to_check="${pkg}"
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
        status_to_check=".status==\"$repology_status\" and"
    fi

    if [ -z "$repology_version" ]; then
        version_to_check="$version"
    else
        version_to_check="$repology_version"
    fi

    # XXX: Repology seems to implement a whitelist of agents, which does not
    # seem to include curl, or Gloire's repository, which is recommended under
    # their bulk user guidelines.
    # It is a biiiit dirty, but we will disguise ourselves as mozilla to bypass
    # this while we get in communication with the repology people for a
    # recommended approach.
    repology_response="$(curl -s -A 'Mozilla/5.0 (X11; Linux x86_64; rv:153.0) Gecko/20100101 Firefox/153.0' https://repology.org/api/v1/project/$name_to_check)"

    checked_vers=$(echo "$repology_response" | jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' (.repo=="arch" or .repo=="nix_unstable" or .repo=="chimera" or .repo=="homebrew")).version' | grep -v '"HEAD"' | sort -Vr | head -n 1)
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

    if ! [ "$version_to_check" = "$checked_vers" ]; then
        if ! [ "$(printf "$checked_vers\n$version_to_check\n" | sort -Vr | head -n 1)" = "$version_to_check" ]; then
            printf " \033[0;97;42mneeds update $version_to_check -> $checked_vers\033[0m\n"
        else
            printf " \033[0;97;44mmore up-to-date than detected ($version_to_check vs $checked_vers)\033[0m\n"
        fi
        continue
    fi

    printf "\33[2K\r"
done
