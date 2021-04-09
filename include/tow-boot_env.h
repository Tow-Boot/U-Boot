#ifndef _TOW_BOOT_ENV_H
#define _TOW_BOOT_ENV_H

#ifdef CONFIG_CMD_POWEROFF
#define TOW_BOOT_MAYBE_POWEROFF "tb_menu add 'Shutdown' 'Powers off the device' 'poweroff';"
#else
#define TOW_BOOT_MAYBE_POWEROFF
#endif

#ifdef CONFIG_CMD_USB
#define TOW_BOOT_MAYBE_USB "tb_menu add 'Rescan USB' 'Look for new USB devices' \"usb reset; $menucmd\";"
#else
#define TOW_BOOT_MAYBE_USB
#endif

#define TOW_BOOT_ENV \
	/* Used internally to check updater runs on the right board. */ \
	"board_identifier=@boardIdentifier@\0" \
	\
	/* By default, no setup for LEDs */ \
	"setup_leds=echo\0" \
	"bootcmd=run setup_leds; run distro_bootcmd\0" \
	"target_name_mmc0=" CONFIG_TOW_BOOT_MMC0_NAME "\0" \
	"target_name_mmc1=" CONFIG_TOW_BOOT_MMC1_NAME "\0" \
	"target_name_mmc2=" CONFIG_TOW_BOOT_MMC2_NAME "\0" \
	"target_name_usb0=USB\0" \
	"target_name_nvme0=NVME\0" \
	"target_name_pxe=PXE\0" \
	"target_name_dhcp=DHCP\0" \
	\
	\
	"towboot_menu=" \
	/* Builds the boot options in the same order distro_bootcmd uses. */ \
	"tb_menu new;" \
	"for target in ${boot_targets}; do" \
	/*   # Skip fel as it has no value for end-users */ \
	"    if test \"$target\" != fel; then" \
	"        env indirect target_name target_name_${target} \"(${target})\";" \
	"        tb_menu add \"Boot from ${target_name}\" '' \"run bootcmd_${target}; echo ${target_name} Boot failed.; pause; $menucmd\" ;" \
	"    fi;" \
	"done;" \
	\
	"tb_menu separator;" \
	TOW_BOOT_MAYBE_USB \
	/* In the future, this would be "Firmware Options". */ \
	"tb_menu add 'Firmware Console' 'Go to the console' 'cls; echo; echo \"Use `run menucmd` to return to the menu.\" ; echo' ;" \
	"tb_menu separator;" \
	/* `reset` is universal. Power off is optional */ \
	"tb_menu add 'Reboot' 'Reboots the device' 'reset';" \
	TOW_BOOT_MAYBE_POWEROFF \
	/* Show this menu! */ \
	"tb_menu show" \
	"\0" \
	\
	\
	"menucmd=run towboot_menu\0" \

#endif
