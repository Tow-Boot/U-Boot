#!/usr/bin/env bash

set -e
set -u

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