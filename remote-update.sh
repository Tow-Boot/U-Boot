#!/usr/bin/env bash

set -e
set -u
PS4=" $ "

this="${BASH_SOURCE[0]%/*}"
. "$this/lib/main.sh"

if (( $# < 2 )); then
	echo "Usage: $0 <remote name> <20yy.mm>"
	exit 1
fi

REMOTE="$1"; shift
PREFIX="tow-boot/$1"; shift

if (( $(_wrong_branch_names "$PREFIX" | wc -l) > 0 )); then
	{
	echo ""
	echo "Warning: these branch names are invalid, and will not be pushed:"
	_wrong_branch_names "$PREFIX" | sed -e 's/^/ - /'
	} >&2
fi

set -x

# shellcheck disable=2046
git push --force -u "$REMOTE" "$PREFIX/_all" $(_release_branches "$PREFIX")
