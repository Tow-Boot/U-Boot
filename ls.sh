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

_release_branches "tow-boot/$RELEASE"

if (( $(_wrong_branch_names "tow-boot/$RELEASE" | wc -l) > 0 )); then
	{
	echo ""
	echo "Warning: these branch names are invalid"
	} >&2
	_wrong_branch_names "tow-boot/$RELEASE"
fi
