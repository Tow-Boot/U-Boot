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
to the feature.

Feature branch names that cannot be upstreamed for opinionated reasons are
prefixed with `tb-`, meaning that they are additional customizations for
Tow-Boot that are unlikely to be desirable by upstream.

Board-specific feature branches are prefixed `board-$identifier[-$feature]`.
The feature name may be missing if the changes are trivial and require only
a single branch.

Feature branch names that cannot be upstreamed as-they-are, because of the
style of workarounds used, are prefixed `wip-`. The changes should generally
be acceptable by upstream, once they are implemented correctly.

> **NOTE**: `wip-` branches are allowed to refer to any other branches for
> the release, when they build up on the features. The octopus merge strategy
> used to produce the `_all` branch should handle most situation without
> making a fuss. If it does not, it may mean the code is depending or
> conflicting on changes from another set of branches.
>
> Care should be taken to prevent complex inter-dependencies when authoring
> changes.

### Commits

External patches brought into the tree should include an `Origin: ` line
detailing the exact source used initially.

When rebasing, be mindful about changes from the downstream origin, and
rebase on top of those if there are any.
