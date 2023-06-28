#!/usr/bin/env bash

set -e
set -u
PS4=" $ "

this="${BASH_SOURCE[0]%/*}"
. "$this/lib/main.sh"

if (( $# < 2 )); then
	echo "Usage: $0 <remote name> <branch prefix>"
	exit 1
fi

REMOTE="$1"; shift
PREFIX="$1"; shift

set -x

git fetch -p "$REMOTE"
git branch -r \
	| grep "$REMOTE"/"$PREFIX"/ \
	| grep -v '_all' \
	| cut -d'/' -f2- \
	| xargs -I {} git push "$REMOTE" :{}
