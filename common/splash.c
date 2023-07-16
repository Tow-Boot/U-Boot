/*
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., http://www.fsf.org/about/contact/
 *
 */

#include <common.h>
#include <display_options.h>
#include <env.h>
#include <mapmem.h>
#include <splash.h>
#include <video.h>

void splash_get_pos(int *x, int *y)
{
	char *s = env_get("splashpos");

	if (!CONFIG_IS_ENABLED(SPLASH_SCREEN_ALIGN) || !s)
		return;

	if (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	else
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	if (s != NULL) {
		if (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		else
			*y = simple_strtol(s + 1, NULL, 0);
	}
}

#if CONFIG_IS_ENABLED(VIDEO)
#ifdef CONFIG_VIDEO_LOGO
#include <bmp_logo.h>
#include <bmp_logo_data.h>
#endif
#endif

#if CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)

#include <dm.h>
#include <video_console.h>
#include <video_font.h>
#include <video_font_data.h>

void splash_display_banner(void)
{
	struct video_fontdata __maybe_unused *fontdata = fonts;
	struct udevice *dev;
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	int col, row, ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &dev);
	if (ret)
		return;

#if IS_ENABLED(CONFIG_VIDEO_LOGO)
	col = BMP_LOGO_WIDTH / fontdata->width + 1;
	row = BMP_LOGO_HEIGHT / fontdata->height + 1;
#else
	col = 0;
	row = 0;
#endif

	display_options_get_banner(false, buf, sizeof(buf));
	vidconsole_position_cursor(dev, col, 1);
	vidconsole_put_string(dev, buf);
	vidconsole_position_cursor(dev, 0, row);
}
#endif /* CONFIG_VIDEO && !CONFIG_HIDE_LOGO_VERSION */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * For additional details please refer to doc/README.splashprepare.
 */
int splash_display(void)
{
	// Clamp to bottom right
	int x = BMP_ALIGN_END, y = BMP_ALIGN_END;

#if CONFIG_IS_ENABLED(VIDEO) && !CONFIG_IS_ENABLED(HIDE_LOGO_VERSION)
	splash_display_banner();
#endif
	return bmp_display(map_to_sysmem(bmp_logo_bitmap), x, y);
}
