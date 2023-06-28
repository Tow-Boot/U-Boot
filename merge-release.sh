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

RELEASE="$1"
shift
PREFIX="tow-boot/$RELEASE"
UBOOT_TAG="v${RELEASE}"

branches() {
	git branch --all --format '%(refname:lstrip=2)' --list "$PREFIX"/* \
		| grep -v "$PREFIX/_all" \
		| sort
}

branch_details() {
	for branch in $(branches); do
		printf " - %s ⇒ %s\n" \
			"$(git rev-parse "$branch")" \
			"$branch"
	done
}

message() {
cat <<EOF
Integration branch for Tow-Boot ${RELEASE}

This branch includes changes from the following $(branches | wc -l) branches:

$(branch_details)

Including contributions from:
$(
	git shortlog --summary --numbered --no-merges \
		--group=author --group=trailer:co-authored-by \
		"${UBOOT_TAG}..${PREFIX}/_all" \
		| sed -e 's/^/ - /'
)

EOF
}

git checkout -B "$PREFIX/_all" "$UBOOT_TAG"
#shellcheck disable=SC2046
git merge --no-ff $(branches) -m "[TBD]"
git commit --amend -m "$(message)"
