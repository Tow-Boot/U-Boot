#!/usr/bin/env bash

# Order of the release prefixes
_RELEASE_PREFIXES=(
	upstream-
	base-
	wip-
	tb-
	soc-
	board-
)

_ALLOWED_EXTRA_PREFIXES=(
	"${_RELEASE_PREFIXES[@]}"
	_all
)

_RELEASE_PREFIXES_PATTERN=""
_ALLOWED_EXTRA_PREFIXES_PATTERN=""

# ============================================================================

# All branches for a prefix.
_get_branches() {
	local RELEASE="$1"; shift

	git branch --all --format '%(refname:lstrip=2)' --list "$RELEASE"/* \
		| sort
}

# All branches that produces the `_all` branches.
_release_branches() {
	local RELEASE="$1"; shift
	# We are not using the pattern here as we want to order them
	# in the same order they are merged in.
	for branch_prefix in "${_ALLOWED_EXTRA_PREFIXES[@]}"; do
		_get_branches "$RELEASE" | grep "/$branch_prefix" || :
	done
}

_wrong_branch_names() {
	local RELEASE="$1"; shift
	_get_branches "$RELEASE" | grep -v "${_ALLOWED_EXTRA_PREFIXES_PATTERN}"
}

_ALLOWED_EXTRA_PREFIXES_PATTERN="$(
	_sep=""
	for branch_prefix in "${_ALLOWED_EXTRA_PREFIXES[@]}"; do
		printf "%s%s" "$_sep" "/$branch_prefix"
		_sep="\\|"
	done
)"

_RELEASE_PREFIXES_PATTERN="$(
	_sep=""
	for branch_prefix in "${_RELEASE_PREFIXES[@]}"; do
		printf "%s%s" "$_sep" "/$branch_prefix"
		_sep="\\|"
	done
)"
