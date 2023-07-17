U-Boot Working Repository
=========================

Upstream can be found at:

 - https://source.denx.de/u-boot/u-boot/

Do not assume branches are in perfect working order, especially in isolation.

* * *

Repository organization
=======================

### Tags

Only the main release tags from U-Boot are ported over, as a convenience for
git operations.

Tags are created for Tow-Boot releases pointing to the tip of the generic
integration branch.

### Branches

Tow-Boot branches aimed at releasing are named with the prefix `tow-boot/20yy.mm`,
where `yy.mm` follows the U-Boot naming scheme.

A final generic integration branch merging other branches is named
`tow-boot/20yy.mm/_all`.

#### Feature branch name prefixes

Other branches under the prefix name are named (lowercased, dashes) according
to the change type and feature.

Feature branches coming from upstream contributions not yet merged are named
`upstream-$feature`.

Feature branches that can be upstreamed are named `base-$feature`. They
should be self-contained and not rely on other branches. If it does, it is
`wip-` until it's ready.

Feature branch names that cannot be upstreamed as-they-are, because it's WIP,
or because of the style of workarounds used, are prefixed `wip-`.
The changes should generally be acceptable by upstream, once they are
implemented correctly.

Feature branches names that cannot be upstreamed for opinionated reasons are
prefixed with `tb-$feature`. This means that they are additional
customizations for Tow-Boot that are unlikely to be desirable by upstream.

Family-specific feature branches are prefixed `soc-$family[-$feature]`. They
may or may not be upstreamable as a whole.

Board-specific feature branches are prefixed `board-$identifier[-$feature]`.
The feature name may be missing if the changes are trivial and require only
a single branch. They may or may not be upstreamable as a whole.

Branch prefixes not listed here are forbidden.

> **NOTE**: `wip-` branches are allowed to refer to any other branches for
> the release, when they build up on the features. The octopus merge strategy
> used to produce the `_all` branch should handle most situation without
> making a fuss. If it does not, it may mean the code is depending or
> conflicting on changes from another set of branches.
>
> Similarly, set of changes for an SoC family may require being merged into
> a base branch for the family.
>
> Care should be taken to prevent complex inter-dependencies when authoring
> changes.

The `_all` branch is merged with a script by sorting branches by type in the
same order as this list, and then sorted by name.

### Commits

External patches brought into the tree should include an `Origin: ` line
detailing the exact source used initially.

When rebasing, be mindful about changes from the downstream origin, and
rebase on top of those if there are any.
