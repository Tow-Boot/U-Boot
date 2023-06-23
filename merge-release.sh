#!/usr/bin/env bash

set -e
set -u

if (( $# < 1 )); then
	echo "Usage: $0 <20yy.mm>"
	exit 1
fi

RELEASE="$1"
shift
PREFIX="tow-boot/$RELEASE"

branches() {
	git branch --all --format '%(refname:lstrip=2)' --list "$PREFIX"/* \
		| grep -v "$PREFIX/_all" \
		| sort
}

message() {
cat <<EOF
Integration branch for Tow-Boot ${RELEASE}

This branch includes changes from the following branches:

$(branches | sed -e 's/^/ - /')

EOF
}

git checkout -B "$PREFIX/_all" "v${RELEASE}"
#shellcheck disable=SC2046
git merge --no-ff $(branches) -m "$(message)"
