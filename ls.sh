#!/usr/bin/env bash

set -e
set -u
PS4=" $ "

this="${BASH_SOURCE[0]%/*}"
. "$this/lib/main.sh"

if (( $# < 1 )); then
	echo "Usage: $0 <20yy.mm>"
	exit 1
fi

RELEASE="$1"; shift

_release_branches "tow-boot/$RELEASE" | grep -v '/_all$'
