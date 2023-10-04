// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Samuel Dionne-Riel <samuel@dionne-riel.com>
 * Copyright (c) 2017 Google, Inc
 * Largely derived from `cmd/led.c`
 * Original written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <vibrator.h>
#include <dm/uclass-internal.h>

static const char *const state_label[] = {
	[VIBRATOR_STATE_OFF]	= "off",
	[VIBRATOR_STATE_ON]	= "on",
	[VIBRATOR_STATE_TOGGLE]	= "toggle",
};

enum vibrator_state_t get_vibrator_cmd(char *var)
{
	int i;

	for (i = 0; i < VIBRATOR_STATE_COUNT; i++) {
		if (!strncmp(var, state_label[i], strlen(var)))
			return i;
	}

	return -1;
}

static int show_vibrator_state(struct udevice *dev)
{
	int ret;

	ret = vibrator_get_state(dev);
	if (ret >= VIBRATOR_STATE_COUNT)
		ret = -EINVAL;
	if (ret >= 0)
		printf("%s\n", state_label[ret]);

	return ret;
}

static int list_vibrators(void)
{
	struct udevice *dev;
	int ret;

	for (uclass_find_first_device(UCLASS_VIBRATOR, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct vibrator_uc_plat *plat = dev_get_uclass_plat(dev);

		if (!plat->label)
			continue;
		printf("%-15s ", plat->label);
		if (device_active(dev)) {
			ret = show_vibrator_state(dev);
			if (ret < 0)
				printf("Error %d\n", ret);
		} else {
			printf("<inactive>\n");
		}
	}

	return 0;
}

int timed_vibration(struct udevice *dev, int duration_ms)
{
	int ret;
	ret = vibrator_set_state(dev, VIBRATOR_STATE_ON);
	if (ret < 0) {
		printf("Vibrator operation failed (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	udelay(duration_ms * 1000);

	ret = vibrator_set_state(dev, VIBRATOR_STATE_OFF);
	if (ret < 0) {
		printf("Vibrator operation failed (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
}

int do_vibrator(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	enum vibrator_state_t cmd;
	const char *vibrator_label;
	struct udevice *dev;
	int ret;
	int duration_ms = 0;

	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;
	vibrator_label = argv[1];
	if (strncmp(vibrator_label, "list", 4) == 0)
		return list_vibrators();

	cmd = argc > 2 ? get_vibrator_cmd(argv[2]) : VIBRATOR_STATE_COUNT;
	ret = vibrator_get_by_label(vibrator_label, &dev);
	if (ret) {
		printf("Vibrator '%s' not found (err=%d)\n", vibrator_label, ret);
		return CMD_RET_FAILURE;
	}

	if (strncmp(argv[2], "timed", 5) == 0) {
		if (argc < 4)
			return CMD_RET_USAGE;
		duration_ms = dectoul(argv[3], NULL);

		return timed_vibration(dev, duration_ms);
	}

	switch (cmd) {
	case VIBRATOR_STATE_OFF:
	case VIBRATOR_STATE_ON:
	case VIBRATOR_STATE_TOGGLE:
		ret = vibrator_set_state(dev, cmd);
		break;
	case VIBRATOR_STATE_COUNT:
		printf("Vibrator '%s': ", vibrator_label);
		ret = show_vibrator_state(dev);
		break;
	}
	if (ret < 0) {
		printf("Vibrator '%s' operation failed (err=%d)\n", vibrator_label, ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

U_BOOT_CMD(
	vibrator, 4, 1, do_vibrator,
	"manage VIBRATORs",
	"<vibrator_label> on|off\tChange VIBRATOR state\n"
	"vibrator <vibrator_label> timed <ms duration>\t\tVibrate for the given duration (will block)\n"
	"vibrator <vibrator_label>\tGet VIBRATOR state\n"
	"vibrator list\t\tShow a list of VIBRATORs"
);
