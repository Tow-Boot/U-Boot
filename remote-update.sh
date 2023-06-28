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
TAG="$1"; shift

set -x

# shellcheck disable=2046
git push --force -u "$REMOTE" $("$this"/ls.sh "$TAG") "tow-boot/$TAG/_all"
