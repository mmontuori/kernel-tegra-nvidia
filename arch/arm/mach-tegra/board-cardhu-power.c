/*
 * arch/arm/mach-tegra/board-cardhu-power.c
 *
 * Copyright (C) 2011 NVIDIA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/i2c.h>
#include <linux/pda_power.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/tps6591x.h>
#include <linux/gpio.h>
#include <mach/suspend.h>
#include <linux/io.h>
#include <linux/regulator/gpio-switch-regulator.h>
#include <linux/regulator/tps6591x-regulator.h>
#include <linux/regulator/tps6236x-regulator.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/pinmux.h>
#include <mach/edp.h>

#include "gpio-names.h"
#include "board.h"
#include "board-cardhu.h"
#include "power.h"
#include "wakeups-t3.h"

#define PMC_CTRL		0x0
#define PMC_CTRL_INTR_LOW	(1 << 17)

static struct regulator_consumer_supply tps6591x_vdd1_supply_skubit0_0[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
	REGULATOR_SUPPLY("en_vddio_ddr_1v2", NULL),
};

static struct regulator_consumer_supply tps6591x_vdd1_supply_skubit0_1[] = {
	REGULATOR_SUPPLY("en_vddio_ddr_1v2", NULL),
};

static struct regulator_consumer_supply tps6591x_vdd2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_gen1v5", NULL),
	REGULATOR_SUPPLY("vcore_lcd", NULL),
	REGULATOR_SUPPLY("track_ldo1", NULL),
	REGULATOR_SUPPLY("external_ldo_1v2", NULL),
	REGULATOR_SUPPLY("vcore_cam1", NULL),
	REGULATOR_SUPPLY("vcore_cam2", NULL),
};

static struct regulator_consumer_supply tps6591x_vddctrl_supply_0[] = {
	REGULATOR_SUPPLY("vdd_cpu_pmu", NULL),
	REGULATOR_SUPPLY("vdd_cpu", NULL),
	REGULATOR_SUPPLY("vdd_sys", NULL),
};

static struct regulator_consumer_supply tps6591x_vio_supply_0[] = {
	REGULATOR_SUPPLY("vdd_gen1v8", NULL),
	REGULATOR_SUPPLY("avdd_hdmi_pll", NULL),
	REGULATOR_SUPPLY("avdd_usb_pll", NULL),
	REGULATOR_SUPPLY("avdd_osc", NULL),
	REGULATOR_SUPPLY("vddio_sys", NULL),
	REGULATOR_SUPPLY("vddio_sdmmc4", NULL),
	REGULATOR_SUPPLY("vdd1v8_satelite", NULL),
	REGULATOR_SUPPLY("vddio_uart", NULL),
	REGULATOR_SUPPLY("vddio_audio", NULL),
	REGULATOR_SUPPLY("vddio_bb", NULL),
	REGULATOR_SUPPLY("vddio_lcd_pmu", NULL),
	REGULATOR_SUPPLY("vddio_cam", NULL),
	REGULATOR_SUPPLY("vddio_vi", NULL),
	REGULATOR_SUPPLY("ldo6", NULL),
	REGULATOR_SUPPLY("ldo7", NULL),
	REGULATOR_SUPPLY("ldo8", NULL),
	REGULATOR_SUPPLY("vcore_audio", NULL),
	REGULATOR_SUPPLY("avcore_audio", NULL),
	REGULATOR_SUPPLY("vddio_sdmmc3", NULL),
	REGULATOR_SUPPLY("vcore1_lpddr2", NULL),
	REGULATOR_SUPPLY("vcom_1v8", NULL),
	REGULATOR_SUPPLY("pmuio_1v8", NULL),
	REGULATOR_SUPPLY("avdd_ic_usb", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo1_supply_0[] = {
	REGULATOR_SUPPLY("avdd_pexb", NULL),
	REGULATOR_SUPPLY("vdd_pexb", NULL),
	REGULATOR_SUPPLY("avdd_pex_pll", NULL),
	REGULATOR_SUPPLY("avdd_pexa", NULL),
	REGULATOR_SUPPLY("vdd_pexa", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo2_supply_0[] = {
	REGULATOR_SUPPLY("avdd_sata", NULL),
	REGULATOR_SUPPLY("vdd_sata", NULL),
	REGULATOR_SUPPLY("avdd_sata_pll", NULL),
	REGULATOR_SUPPLY("avdd_plle", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo3_supply_e118x[] = {
	REGULATOR_SUPPLY("vddio_sdmmc1", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo3_supply_e1198[] = {
	REGULATOR_SUPPLY("unused_rail_ldo3", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_rtc", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo5_supply_e118x[] = {
	REGULATOR_SUPPLY("avdd_vdac", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo5_supply_e1198[] = {
	REGULATOR_SUPPLY("avdd_vdac", NULL),
	REGULATOR_SUPPLY("vddio_sdmmc1", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo6_supply_0[] = {
	REGULATOR_SUPPLY("avdd_dsi_csi", NULL),
};
static struct regulator_consumer_supply tps6591x_ldo7_supply_0[] = {
	REGULATOR_SUPPLY("avdd_plla_p_c_s", NULL),
	REGULATOR_SUPPLY("avdd_pllm", NULL),
	REGULATOR_SUPPLY("avdd_pllu_d", NULL),
	REGULATOR_SUPPLY("avdd_pllu_d2", NULL),
	REGULATOR_SUPPLY("avdd_pllx", NULL),
};

static struct regulator_consumer_supply tps6591x_ldo8_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr_hs", NULL),
};

#define TPS_PDATA_INIT(_name, _sname, _minmv, _maxmv, _supply_reg, _always_on, \
	_boot_on, _apply_uv, _init_uV, _init_enable, _init_apply, _ectrl) \
	static struct tps6591x_regulator_platform_data pdata_##_name##_##_sname = \
	{								\
		.regulator = {						\
			.constraints = {				\
				.min_uV = (_minmv)*1000,		\
				.max_uV = (_maxmv)*1000,		\
				.valid_modes_mask = (REGULATOR_MODE_NORMAL |  \
						     REGULATOR_MODE_STANDBY), \
				.valid_ops_mask = (REGULATOR_CHANGE_MODE |    \
						   REGULATOR_CHANGE_STATUS |  \
						   REGULATOR_CHANGE_VOLTAGE), \
				.always_on = _always_on,		\
				.boot_on = _boot_on,			\
				.apply_uV = _apply_uv,			\
			},						\
			.num_consumer_supplies =			\
				ARRAY_SIZE(tps6591x_##_name##_supply_##_sname),	\
			.consumer_supplies = tps6591x_##_name##_supply_##_sname,	\
			.supply_regulator = _supply_reg,		\
		},							\
		.init_uV =  _init_uV * 1000,				\
		.init_enable = _init_enable,				\
		.init_apply = _init_apply,				\
		.ectrl = _ectrl						\
	}

TPS_PDATA_INIT(vdd1, skubit0_0, 600,  1500, 0, 1, 1, 0, -1, 0, 0, EXT_CTRL_SLEEP_OFF);
TPS_PDATA_INIT(vdd1, skubit0_1, 600,  1500, 0, 1, 1, 0, -1, 0, 0, EXT_CTRL_SLEEP_OFF);
TPS_PDATA_INIT(vdd2, 0,         600,  1500, 0, 1, 1, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(vddctrl, 0,      600,  1400, 0, 1, 1, 0, -1, 0, 0, EXT_CTRL_EN1);
TPS_PDATA_INIT(vio,  0,         1500, 3300, 0, 1, 1, 0, -1, 0, 0, 0);

TPS_PDATA_INIT(ldo1, 0,         1000, 3300, tps6591x_rails(VDD_2), 0, 0, 0, -1, 0, 1, 0);
TPS_PDATA_INIT(ldo2, 0,         1000, 3300, tps6591x_rails(VDD_2), 0, 0, 0, -1, 0, 1, 0);

TPS_PDATA_INIT(ldo3, e118x,     1000, 3300, 0, 0, 0, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(ldo3, e1198,     1000, 3300, 0, 0, 0, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(ldo4, 0,         1000, 3300, 0, 1, 0, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(ldo5, e118x,     1000, 3300, 0, 0, 0, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(ldo5, e1198,     1000, 3300, 0, 0, 0, 0, -1, 0, 0, 0);

TPS_PDATA_INIT(ldo6, 0,         1000, 3300, tps6591x_rails(VIO), 0, 0, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(ldo7, 0,         1000, 3300, tps6591x_rails(VIO), 1, 0, 0, -1, 0, 0, 0);
TPS_PDATA_INIT(ldo8, 0,         1000, 3300, tps6591x_rails(VIO), 1, 0, 0, -1, 0, 0, 0);

#if defined(CONFIG_RTC_DRV_TPS6591x)
static struct tps6591x_rtc_platform_data rtc_data = {
	.irq = TEGRA_NR_IRQS + TPS6591X_INT_RTC_ALARM,
	.time = {
		.tm_year = 2000,
		.tm_mon = 0,
		.tm_mday = 1,
		.tm_hour = 0,
		.tm_min = 0,
		.tm_sec = 0,
	},
};

#define TPS_RTC_REG()					\
	{						\
		.id	= 0,				\
		.name	= "rtc_tps6591x",		\
		.platform_data = &rtc_data,		\
	}
#endif

#define TPS_REG(_id, _name, _sname)				\
	{							\
		.id	= TPS6591X_ID_##_id,			\
		.name	= "tps6591x-regulator",			\
		.platform_data	= &pdata_##_name##_##_sname,	\
	}

#define TPS6591X_DEV_COMMON_E118X 		\
	TPS_REG(VDD_2, vdd2, 0),		\
	TPS_REG(VDDCTRL, vddctrl, 0),		\
	TPS_REG(LDO_1, ldo1, 0),		\
	TPS_REG(LDO_2, ldo2, 0),		\
	TPS_REG(LDO_3, ldo3, e118x),		\
	TPS_REG(LDO_4, ldo4, 0),		\
	TPS_REG(LDO_5, ldo5, e118x),		\
	TPS_REG(LDO_6, ldo6, 0),		\
	TPS_REG(LDO_7, ldo7, 0),		\
	TPS_REG(LDO_8, ldo8, 0)

static struct tps6591x_subdev_info tps_devs_e118x_skubit0_0[] = {
	TPS_REG(VIO, vio, 0),
	TPS_REG(VDD_1, vdd1, skubit0_0),
	TPS6591X_DEV_COMMON_E118X,
#if defined(CONFIG_RTC_DRV_TPS6591x)
	TPS_RTC_REG(),
#endif
};

static struct tps6591x_subdev_info tps_devs_e118x_skubit0_1[] = {
	TPS_REG(VIO, vio, 0),
	TPS_REG(VDD_1, vdd1, skubit0_1),
	TPS6591X_DEV_COMMON_E118X,
#if defined(CONFIG_RTC_DRV_TPS6591x)
	TPS_RTC_REG(),
#endif
};

#define TPS6591X_DEV_COMMON_CARDHU		\
	TPS_REG(VDD_2, vdd2, 0),		\
	TPS_REG(VDDCTRL, vddctrl, 0),		\
	TPS_REG(LDO_1, ldo1, 0),		\
	TPS_REG(LDO_2, ldo2, 0),		\
	TPS_REG(LDO_3, ldo3, e1198),		\
	TPS_REG(LDO_4, ldo4, 0),		\
	TPS_REG(LDO_5, ldo5, e1198),		\
	TPS_REG(LDO_6, ldo6, 0),		\
	TPS_REG(LDO_7, ldo7, 0),		\
	TPS_REG(LDO_8, ldo8, 0)

static struct tps6591x_subdev_info tps_devs_e1198_skubit0_0[] = {
	TPS_REG(VIO, vio, 0),
	TPS_REG(VDD_1, vdd1, skubit0_0),
	TPS6591X_DEV_COMMON_CARDHU,
#if defined(CONFIG_RTC_DRV_TPS6591x)
	TPS_RTC_REG(),
#endif
};

static struct tps6591x_subdev_info tps_devs_e1198_skubit0_1[] = {
	TPS_REG(VIO, vio, 0),
	TPS_REG(VDD_1, vdd1, skubit0_1),
	TPS6591X_DEV_COMMON_CARDHU,
#if defined(CONFIG_RTC_DRV_TPS6591x)
	TPS_RTC_REG(),
#endif
};

#define TPS_GPIO_INIT_PDATA(gpio_nr, _init_apply, _sleep_en, _pulldn_en, _output_en, _output_val)	\
	[gpio_nr] = {					\
			.sleep_en	= _sleep_en,	\
			.pulldn_en	= _pulldn_en,	\
			.output_mode_en	= _output_en,	\
			.output_val	= _output_val,	\
			.init_apply	= _init_apply,	\
		     }
static struct tps6591x_gpio_init_data tps_gpio_pdata_e1291_a04[] =  {
	TPS_GPIO_INIT_PDATA(0, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(1, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(2, 1, 1, 0, 1, 1),
	TPS_GPIO_INIT_PDATA(3, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(4, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(5, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(6, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(7, 0, 0, 0, 0, 0),
	TPS_GPIO_INIT_PDATA(8, 0, 0, 0, 0, 0),
};

static struct tps6591x_sleep_keepon_data tps_slp_keepon = {
	.clkout32k_keepon = 1,
};

static struct tps6591x_platform_data tps_platform = {
	.irq_base	= TPS6591X_IRQ_BASE,
	.gpio_base	= TPS6591X_GPIO_BASE,
	.dev_slp_en	= true,
	.slp_keepon	= &tps_slp_keepon,
};

static struct i2c_board_info __initdata cardhu_regulators[] = {
	{
		I2C_BOARD_INFO("tps6591x", 0x2D),
		.irq		= INT_EXTERNAL_PMU,
		.platform_data	= &tps_platform,
	},
};

/* TPS62361B DC-DC converter */
static struct regulator_consumer_supply tps6236x_dcdc_supply[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
};

static struct tps6236x_regulator_platform_data tps6236x_pdata = {
	.reg_init_data = {					\
		.constraints = {				\
			.min_uV = 500000,			\
			.max_uV = 1770000,			\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |  \
					     REGULATOR_MODE_STANDBY), \
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |    \
					   REGULATOR_CHANGE_STATUS |  \
					   REGULATOR_CHANGE_VOLTAGE), \
			.always_on = 1,				\
			.boot_on =  1,				\
			.apply_uV = 0,				\
		},						\
		.num_consumer_supplies = ARRAY_SIZE(tps6236x_dcdc_supply), \
		.consumer_supplies = tps6236x_dcdc_supply,		\
		},							\
	.internal_pd_enable = 0,					\
	.vsel = 3,							\
	.init_uV = -1,							\
	.init_apply = 0,						\
};

static struct i2c_board_info __initdata tps6236x_boardinfo[] = {
	{
		I2C_BOARD_INFO("tps62361B", 0x60),
		.platform_data	= &tps6236x_pdata,
	},
};

int __init cardhu_regulator_init(void)
{
	struct board_info board_info;
	struct board_info pmu_board_info;
	void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	u32 pmc_ctrl;

	/* configure the power management controller to trigger PMU
	 * interrupts when low */

	pmc_ctrl = readl(pmc + PMC_CTRL);
	writel(pmc_ctrl | PMC_CTRL_INTR_LOW, pmc + PMC_CTRL);

	/* The regulator details have complete constraints */
	regulator_has_full_constraints();

	tegra_get_board_info(&board_info);
	tegra_get_pmu_board_info(&pmu_board_info);

	if ((board_info.board_id == BOARD_E1198) ||
		(board_info.board_id == BOARD_E1291)) {
		if ((board_info.sku & 1) == 1) {
			tps_platform.num_subdevs =
					ARRAY_SIZE(tps_devs_e1198_skubit0_1);
			tps_platform.subdevs = tps_devs_e1198_skubit0_1;
		} else {
			tps_platform.num_subdevs =
					ARRAY_SIZE(tps_devs_e1198_skubit0_0);
			tps_platform.subdevs = tps_devs_e1198_skubit0_0;
		}
	} else {
		if ((pmu_board_info.sku & 1) == 1) {
			tps_platform.num_subdevs = ARRAY_SIZE(tps_devs_e118x_skubit0_1);
			tps_platform.subdevs = tps_devs_e118x_skubit0_1;
		} else {
			tps_platform.num_subdevs = ARRAY_SIZE(tps_devs_e118x_skubit0_0);
			tps_platform.subdevs = tps_devs_e118x_skubit0_0;
		}
	}

	/* E1291-A04: Enable DEV_SLP and enable sleep on GPIO2 */
	if ((board_info.board_id == BOARD_E1291) && (board_info.fab == 0x4)) {
		tps_platform.dev_slp_en = true;
		tps_platform.gpio_init_data = tps_gpio_pdata_e1291_a04;
		tps_platform.num_gpioinit_data =
					ARRAY_SIZE(tps_gpio_pdata_e1291_a04);
	}

	i2c_register_board_info(4, cardhu_regulators, 1);

	/* Resgister the TPS6236x for all boards whose sku bit 0 is set. */
	if (((board_info.sku & 1) == 1) || ((pmu_board_info.sku & 1) == 1)) {
		pr_info("Registering the device TPS62361B\n");
		i2c_register_board_info(4, tps6236x_boardinfo, 1);
	}
	return 0;
}

/* EN_5V_CP from PMU GP0 */
static struct regulator_consumer_supply gpio_switch_en_5v_cp_supply[] = {
	REGULATOR_SUPPLY("vdd_5v0_sby", NULL),
	REGULATOR_SUPPLY("vdd_hall", NULL),
	REGULATOR_SUPPLY("vterm_ddr", NULL),
	REGULATOR_SUPPLY("v2ref_ddr", NULL),
};
static int gpio_switch_en_5v_cp_voltages[] = { 5000};

/* EN_5V0 From PMU GP2 */
static struct regulator_consumer_supply gpio_switch_en_5v0_supply[] = {
	REGULATOR_SUPPLY("vdd_5v0_sys", NULL),
};
static int gpio_switch_en_5v0_voltages[] = { 5000};

/* EN_DDR From PMU GP6 */
static struct regulator_consumer_supply gpio_switch_en_ddr_supply[] = {
	REGULATOR_SUPPLY("mem_vddio_ddr", NULL),
	REGULATOR_SUPPLY("t30_vddio_ddr", NULL),
};
static int gpio_switch_en_ddr_voltages[] = { 1500};

/* EN_3V3_SYS From PMU GP7 */
static struct regulator_consumer_supply gpio_switch_en_3v3_sys_supply[] = {
	REGULATOR_SUPPLY("vdd_lvds", NULL),
	REGULATOR_SUPPLY("vdd_pnl", NULL),
	REGULATOR_SUPPLY("vcom_3v3", NULL),
	REGULATOR_SUPPLY("vdd_3v3", NULL),
	REGULATOR_SUPPLY("vcore_mmc", NULL),
	REGULATOR_SUPPLY("vddio_pex_ctl", NULL),
	REGULATOR_SUPPLY("hvdd_pex", NULL),
	REGULATOR_SUPPLY("avdd_hdmi", NULL),
	REGULATOR_SUPPLY("vpp_fuse", NULL),
	REGULATOR_SUPPLY("avdd_usb", NULL),
	REGULATOR_SUPPLY("vdd_ddr_rx", NULL),
	REGULATOR_SUPPLY("vcore_nand", NULL),
	REGULATOR_SUPPLY("hvdd_sata", NULL),
	REGULATOR_SUPPLY("vddio_gmi_pmu", NULL),
	REGULATOR_SUPPLY("avdd_cam1", NULL),
	REGULATOR_SUPPLY("vdd_af", NULL),
	REGULATOR_SUPPLY("avdd_cam2", NULL),
	REGULATOR_SUPPLY("vdd_acc", NULL),
	REGULATOR_SUPPLY("vdd_phtl", NULL),
	REGULATOR_SUPPLY("vddio_tp", NULL),
	REGULATOR_SUPPLY("vdd_led", NULL),
	REGULATOR_SUPPLY("vddio_cec", NULL),
	REGULATOR_SUPPLY("vdd_cmps", NULL),
	REGULATOR_SUPPLY("vdd_temp", NULL),
	REGULATOR_SUPPLY("vpp_kfuse", NULL),
	REGULATOR_SUPPLY("vddio_ts", NULL),
	REGULATOR_SUPPLY("vdd_ir_led", NULL),
	REGULATOR_SUPPLY("vddio_1wire", NULL),
	REGULATOR_SUPPLY("avddio_audio", NULL),
	REGULATOR_SUPPLY("vdd_ec", NULL),
	REGULATOR_SUPPLY("vcom_pa", NULL),
	REGULATOR_SUPPLY("vdd_3v3_devices", NULL),
	REGULATOR_SUPPLY("vdd_3v3_dock", NULL),
	REGULATOR_SUPPLY("vdd_3v3_edid", NULL),
	REGULATOR_SUPPLY("vdd_3v3_hdmi_cec", NULL),
	REGULATOR_SUPPLY("vdd_3v3_gmi", NULL),
	REGULATOR_SUPPLY("vdd_3v3_spk_amp", NULL),
	REGULATOR_SUPPLY("vdd_3v3_sensor", NULL),
	REGULATOR_SUPPLY("vdd_3v3_cam", NULL),
	REGULATOR_SUPPLY("vdd_3v3_als", NULL),
	REGULATOR_SUPPLY("debug_cons", NULL),
};
static int gpio_switch_en_3v3_sys_voltages[] = { 3300};

/* DIS_5V_SWITCH from AP SPI2_SCK X02 */
static struct regulator_consumer_supply gpio_switch_dis_5v_switch_supply[] = {
	REGULATOR_SUPPLY("master_5v_switch", NULL),
};
static int gpio_switch_dis_5v_switch_voltages[] = { 5000};

/* EN_VDD_BL */
static struct regulator_consumer_supply gpio_switch_en_vdd_bl_supply[] = {
	REGULATOR_SUPPLY("vdd_backlight", NULL),
	REGULATOR_SUPPLY("vdd_backlight1", NULL),
};
static int gpio_switch_en_vdd_bl_voltages[] = { 5000};

/* EN_VDD_BL2 (E1291-A03) from AP PEX_L0_PRSNT_N DD.00 */
static struct regulator_consumer_supply gpio_switch_en_vdd_bl2_supply[] = {
	REGULATOR_SUPPLY("vdd_backlight2", NULL),
};
static int gpio_switch_en_vdd_bl2_voltages[] = { 5000};

/* EN_3V3_MODEM from AP GPIO VI_VSYNCH D06*/
static struct regulator_consumer_supply gpio_switch_en_3v3_modem_supply[] = {
	REGULATOR_SUPPLY("vdd_3v3_mini_card", NULL),
	REGULATOR_SUPPLY("vdd_mini_card", NULL),
};
static int gpio_switch_en_3v3_modem_voltages[] = { 3300};

/* EN_USB1_VBUS_OC*/
static struct regulator_consumer_supply gpio_switch_en_usb1_vbus_oc_supply[] = {
	REGULATOR_SUPPLY("vdd_vbus_micro_usb", NULL),
};
static int gpio_switch_en_usb1_vbus_oc_voltages[] = { 5000};

/*EN_USB3_VBUS_OC*/
static struct regulator_consumer_supply gpio_switch_en_usb3_vbus_oc_supply[] = {
	REGULATOR_SUPPLY("vdd_vbus_typea_usb", NULL),
};
static int gpio_switch_en_usb3_vbus_oc_voltages[] = { 5000};

/* EN_VDDIO_VID_OC from AP GPIO VI_PCLK T00*/
static struct regulator_consumer_supply gpio_switch_en_vddio_vid_oc_supply[] = {
	REGULATOR_SUPPLY("vdd_hdmi_con", NULL),
};
static int gpio_switch_en_vddio_vid_oc_voltages[] = { 5000};

/* EN_VDD_PNL1 from AP GPIO VI_D6 L04*/
static struct regulator_consumer_supply gpio_switch_en_vdd_pnl1_supply[] = {
	REGULATOR_SUPPLY("vdd_lcd_panel", NULL),
};
static int gpio_switch_en_vdd_pnl1_voltages[] = { 3300};

/* CAM1_LDO_EN from AP GPIO KB_ROW6 R06*/
static struct regulator_consumer_supply gpio_switch_cam1_ldo_en_supply[] = {
	REGULATOR_SUPPLY("vdd_2v8_cam1", NULL),
	REGULATOR_SUPPLY("vdd_2v8_cam1_af", NULL),
};
static int gpio_switch_cam1_ldo_en_voltages[] = { 2800};

/* CAM2_LDO_EN from AP GPIO KB_ROW7 R07*/
static struct regulator_consumer_supply gpio_switch_cam2_ldo_en_supply[] = {
	REGULATOR_SUPPLY("vdd_2v8_cam2", NULL),
	REGULATOR_SUPPLY("vdd_2v8_cam2_af", NULL),
};
static int gpio_switch_cam2_ldo_en_voltages[] = { 2800};

/* CAM3_LDO_EN from AP GPIO KB_ROW8 S00*/
static struct regulator_consumer_supply gpio_switch_cam3_ldo_en_supply[] = {
	REGULATOR_SUPPLY("vdd_cam3", NULL),
};
static int gpio_switch_cam3_ldo_en_voltages[] = { 3300};

/* EN_VDD_COM from AP GPIO SDMMC3_DAT5 D00*/
static struct regulator_consumer_supply gpio_switch_en_vdd_com_supply[] = {
	REGULATOR_SUPPLY("vdd_com_bd", NULL),
};
static int gpio_switch_en_vdd_com_voltages[] = { 3300};

/* EN_VDD_SDMMC1 from AP GPIO VI_HSYNC D07*/
static struct regulator_consumer_supply gpio_switch_en_vdd_sdmmc1_supply[] = {
	REGULATOR_SUPPLY("vddio_sd_slot", NULL),
};
static int gpio_switch_en_vdd_sdmmc1_voltages[] = { 3300};

/* EN_3V3_EMMC from AP GPIO SDMMC3_DAT4 D01*/
static struct regulator_consumer_supply gpio_switch_en_3v3_emmc_supply[] = {
	REGULATOR_SUPPLY("vdd_emmc_core", NULL),
};
static int gpio_switch_en_3v3_emmc_voltages[] = { 3300};

/* EN_3V3_PEX_HVDD from AP GPIO VI_D09 L07*/
static struct regulator_consumer_supply gpio_switch_en_3v3_pex_hvdd_supply[] = {
	REGULATOR_SUPPLY("hvdd_pex_3v3", NULL),
};
static int gpio_switch_en_3v3_pex_hvdd_voltages[] = { 3300};

/* EN_3v3_FUSE from AP GPIO VI_D08 L06*/
static struct regulator_consumer_supply gpio_switch_en_3v3_fuse_supply[] = {
	REGULATOR_SUPPLY("vdd_fuse", NULL),
};
static int gpio_switch_en_3v3_fuse_voltages[] = { 3300};

/* EN_1V8_CAM from AP GPIO GPIO_PBB4 PBB04*/
static struct regulator_consumer_supply gpio_switch_en_1v8_cam_supply[] = {
	REGULATOR_SUPPLY("vdd_1v8_cam1", NULL),
	REGULATOR_SUPPLY("vdd_1v8_cam2", NULL),
	REGULATOR_SUPPLY("vdd_1v8_cam3", NULL),
};
static int gpio_switch_en_1v8_cam_voltages[] = { 1800};

static struct regulator_consumer_supply gpio_switch_en_vbrtr_supply[] = {
	REGULATOR_SUPPLY("vdd_vbrtr", NULL),
};
static int gpio_switch_en_vbrtr_voltages[] = { 3300};

static int enable_load_switch_rail(
		struct gpio_switch_regulator_subdev_data *psubdev_data)
{
	int ret;

	if (psubdev_data->pin_group <= 0)
		return -EINVAL;

	/* Tristate and make pin as input*/
	ret = tegra_pinmux_set_tristate(psubdev_data->pin_group,
						TEGRA_TRI_TRISTATE);
	if (ret < 0)
		return ret;
	return gpio_direction_input(psubdev_data->gpio_nr);
}

static int disable_load_switch_rail(
		struct gpio_switch_regulator_subdev_data *psubdev_data)
{
	int ret;

	if (psubdev_data->pin_group <= 0)
		return -EINVAL;

	/* Un-tristate and driver low */
	ret = tegra_pinmux_set_tristate(psubdev_data->pin_group,
						TEGRA_TRI_NORMAL);
	if (ret < 0)
		return ret;
	return gpio_direction_output(psubdev_data->gpio_nr, 0);
}


/* Macro for defining gpio switch regulator sub device data */
#define GREG_INIT(_id, _var, _name, _input_supply, _always_on, _boot_on, \
	_gpio_nr, _active_low, _init_state, _pg, _enable, _disable)	 \
	static struct gpio_switch_regulator_subdev_data gpio_pdata_##_var =  \
	{								\
		.regulator_name	= "gpio-switch-"#_name,			\
		.input_supply	= _input_supply,			\
		.id		= _id,					\
		.gpio_nr	= _gpio_nr,				\
		.pin_group	= _pg,					\
		.active_low	= _active_low,				\
		.init_state	= _init_state,				\
		.voltages	= gpio_switch_##_name##_voltages,	\
		.n_voltages	= ARRAY_SIZE(gpio_switch_##_name##_voltages), \
		.num_consumer_supplies =				\
				ARRAY_SIZE(gpio_switch_##_name##_supply), \
		.consumer_supplies = gpio_switch_##_name##_supply,	\
		.constraints = {					\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |	\
					     REGULATOR_MODE_STANDBY),	\
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |	\
					   REGULATOR_CHANGE_STATUS |	\
					   REGULATOR_CHANGE_VOLTAGE),	\
			.always_on = _always_on,			\
			.boot_on = _boot_on,				\
		},							\
		.enable_rail = _enable,					\
		.disable_rail = _disable,				\
	}

/* common to most of boards*/
GREG_INIT(0, en_5v_cp,		en_5v_cp,	NULL,			1,	0,	TPS6591X_GPIO_GP0,	false,	1,	0,	0,	0);
GREG_INIT(1, en_5v0,		en_5v0,		NULL,			0,      0,      TPS6591X_GPIO_GP2,	false,	0,	0,	0,	0);
GREG_INIT(2, en_ddr,		en_ddr,		NULL,			0,      0,      TPS6591X_GPIO_GP6,	false,	0,	0,	0,	0);
GREG_INIT(3, en_3v3_sys,	en_3v3_sys,	NULL,			0,      0,      TPS6591X_GPIO_GP7,	false,	0,	0,	0,	0);
GREG_INIT(4, en_vdd_bl,		en_vdd_bl,	NULL,			1,      0,      TEGRA_GPIO_PK3,		false,	1,	0,	0,	0);
GREG_INIT(5, en_3v3_modem,	en_3v3_modem,	NULL,			1,      0,      TEGRA_GPIO_PD6,		false,	1,	0,	0,	0);
GREG_INIT(6, en_vdd_pnl1,	en_vdd_pnl1,	"vdd_3v3_devices",	1,      0,      TEGRA_GPIO_PL4,		false,	1,	0,	0,	0);
GREG_INIT(7, cam3_ldo_en,	cam3_ldo_en,	"vdd_3v3_devices",	0,      0,      TEGRA_GPIO_PS0,		false,	0,	0,	0,	0);
GREG_INIT(8, en_vdd_com,	en_vdd_com,	"vdd_3v3_devices",	1,      0,      TEGRA_GPIO_PD0,		false,	1,	0,	0,	0);
GREG_INIT(9, en_3v3_fuse,	en_3v3_fuse,	"vdd_3v3_devices",	0,      0,      TEGRA_GPIO_PL6,		false,	0,	0,	0,	0);
GREG_INIT(10, en_3v3_emmc,	en_3v3_emmc,	"vdd_3v3_devices",	1,      0,      TEGRA_GPIO_PD1,		false,	1,	0,	0,	0);
GREG_INIT(11, en_vdd_sdmmc1,	en_vdd_sdmmc1,	"vdd_3v3_devices",	1,      0,      TEGRA_GPIO_PD7,		false,	1,	0,	0,	0);
GREG_INIT(12, en_3v3_pex_hvdd,	en_3v3_pex_hvdd, "vdd_3v3_devices",	0,      0,      TEGRA_GPIO_PL7,		false,	0,	0,	0,	0);
GREG_INIT(13, en_1v8_cam,	en_1v8_cam,	"vdd_gen1v8",		0,      0,      TEGRA_GPIO_PBB4,	false,	0,	0,	0,	0);

/* E1291-A04 specific */
GREG_INIT(1, en_5v0_a04,	en_5v0,		NULL,			0,      0,      TPS6591X_GPIO_GP8,	false,	0,	0,	0,	0);
GREG_INIT(2, en_ddr_a04,	en_ddr,		NULL,			0,      0,      TPS6591X_GPIO_GP7,	false,	0,	0,	0,	0);
GREG_INIT(3, en_3v3_sys_a04,	en_3v3_sys,	NULL,			0,      0,      TPS6591X_GPIO_GP6,	false,	0,	0,	0,	0);


/*Specific to pm269*/
GREG_INIT(4, en_vdd_bl_pm269,		en_vdd_bl,		NULL,
	1,      0,      TEGRA_GPIO_PH3,	false,	1,	0,	0,	0);
GREG_INIT(6, en_vdd_pnl1_pm269,		en_vdd_pnl1,		"vdd_3v3_devices",
	1,      0,      TEGRA_GPIO_PW1,	false,	1,	0,	0,	0);
GREG_INIT(9, en_3v3_fuse_pm269,		en_3v3_fuse,		"vdd_3v3_devices",
	0,      0,      TEGRA_GPIO_PC1,	false,	0,	0,	0,	0);
GREG_INIT(11, en_vdd_sdmmc1_pm269,	en_vdd_sdmmc1,		"vdd_3v3_devices",
	1,      0,      TEGRA_GPIO_PP1,	false,	1,	0,	0,	0);
GREG_INIT(12, en_3v3_pex_hvdd_pm269,	en_3v3_pex_hvdd,	"vdd_3v3_devices",
	0,      0,      TEGRA_GPIO_PC6,	false,	0,	0,	0,	0);
GREG_INIT(17, en_vddio_vid_oc_pm269,	en_vddio_vid_oc,	"master_5v_switch",
	0,      0,      TEGRA_GPIO_PP2,	false,	0,	TEGRA_PINGROUP_DAP3_DOUT,
	enable_load_switch_rail, disable_load_switch_rail);

/* Specific to E1187/E1186 */
GREG_INIT(14, dis_5v_switch_e118x,	dis_5v_switch,		"vdd_5v0_sys",
		0,      0,      TEGRA_GPIO_PX2,		true,	0,	0,	0,	0);
GREG_INIT(15, en_usb1_vbus_oc_e118x,	en_usb1_vbus_oc,	"master_5v_switch",
		0,      0,      TEGRA_GPIO_PI4,		false,	0,	TEGRA_PINGROUP_GMI_RST_N,
		enable_load_switch_rail, disable_load_switch_rail);
GREG_INIT(16, en_usb3_vbus_oc_e118x,	en_usb3_vbus_oc,	"master_5v_switch",
		0,      0,      TEGRA_GPIO_PH7,		false,	0,	TEGRA_PINGROUP_GMI_AD15,
		enable_load_switch_rail, disable_load_switch_rail);
GREG_INIT(17, en_vddio_vid_oc_e118x,	en_vddio_vid_oc,	"master_5v_switch",
		0,      0,      TEGRA_GPIO_PT0,		false,	0,	TEGRA_PINGROUP_VI_PCLK,
		enable_load_switch_rail, disable_load_switch_rail);

/* E1198/E1291 specific  fab < A03 */
GREG_INIT(15, en_usb1_vbus_oc,		en_usb1_vbus_oc,	"vdd_5v0_sys",
		0,      0,      TEGRA_GPIO_PI4,		false,	0,	TEGRA_PINGROUP_GMI_RST_N,
		enable_load_switch_rail, disable_load_switch_rail);
GREG_INIT(16, en_usb3_vbus_oc,		en_usb3_vbus_oc,	"vdd_5v0_sys",
		0,      0,      TEGRA_GPIO_PH7,		false,	0,	TEGRA_PINGROUP_GMI_AD15,
		enable_load_switch_rail, disable_load_switch_rail);

/* E1198/E1291 specific  fab >= A03 */
GREG_INIT(15, en_usb1_vbus_oc_a03,	en_usb1_vbus_oc,	"vdd_5v0_sys",
		0,      0,      TEGRA_GPIO_PDD6,		false,	0,	TEGRA_PINGROUP_PEX_L1_CLKREQ_N,
		enable_load_switch_rail, disable_load_switch_rail);
GREG_INIT(16, en_usb3_vbus_oc_a03,		en_usb3_vbus_oc,	"vdd_5v0_sys",
		0,      0,      TEGRA_GPIO_PDD4,		false,	0,	TEGRA_PINGROUP_PEX_L1_PRSNT_N,
		enable_load_switch_rail, disable_load_switch_rail);

/* E1198/E1291 specific */
GREG_INIT(17, en_vddio_vid_oc,		en_vddio_vid_oc,	"vdd_5v0_sys",
		0,      0,      TEGRA_GPIO_PT0,		false,	0,	TEGRA_PINGROUP_VI_PCLK,
		enable_load_switch_rail, disable_load_switch_rail);

/* E1198/E1291 specific*/
GREG_INIT(18, cam1_ldo_en,	cam1_ldo_en,	"vdd_3v3_cam",	0,      0,      TEGRA_GPIO_PR6,		false,	0,	0,	0,	0);
GREG_INIT(19, cam2_ldo_en,	cam2_ldo_en,	"vdd_3v3_cam",	0,      0,      TEGRA_GPIO_PR7,		false,	0,	0,	0,	0);

/* E1291 A03 specific */
GREG_INIT(20, en_vdd_bl1_a03,	en_vdd_bl,	NULL,		1,      0,      TEGRA_GPIO_PDD2,	false,	1,	0,	0,	0);
GREG_INIT(21, en_vdd_bl2_a03,	en_vdd_bl2,	NULL,		1,      0,      TEGRA_GPIO_PDD0,	false,	1,	0,	0,	0);

GREG_INIT(22, en_vbrtr,		en_vbrtr,	"vdd_3v3_devices",	0,      0,      PMU_TCA6416_GPIO_PORT12,	false,	0,	0,	0,	0);

#define ADD_GPIO_REG(_name) &gpio_pdata_##_name

#define COMMON_GPIO_REG \
	ADD_GPIO_REG(en_5v_cp),			\
	ADD_GPIO_REG(en_5v0),			\
	ADD_GPIO_REG(en_ddr),			\
	ADD_GPIO_REG(en_3v3_sys),		\
	ADD_GPIO_REG(en_3v3_modem),		\
	ADD_GPIO_REG(en_vdd_pnl1),		\
	ADD_GPIO_REG(cam3_ldo_en),		\
	ADD_GPIO_REG(en_vdd_com),		\
	ADD_GPIO_REG(en_3v3_fuse),		\
	ADD_GPIO_REG(en_3v3_emmc),		\
	ADD_GPIO_REG(en_vdd_sdmmc1),		\
	ADD_GPIO_REG(en_3v3_pex_hvdd),		\
	ADD_GPIO_REG(en_1v8_cam),

#define COMMON_GPIO_REG_E1291_A04 \
	ADD_GPIO_REG(en_5v_cp),			\
	ADD_GPIO_REG(en_5v0_a04),		\
	ADD_GPIO_REG(en_ddr_a04),		\
	ADD_GPIO_REG(en_3v3_sys_a04),		\
	ADD_GPIO_REG(en_3v3_modem),		\
	ADD_GPIO_REG(en_vdd_pnl1),		\
	ADD_GPIO_REG(cam3_ldo_en),		\
	ADD_GPIO_REG(en_vdd_com),		\
	ADD_GPIO_REG(en_3v3_fuse),		\
	ADD_GPIO_REG(en_3v3_emmc),		\
	ADD_GPIO_REG(en_vdd_sdmmc1),		\
	ADD_GPIO_REG(en_3v3_pex_hvdd),		\
	ADD_GPIO_REG(en_1v8_cam),

#define PM269_GPIO_REG \
	ADD_GPIO_REG(en_5v_cp),			\
	ADD_GPIO_REG(en_5v0),			\
	ADD_GPIO_REG(en_ddr),			\
	ADD_GPIO_REG(en_vdd_bl_pm269),		\
	ADD_GPIO_REG(en_3v3_sys),		\
	ADD_GPIO_REG(en_3v3_modem),		\
	ADD_GPIO_REG(en_vdd_pnl1_pm269),		\
	ADD_GPIO_REG(cam3_ldo_en),		\
	ADD_GPIO_REG(en_vdd_com),		\
	ADD_GPIO_REG(en_3v3_fuse_pm269),	\
	ADD_GPIO_REG(en_3v3_emmc),		\
	ADD_GPIO_REG(en_vdd_sdmmc1_pm269),	\
	ADD_GPIO_REG(en_3v3_pex_hvdd_pm269),	\
	ADD_GPIO_REG(en_1v8_cam),		\
	ADD_GPIO_REG(dis_5v_switch_e118x),	\
	ADD_GPIO_REG(en_usb1_vbus_oc_e118x),	\
	ADD_GPIO_REG(en_usb3_vbus_oc_e118x),	\
	ADD_GPIO_REG(en_vddio_vid_oc_pm269),

#define E118x_GPIO_REG	\
	ADD_GPIO_REG(en_vdd_bl),		\
	ADD_GPIO_REG(dis_5v_switch_e118x),	\
	ADD_GPIO_REG(en_usb1_vbus_oc_e118x),	\
	ADD_GPIO_REG(en_usb3_vbus_oc_e118x),	\
	ADD_GPIO_REG(en_vddio_vid_oc_e118x), \
	ADD_GPIO_REG(en_vbrtr),

#define E1198_GPIO_REG	\
	ADD_GPIO_REG(en_vddio_vid_oc),		\
	ADD_GPIO_REG(cam1_ldo_en),		\
	ADD_GPIO_REG(cam2_ldo_en),

#define E1291_1198_A00_GPIO_REG	\
	ADD_GPIO_REG(en_usb1_vbus_oc),		\
	ADD_GPIO_REG(en_usb3_vbus_oc),		\
	ADD_GPIO_REG(en_vdd_bl),

#define E1291_A03_GPIO_REG	\
	ADD_GPIO_REG(en_usb1_vbus_oc_a03),		\
	ADD_GPIO_REG(en_usb3_vbus_oc_a03),		\
	ADD_GPIO_REG(en_vdd_bl1_a03), \
	ADD_GPIO_REG(en_vdd_bl2_a03),

/* Gpio switch regulator platform data  for E1186/E1187*/

/* Gpio switch regulator platform data  for E1186/E1187*/
static struct gpio_switch_regulator_subdev_data *gswitch_subdevs_e118x[] = {
	COMMON_GPIO_REG
	E118x_GPIO_REG
};

/* Gpio switch regulator platform data for E1198 and E1291*/
static struct gpio_switch_regulator_subdev_data *gswitch_subdevs_e1198[] = {
	COMMON_GPIO_REG
	E1291_1198_A00_GPIO_REG
	E1198_GPIO_REG
};

/* Gpio switch regulator platform data for PM269*/
static struct gpio_switch_regulator_subdev_data *gswitch_subdevs_pm269[] = {
	PM269_GPIO_REG
};

/* Gpio switch regulator platform data for E1291 A03*/
static struct gpio_switch_regulator_subdev_data *gswitch_subdevs_e1291_a03[] = {
	COMMON_GPIO_REG
	E1291_A03_GPIO_REG
	E1198_GPIO_REG
};

/* Gpio switch regulator platform data for E1291 A04*/
static struct gpio_switch_regulator_subdev_data *gswitch_subdevs_e1291_a04[] = {
	COMMON_GPIO_REG_E1291_A04
	E1291_A03_GPIO_REG
	E1198_GPIO_REG
};


static struct gpio_switch_regulator_platform_data  gswitch_pdata;
static struct platform_device gswitch_regulator_pdata = {
	.name = "gpio-switch-regulator",
	.id   = -1,
	.dev  = {
	     .platform_data = &gswitch_pdata,
	},
};

int __init cardhu_gpio_switch_regulator_init(void)
{
	int i;
	struct board_info board_info;
	tegra_get_board_info(&board_info);
	switch (board_info.board_id) {
	case BOARD_E1198:
		gswitch_pdata.num_subdevs = ARRAY_SIZE(gswitch_subdevs_e1198);
		gswitch_pdata.subdevs = gswitch_subdevs_e1198;
		break;
	case BOARD_E1291:
		if (board_info.fab == 0x3) {
			gswitch_pdata.num_subdevs =
					ARRAY_SIZE(gswitch_subdevs_e1291_a03);
			gswitch_pdata.subdevs = gswitch_subdevs_e1291_a03;
		} else if (board_info.fab == 0x4) {
			gswitch_pdata.num_subdevs =
					ARRAY_SIZE(gswitch_subdevs_e1291_a04);
			gswitch_pdata.subdevs = gswitch_subdevs_e1291_a04;
		} else {
			gswitch_pdata.num_subdevs =
					ARRAY_SIZE(gswitch_subdevs_e1198);
			gswitch_pdata.subdevs = gswitch_subdevs_e1198;
		}
		break;
	case BOARD_PM269:
		gswitch_pdata.num_subdevs = ARRAY_SIZE(gswitch_subdevs_pm269);
		gswitch_pdata.subdevs = gswitch_subdevs_pm269;
		break;
	default:
		gswitch_pdata.num_subdevs = ARRAY_SIZE(gswitch_subdevs_e118x);
		gswitch_pdata.subdevs = gswitch_subdevs_e118x;
		break;
	}

	for (i = 0; i < gswitch_pdata.num_subdevs; ++i) {
		struct gpio_switch_regulator_subdev_data *gswitch_data = gswitch_pdata.subdevs[i];
		if (gswitch_data->gpio_nr <= TEGRA_NR_GPIOS)
			tegra_gpio_enable(gswitch_data->gpio_nr);
	}

	return platform_device_register(&gswitch_regulator_pdata);
}

static struct tegra_suspend_platform_data cardhu_suspend_data = {
	.cpu_timer	= 2000,
	.cpu_off_timer	= 200,
	.suspend_mode	= TEGRA_SUSPEND_LP0,
	.core_timer	= 0x7e7e,
	.core_off_timer = 0,
	.separate_req	= true,
	.corereq_high	= true,
	.sysclkreq_high	= true,
	.wake_enb	= TEGRA_WAKE_KBC_EVENT | TEGRA_WAKE_GPIO_PV0 |
			  TEGRA_WAKE_PWR_INT | TEGRA_WAKE_RTC_ALARM,
	.wake_high	= TEGRA_WAKE_RTC_ALARM,
	.wake_low	= TEGRA_WAKE_GPIO_PV0 | TEGRA_WAKE_PWR_INT,
	.wake_any	= TEGRA_WAKE_KBC_EVENT,
	.cpu_lp2_min_residency = 2000,
};

int __init cardhu_suspend_init(void)
{
	struct board_info board_info;
	struct board_info pmu_board_info;

	tegra_get_board_info(&board_info);
	tegra_get_pmu_board_info(&pmu_board_info);

	/* For PMU Fab A03 and A04 make core_pwr_req to high */
	if ((pmu_board_info.fab == 0x3) || (pmu_board_info.fab == 0x4))
		cardhu_suspend_data.corereq_high = true;

	/* CORE_PWR_REQ to be high for all processor/pmu board whose sku bit 0
	 * is set. This is require to enable the dc-dc converter tps62361x */
	if (((board_info.sku & 1) == 1) || ((pmu_board_info.sku & 1) == 1))
		cardhu_suspend_data.corereq_high = true;

	switch (board_info.board_id) {
	case BOARD_E1291:
		/* CORE_PWR_REQ to be high for E1291-A03 */
		if (board_info.fab == 0x3)
			cardhu_suspend_data.corereq_high = true;
		break;
	case BOARD_E1198:
	case BOARD_PM269:
		break;
	case BOARD_E1187:
	case BOARD_E1186:
		cardhu_suspend_data.cpu_timer = 5000;
		cardhu_suspend_data.cpu_off_timer = 5000;
		break;
	default:
		break;
	}

	tegra_init_suspend(&cardhu_suspend_data);
	return 0;
}

static void cardhu_power_off(void)
{
	int ret;
	pr_err("cardhu: Powering off the device\n");
	ret = tps6591x_power_off();
	if (ret)
		pr_err("cardhu: failed to power off\n");

	while (1);
}

int __init cardhu_power_off_init(void)
{
	pm_power_off = cardhu_power_off;
	return 0;
}

#ifdef CONFIG_TEGRA_EDP_LIMITS
/*
 * placeholder for now. needs to be changed with characterized data.
 * step size cannot be less than 4C
 */
static struct tegra_edp_limits cardhu_edp_limits[] = {
/* Temperature	 1 CPU	  2 CPUs   3 CPUs   4 CPUs */
	{60,	{1400000, 1300000, 1300000, 1300000} },
	{70,	{1400000, 1300000, 1300000, 1260000} },
	{80,	{1400000, 1300000, 1300000, 1200000} },
	{90,	{1400000, 1300000, 1300000, 1100000} },
};

void cardhu_thermal_zones_info(struct tegra_edp_limits **z, int *sz)
{
	*z = cardhu_edp_limits;
	*sz = ARRAY_SIZE(cardhu_edp_limits);
}

int __init cardhu_edp_init(void)
{
	tegra_init_cpu_edp_limits(cardhu_edp_limits, ARRAY_SIZE(cardhu_edp_limits));
	return 0;
}
#endif