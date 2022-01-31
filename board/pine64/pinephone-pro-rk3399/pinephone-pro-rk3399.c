// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * (C) Copyright 2022 Peter Robinson <pbrobinson at gmail.com>
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
#include <power/regulator.h>
#include <linux/delay.h>
#include <power/rk8xx_pmic.h>

#define GRF_IO_VSEL_BT565_GPIO2AB 1
#define GRF_IO_VSEL_AUDIO_GPIO3D4A 2
#define PMUGRF_CON0_VSEL_SHIFT 8

#ifndef CONFIG_SPL_BUILD
int board_early_init_f(void)
{
	struct udevice *regulator;
	int ret;

	ret = regulator_get_by_platname("vcc5v0_usb", &regulator);
	if (ret) {
		pr_debug("%s vcc5v0_usb init fail! ret %d\n", __func__, ret);
		goto out;
	}

	ret = regulator_set_enable(regulator, true);
	if (ret)
		pr_debug("%s vcc5v0-host-en-gpio set fail! ret %d\n", __func__, ret);

out:
	return 0;
}
#endif
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
	rk_setreg(&grf->io_vsel,
		  GRF_IO_VSEL_BT565_GPIO2AB | GRF_IO_VSEL_AUDIO_GPIO3D4A);
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

	return ret;
}

void setup_gpio_pins(void)
{
	struct rockchip_gpio_regs * const gpio4 = (void *)GPIO4_BASE;

	/* BROM leaves GPIO4_PD3 enabled as output for some unknown reason,
	 * and this breaks kernel expectations. (STK3311 probe on PPP).
	 * Set GPIO4_PD3 to input direction. */
	spl_gpio_input(gpio4, GPIO(BANK_D, 3));
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

static void pinephone_pro_init(void)
{
	struct udevice *pmic;
	int ret;

	printf("Initializing Pinephone Pro charger\n");

	ret = uclass_first_device_err(UCLASS_PMIC, &pmic);
	if (ret) {
		printf("ERROR: PMIC not found! (%d)\n", ret);
		return;
	}

	/*
	 * Raise input current limit to 1.5A, which is a standard CDC charger
	 * allowance.
	 */
	ret = rk818_spl_configure_usb_input_current(pmic, 1500);
	if (ret)
		printf("ERROR: Input current limit setup failed!\n");

	/*
	 * Close charger when USB lower then 3.26V
	 */
	rk818_spl_configure_usb_chrg_shutdown(pmic, 3260000);

	/*
	 * Check battery voltage and wait until it's in bootable state.
	 */
	//TODO:
	rk818_wait_battery(pmic);
}

void pmic_setup(void)
{
	if (of_machine_is_compatible("pine64,pinephone-pro"))
		pinephone_pro_init();
}
