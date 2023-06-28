#!/usr/bin/env bash

set -e
set -u
PS4=" $ "

this="${BASH_SOURCE[0]%/*}"
. "$this/lib/main.sh"

if (( $# < 2 )); then
	echo "Usage: $0 <20yy.mm> <name>"
	exit 1
fi

RELEASE="$1"
shift
PREFIX="tow-boot/$RELEASE"

NAME="$1"
shift

git checkout "$PREFIX/$NAME"
