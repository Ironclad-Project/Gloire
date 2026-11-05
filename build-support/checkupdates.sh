#! /bin/sh

set -e

script_dir="$(dirname "$0")"
test -z "${script_dir}" && script_dir=.

base_dir="$(cd "${script_dir}"/.. && pwd -P)"

# Repology's API terms require a User-Agent linking to our repository.
api_url="https://repology.org/api/v1"
user_agent="gloire-checkupdates/1 (+https://codeberg.org/Ironclad/Gloire)"

tmp_dir="$(mktemp -d)"
trap 'rm -rf "${tmp_dir}"' EXIT
trap 'exit 1' HUP INT TERM
response_file="${tmp_dir}/response.json"

# Fetch Repology project $1 into ${response_file}, or print why it failed.
# Leaving out --fail keeps curl from retrying errors such as 403.
fetch_project() {
    if ! http_code="$(curl -sS --compressed -A "${user_agent}" \
            --connect-timeout 20 --max-time 60 \
            --retry 5 --retry-all-errors --retry-max-time 120 \
            -o "${response_file}" -w '%{http_code}' \
            "${api_url}/project/$1" 2>"${tmp_dir}/curl.log")"; then
        tail -n 1 "${tmp_dir}/curl.log"
        return 1
    fi
    if [ "${http_code}" != 200 ]; then
        echo "HTTP ${http_code}"
        return 1
    fi
    if ! jq -e 'type == "array"' "${response_file}" >/dev/null 2>&1; then
        echo "response is not a JSON list"
        return 1
    fi
}

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

    # Repology allows at most one API request per second.
    sleep 1

    # Errors that outlast curl's retries would hit the remaining packages too.
    if ! fetch_error="$(fetch_project "$name_to_check")"; then
        printf ' \033[0;97;101mfetch failed: %s\033[0m\n' "$fetch_error"
        echo "error: Repology is unreachable or refusing requests. Try again later." 1>&2
        exit 1
    fi

    checked_vers=$(jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' (.repo=="arch" or .repo=="nix_unstable" or .repo=="chimera" or .repo=="homebrew")).version' "$response_file" | grep -v '"HEAD"' | sort -Vr | head -n 1)
    if [ -z "$checked_vers" ]; then
        checked_vers=$(jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' .repo=="alpine_edge").version' "$response_file" | sort -Vr | head -n 1)
    fi
    if [ -z "$checked_vers" ]; then
        checked_vers=$(jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' .repo=="debian_unstable").version' "$response_file" | sort -Vr | head -n 1)
    fi
    if [ -z "$checked_vers" ]; then
        checked_vers=$(jq '.[] | select('"$status_to_check"' '"$srcname_to_check"' .repo=="aur").version' "$response_file" | sort -Vr | head -n 1)
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
