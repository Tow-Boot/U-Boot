// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Vasily Khoruzhick <anarsoul@gmail.com>
 * (C) Copyright 2021 Martijn Braam <martijn@brixit.nl>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <spl_gpio.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/gpio.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/misc.h>
#include <linux/delay.h>

#define GRF_IO_VSEL_BT565_SHIFT 0
#define PMUGRF_CON0_VSEL_SHIFT 8

#define GPIO3_BASE	0xff788000
#define GPIO4_BASE	0xff790000

#ifdef CONFIG_MISC_INIT_R
static void setup_iodomain(void)
{
	struct rk3399_grf_regs *grf =
	    syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	struct rk3399_pmugrf_regs *pmugrf =
	    syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);

	/* BT565 is in 1.8v domain */
	rk_setreg(&grf->io_vsel, 1 << GRF_IO_VSEL_BT565_SHIFT);

	/* Set GPIO1 1.8v/3.0v source select to PMU1830_VOL */
	rk_setreg(&pmugrf->soc_con0, 1 << PMUGRF_CON0_VSEL_SHIFT);
}

int misc_init_r(void)
{
	const u32 cpuid_offset = 0x7;
	const u32 cpuid_length = 0x10;
	u8 cpuid[cpuid_length];
	int ret;

	setup_iodomain();

	ret = rockchip_cpuid_from_efuse(cpuid_offset, cpuid_length, cpuid);
	if (ret)
		return ret;

	ret = rockchip_cpuid_set(cpuid, cpuid_length);
	if (ret)
		return ret;

	ret = rockchip_setup_macaddr();

	return ret;
}

void led_setup(void)
{
	struct rockchip_gpio_regs * const gpio3 = (void *)GPIO3_BASE;
	struct rockchip_gpio_regs * const gpio4 = (void *)GPIO4_BASE;

	// Light up the red LED
	// <&gpio4 RK_PD2 GPIO_ACTIVE_HIGH>;
	spl_gpio_output(gpio4, GPIO(BANK_D, 2), 1);

	// Vibrate ASAP
	// <&gpio3 RK_PB1 GPIO_ACTIVE_HIGH>;
	spl_gpio_output(gpio3, GPIO(BANK_B, 1), 1);
	mdelay(400); // 0.4s
	spl_gpio_output(gpio3, GPIO(BANK_B, 1), 0);
}

#endif
