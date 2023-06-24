#!/usr/bin/env bash

set -e
set -u

if (( $# < 2 )); then
	echo "Usage: $0 <20yy.mm> <name> [base]"
	exit 1
fi

RELEASE="$1"
shift
PREFIX="tow-boot/$RELEASE"
UBOOT_TAG="v${RELEASE}"

NAME="$1"
shift

if (( $# > 0 )); then
	BASE="$1"
	shift
else
	BASE="$UBOOT_TAG"
fi

git checkout -B "$PREFIX/$NAME" "$BASE"
