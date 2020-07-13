/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include "disp_dts_gpio.h"
#endif

#ifndef MACH_FPGA
#include <lcm_pmic.h>
#endif

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

#define LCM_ID (0x98)

static const unsigned int BL_MIN_LEVEL = 20;
static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))
#define MDELAY(n)		(lcm_util.mdelay(n))
#define UDELAY(n)		(lcm_util.udelay(n))


#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	  lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

/* static unsigned char lcd_id_pins_value = 0xFF; */
static const unsigned char LCD_MODULE_ID = 0x01;
#define LCM_DSI_CMD_MODE	0
#define FRAME_WIDTH			(1200)
#define FRAME_HEIGHT		(1920)

#define LCM_PHYSICAL_WIDTH	(107640)
#define LCM_PHYSICAL_HEIGHT	(172224)


#define REGFLAG_DELAY		0xFFFC
#define REGFLAG_UDELAY	0xFFFB
#define REGFLAG_END_OF_TABLE	0xFFFD
#define REGFLAG_RESET_LOW	0xFFFE
#define REGFLAG_RESET_HIGH	0xFFFF


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* GPIO45       LCM_RST */
#ifdef GPIO_LCM_RST
#define GPIO_LCD_RST		GPIO_LCM_RST
#else
#define GPIO_LCD_RST		(GPIO45 | 0x80000000)
#endif

/* GPIO154       LED_EN */
#ifdef GPIO_LCM_LED_EN
#define GPIO_LCD_LED_EN		GPIO_LCM_LED_EN
#else
#define GPIO_LCD_LED_EN		(GPIO154 | 0x80000000)
#endif

/* GPIO167       ENP */
#ifdef GPIO_LCD_ENP
#define GPIO_LCD_PWR_ENP	GPIO_LCD_ENP
#else
#define GPIO_LCD_PWR_ENP	(GPIO167 | 0x80000000)
#endif

/* GPIO168       ENN */
#ifdef GPIO_LCD_ENN
#define GPIO_LCD_PWR_ENN	GPIO_LCD_ENN
#else
#define GPIO_LCD_PWR_ENN	(GPIO168 | 0x80000000)
#endif

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
	mt_set_gpio_mode(GPIO, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO, (output>0)? GPIO_OUT_ONE: GPIO_OUT_ZERO);
}

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} }
};

static struct LCM_setting_table init_setting_vdo[] = {
	{0x8F,0x01,{0xA5}},
	{REGFLAG_DELAY,20,{}},
	{0x01,0x01,{0x00}},
	{REGFLAG_DELAY,20,{}},
	{0x8F,0x01,{0xA5}},
	{REGFLAG_DELAY,10,{}},
	{0x83,0x01,{0x00}},
	{0x84,0x01,{0x00}},
	{0x8C,0x01,{0x8E}},
	{0xCD,0x01,{0x6C}},
	{0xC0,0x01,{0x8B}},
	{0xC8,0x01,{0xF0}},
	{0x8B,0x01,{0x10}},
	{0x9F,0x01,{0x80}},
	{0xA9,0x01,{0x00}},
	{0x97,0x01,{0x00}},
	{0xFA,0x01,{0x0D}},
	{0xFD,0x01,{0x13}},
	{0x83,0x01,{0xAA}},
	{0x84,0x01,{0x11}},
	{0x9C,0x01,{0x10}},
	{0xA9,0x01,{0x4B}},
	//{0x83,0x01,{0x00}},
	//{0x84,0x01,{0x00}},
	//{0x91,0x01,{0x06}},	//Bist mode
	{0x11,0x01,{0x00}},
	{REGFLAG_DELAY,120,{}},
	{0x8F,0x01,{0x00}},
	{REGFLAG_DELAY,10,{}},
	//{0x29,0x01,{0x00}},
	//{REGFLAG_DELAY,20,{}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(void *cmdq, struct LCM_setting_table *table,
	unsigned int count, unsigned char force_update)
{
	unsigned int i;
	unsigned cmd;

	for (i = 0; i < count; i++) {
		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;

		case REGFLAG_UDELAY:
			UDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V22(cmdq, cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	//params->physical_width_um = LCM_PHYSICAL_WIDTH;
	//params->physical_height_um = LCM_PHYSICAL_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
	//lcm_dsi_mode = CMD_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	//params->dsi.switch_mode = CMD_MODE;
	//lcm_dsi_mode = SYNC_PULSE_VDO_MODE;
	//params->dsi.mode   = SYNC_PULSE_VDO_MODE;	//SYNC_EVENT_VDO_MODE
#endif
	//LCM_LOGI("lcm_get_params lcm_dsi_mode %d\n", lcm_dsi_mode);
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 8;
	params->dsi.vertical_backporch = 16;
	params->dsi.vertical_frontporch = 30;
	//params->dsi.vertical_frontporch_for_low_power = 540;/*disable dynamic frame rate*/
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch = 68;
	params->dsi.horizontal_frontporch = 68;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.cont_clock = 1;

	params->dsi.ssc_range = 4;
	params->dsi.ssc_disable = 1;
	/*params->dsi.ssc_disable = 1;*/
#ifndef CONFIG_FPGA_EARLY_PORTING
#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 270;	/* this value must be in MTK suggested table */
#else
	params->dsi.PLL_CLOCK = 480;	/* this value must be in MTK suggested table */
#endif
	//params->dsi.PLL_CK_CMD = 220;
	//params->dsi.PLL_CK_VDO = 255;
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 0;
	params->corner_pattern_width = 720;
	params->corner_pattern_height = 32;
#endif

	//params->use_gpioID = 1;
	//params->gpioID_value = 0;
}

#ifdef BUILD_LK
#ifndef MACH_FPGA
#define SM5109_SLAVE_ADDR_WRITE  0x7C
#define I2C_SM5109_PMU_CHANNEL 5
#define I2C_SM5109_PMU_SLAVE_7_BIT_ADDR 0x3E

static int SM5109_read_byte (kal_uint8 addr, kal_uint8 *dataBuffer)
{
	kal_uint32 ret = I2C_OK;
	kal_uint16 len;
	struct mt_i2c_t SM5109_i2c;
	*dataBuffer = addr;

	SM5109_i2c.id = I2C_SM5109_PMU_CHANNEL;
	SM5109_i2c.addr = I2C_SM5109_PMU_SLAVE_7_BIT_ADDR;
	SM5109_i2c.mode = ST_MODE;
	SM5109_i2c.speed = 100;
	len = 1;

	ret = i2c_write_read(&SM5109_i2c, dataBuffer, len, len);
	if (I2C_OK != ret)
	    LCM_LOGI("%s: i2c_read  failed! ret: %d\n", __func__, ret);

	return ret;
}

static int SM5109_write_byte(kal_uint8 addr, kal_uint8 value)
{
	kal_uint32 ret_code = I2C_OK;
	kal_uint8 write_data[2];
	kal_uint16 len;
	struct mt_i2c_t SM5109_i2c;

	write_data[0] = addr;
	write_data[1] = value;

	SM5109_i2c.id = I2C_SM5109_PMU_CHANNEL;
	SM5109_i2c.addr = I2C_SM5109_PMU_SLAVE_7_BIT_ADDR;
	SM5109_i2c.mode = ST_MODE;
	SM5109_i2c.speed = 100;
	len = 2;

	ret_code = i2c_write(&SM5109_i2c, write_data, len);

	return ret_code;
}

static int SM5109_REG_MASK (kal_uint8 addr, kal_uint8 val, kal_uint8 mask)
{
	kal_uint8 SM5109_reg = 0;
	kal_uint32 ret = 0;

	ret = SM5109_read_byte(addr, &SM5109_reg);

	SM5109_reg &= ~mask;
	SM5109_reg |= val;

	ret = SM5109_write_byte(addr, SM5109_reg);

	return ret;
}
#endif
#endif


static void lcm_init_power(void)
{
	LCM_LOGD("lcm_init_power\n");

	lcm_set_gpio_output(GPIO_LCD_PWR_ENP, GPIO_OUT_ONE);
	MDELAY(1);
	lcm_set_gpio_output(GPIO_LCD_PWR_ENN, GPIO_OUT_ONE);
	MDELAY(5);

	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ONE);
	MDELAY(10);
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ZERO);
	MDELAY(1);
	lcm_set_gpio_output(GPIO_LCD_RST, GPIO_OUT_ONE);
	MDELAY(5);

	lcm_set_gpio_output(GPIO_LCD_LED_EN, GPIO_OUT_ONE);
	MDELAY(5);
}

static void lcm_init(void)
{
	LCM_LOGD("lcm_init\n");

	push_table(NULL, init_setting_vdo, sizeof(init_setting_vdo) / sizeof(struct LCM_setting_table), 1);
	LCM_LOGI("nt51021h----SM5109----lcm mode = vdo mode ----\n");
}

static unsigned int lcm_compare_id(void)
{
	unsigned int ret = 0, ret1 = 0, ret2 = 0;

	MDELAY(2500);
	ret = mt_get_gpio_in(GPIO170);
	ret1 = mt_get_gpio_in(GPIO171);
#if defined(BUILD_LK)
	printf("%s, nt51021h GPIO164 = %d , GPIO165 = %d\n", __func__, ret, ret1);
#endif
	LCM_LOGI("%s, nt51021h GPIO164 = %d  GPIO165 = %d\n", __func__, ret, ret1);

	if ((ret == 0 && ret1 == 0)) {
		// truly + boe
		ret2 = 1;
	} else if((ret == 0 && ret1 == 1)) {
		ret2 = 0;
	} else if((ret == 1 && ret1 == 1)) {
		ret2 = 0;
	}
	LCM_LOGI("%s, nt51021h ret2 = %d\n",__func__, ret2);

	return (ret2 == 1) ? 1 : 0;
}

#if (LCM_DSI_CMD_MODE)

/* partial update restrictions:
 * 1. roi width must be 1080 (full lcm width)
 * 2. vertical start (y) must be multiple of 16
 * 3. vertical height (h) must be multiple of 16
 */
static void lcm_validate_roi(int *x, int *y, int *width, int *height)
{
	unsigned int y1 = *y;
	unsigned int y2 = *height + y1 - 1;
	unsigned int x1, w, h;

	x1 = 0;
	w = FRAME_WIDTH;

	y1 = round_down(y1, 16);
	h = y2 - y1 + 1;

	/* in some cases, roi maybe empty. In this case we need to use minimu roi */
	if (h < 16)
		h = 16;

	h = round_up(h, 16);

	/* check height again */
	if (y1 >= FRAME_HEIGHT || y1 + h > FRAME_HEIGHT) {
		/* assign full screen roi */
		LCM_LOGD("%s calc error,assign full roi:y=%d,h=%d\n", __func__, *y, *height);
		y1 = 0;
		h = FRAME_HEIGHT;
	}

	/*LCM_LOGD("lcm_validate_roi (%d,%d,%d,%d) to (%d,%d,%d,%d)\n",*/
	/*	*x, *y, *width, *height, x1, y1, w, h);*/

	*x = x1;
	*width = w;
	*y = y1;
	*height = h;
}
#endif

#if (LCM_DSI_CMD_MODE)
LCM_DRIVER nt51021h_wuxga_cmd_oncell_boe_lcm_drv = {
	.name = "nt51021h_wuxga_cmd_oncell_boe",
#else
LCM_DRIVER nt51021h_wuxga_vdo_oncell_boe_lcm_drv = {
	.name = "nt51021h_wuxga_vdo_oncell_boe",
#endif
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
#if (LCM_DSI_CMD_MODE)
	.validate_roi = lcm_validate_roi,
#endif
};

