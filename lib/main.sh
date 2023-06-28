#!/usr/bin/env bash

# All branches for a prefix.
_get_branches() {
	local PREFIX="$1"; shift

	git branch --all --format '%(refname:lstrip=2)' --list "$PREFIX"/* \
		| sort
}

# All branches that produces the `_all` branches.
_release_branches() {
	local PREFIX="$1"; shift

	_get_branches "$PREFIX" \
		| grep -v "$PREFIX/_all"
}
