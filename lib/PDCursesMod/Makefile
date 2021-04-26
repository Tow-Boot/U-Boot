# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2021 Samuel Dionne-Riel
#

# This file only gets included when CONFIG_PDCURSES is set.

obj- += dummy.o

ifndef CONFIG_SPL_BUILD

#ccflags-y := -I$(srctree)/lib/PDCursesMod/include
#subdir-ccflags-y += -I$(srctree)/$(src)/
subdir-ccflags-y += -I$(srctree)/$(src)/include

obj-$(CONFIG_PDCURSES_DEMOS) += demos/simplemenu.o
obj-$(CONFIG_PDCURSES_DEMOS) += demos/tui.o
obj-$(CONFIG_PDCURSES_DEMOS) += demos/tuidemo.o

obj-y += pdcurses/addch.o
obj-y += pdcurses/addchstr.o
obj-y += pdcurses/addstr.o
obj-y += pdcurses/attr.o
obj-y += pdcurses/beep.o
obj-y += pdcurses/bkgd.o
obj-y += pdcurses/border.o
obj-y += pdcurses/clear.o
obj-y += pdcurses/color.o
obj-y += pdcurses/delch.o
obj-y += pdcurses/deleteln.o
obj-y += pdcurses/getch.o
obj-y += pdcurses/getstr.o
obj-y += pdcurses/getyx.o
obj-y += pdcurses/inch.o
obj-y += pdcurses/inchstr.o
obj-y += pdcurses/initscr.o
obj-y += pdcurses/inopts.o
obj-y += pdcurses/insch.o
obj-y += pdcurses/insstr.o
obj-y += pdcurses/instr.o
obj-y += pdcurses/kernel.o
obj-y += pdcurses/keyname.o
obj-y += pdcurses/mouse.o
obj-y += pdcurses/move.o
obj-y += pdcurses/outopts.o
obj-y += pdcurses/overlay.o
obj-y += pdcurses/pad.o
obj-y += pdcurses/panel.o
obj-y += pdcurses/printw.o
obj-y += pdcurses/refresh.o
obj-y += pdcurses/scanw.o
obj-y += pdcurses/scr_dump.o
obj-y += pdcurses/scroll.o
obj-y += pdcurses/slk.o
obj-y += pdcurses/termattr.o
obj-y += pdcurses/touch.o
obj-y += pdcurses/util.o
obj-y += pdcurses/window.o
obj-y += pdcurses/debug.o

obj-y += ansi/pdcclip.o
obj-y += ansi/pdcdisp.o
obj-y += ansi/pdcgetsc.o
obj-y += ansi/pdckbd.o
obj-y += ansi/pdcscrn.o
obj-y += ansi/pdcsetsc.o
obj-y += ansi/pdcutil.o

endif
