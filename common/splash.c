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
#include <env.h>
#include <splash.h>
#include <lcd.h>
#include <mapmem.h>
#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_CMD_BMP)
#include <bmp_logo.h>
#include <bmp_logo_data.h>
#endif

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
void splash_get_pos(int *x, int *y)
{
	char *s = env_get("splashpos");

	if (!s)
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
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

/*
 * Common function to show a splash image if env("splashimage") is set.
 * Is used for both dm_video and lcd video stacks. For additional
 * details please refer to doc/README.splashprepare.
 *
 * Note: this is used for the *initial logo*, which Tow-Boot *forces*
 *       to be the embedded `logo.h` from LOGO_BMP, centered.
 */
#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_CMD_BMP)
int splash_display(void)
{
	int x = -1, y = -1;

	return bmp_display(map_to_sysmem(bmp_logo_bitmap), x, y);
}
#endif
