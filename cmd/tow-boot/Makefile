# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2021 Samuel Dionne-Riel
#

# This file is always included.

subdir-ccflags-y := -I$(srctree)/lib/PDCursesMod/include
subdir-ccflags-y += -I$(srctree)/$(src)/include

obj- += dummy.o

ifndef CONFIG_SPL_BUILD

obj-$(CONFIG_TOW_BOOT_MENU) += tow-boot_menu.o

endif
