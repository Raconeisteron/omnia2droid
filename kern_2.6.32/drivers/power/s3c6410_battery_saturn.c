/*
 * linux/drivers/power/s3c6410_battery.c
 *
 * Battery measurement code for S3C6410 platform.
 *
 * based on palmtx_battery.c
 *
 * Copyright (C) 2009 Samsung Electronics.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/irq.h>
#include <linux/wakelock.h>
#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <plat/gpio-cfg.h>

#include "s3c6410_battery_saturn.h"

#define S3C_CALL_STATE

static unsigned char fg_tab[] = { 0, 0, 0, 1, 3, 5, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 
	24, 26, 27, 29, 30, 32, 33, 35, 36, 38, 39, 40, 42, 43, 44, 46, 47, 48, 49, 51, 52, 
	53, 54, 55, 56, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 73, 
	74, 75, 76, 77, 78, 79, 79, 80, 81, 82, 83, 83, 84, 85, 85, 86, 87, 88, 88, 89, 90, 
	90, 91, 91, 92, 93, 93, 94, 95, 95, 96, 96, 97, 97, 98, 98, 99, 99, 100, 100, 100, 
	100, 100  
};

static struct wake_lock vbus_wake_lock;
#if (defined __TEST_DEVICE_DRIVER__  || defined __ALWAYS_AWAKE_DEVICE__)
static struct wake_lock wake_lock_for_dev;
#endif /* __TEST_DEVICE_DRIVER__ || __ALWAYS_AWAKE_DEVICE__ */

#ifdef __TEST_DEVICE_DRIVER__
#include <linux/i2c/pmic.h>
#endif /* __TEST_DEVICE_DRIVER__ */

#include <linux/i2c.h>
#include "fuel_gauge_saturn.c"
#ifdef S3C_CALL_STATE
int call_state = 0;
#define OFFSET_VOICE_CALL_2G		(0x1 << 1)
#define OFFSET_VOICE_CALL_3G		(0x1 << 0)
#endif
/* Prototypes */
extern int s3c_adc_get_adc_data(int channel);
extern int get_usb_power_state(void);

static ssize_t s3c_bat_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf);
static ssize_t s3c_bat_store(struct device *dev, 
			     struct device_attribute *attr,
			     const char *buf, size_t count);
static void s3c_set_chg_en(int enable);

#ifdef __TEST_DEVICE_DRIVER__
extern int amp_enable(int);
extern int audio_power(int);

static ssize_t s3c_test_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf);
static ssize_t s3c_test_store(struct device *dev, 
			     struct device_attribute *attr,
			     const char *buf, size_t count);
static int bat_temper_state = 0; // for test 1: overheat, 2: freeze
#endif /* __TEST_DEVICE_DRIVER__ */

#ifdef __TEST_MODE_INTERFACE__
static struct power_supply *s3c_power_supplies_test = NULL;
static void polling_timer_func(unsigned long unused);
static void s3c_bat_status_update(struct power_supply *bat_ps);
#endif /* __TEST_MODE_INTERFACE__ */

#define ADC_DATA_ARR_SIZE	6
#define ADC_TOTAL_COUNT		10
#define POLLING_INTERVAL	2000
#ifdef __TEST_MODE_INTERFACE__
#define POLLING_INTERVAL_TEST	1000
#endif /* __TEST_MODE_INTERFACE__ */

#ifdef __9BITS_RESOLUTION__
#define INVALID_VOL_ADC		20
#else /* __9BITS_RESOLUTION__ */
#define INVALID_VOL_ADC		160
#endif /* __9BITS_RESOLUTION__ */

#define ENABLE		1
#define DISABLE		0

#ifndef convert_adc2voltage
	#ifdef __9BITS_RESOLUTION__
	#define convert_adc2voltage(x)		(x * 10000 / 848)
	#else /* __9BITS_RESOLUTION__ */
	#define convert_adc2voltage(x)		((x - 2170 ) * 10 / 7 + 3200)
	#endif /* __9BITS_RESOLUTION__ */
#endif /* convert_adc2voltage */

static struct work_struct bat_work;
static struct work_struct cable_work;

#ifdef COMPENSATE_BOOTING
static struct workqueue_struct *batt_drv_wqueue;
static struct work_struct compensate_boot_work;
#endif /* COMPENSATE_BOOTING */

static struct device *dev;
static struct timer_list polling_timer;
static struct timer_list cable_timer;
static int cable_intr_cnt = 0;

static int s3c_battery_initial;
static int force_update;
static int full_charge_flag;

static int batt_max;
static int batt_full;
static int batt_safe_rech;
static int batt_almost;
static int batt_high;
static int batt_medium;
static int batt_low;
static int batt_critical;
static int batt_min;
static int batt_off;
static unsigned int start_time_msec;
static unsigned int total_time_msec;

static char *status_text[] = {
	[POWER_SUPPLY_STATUS_UNKNOWN] =		"Unknown",
	[POWER_SUPPLY_STATUS_CHARGING] =	"Charging",
	[POWER_SUPPLY_STATUS_DISCHARGING] =	"Discharging",
	[POWER_SUPPLY_STATUS_NOT_CHARGING] =	"Not Charging",
	[POWER_SUPPLY_STATUS_FULL] =		"Full",
};

typedef enum {
	CHARGER_BATTERY = 0,
	CHARGER_USB,
	CHARGER_AC,
	CHARGER_DISCHARGE,
	CHARGER_UNKNOWN
} charger_type_t;

struct battery_info {
	u32 batt_id;		/* Battery ID from ADC */
	s32 batt_vol;		/* Battery voltage from ADC */
	s32 batt_vol_adc;	/* Battery ADC value */
	s32 batt_vol_adc_cal;	/* Battery ADC value (calibrated)*/
	s32 batt_temp;		/* Battery Temperature (C) from ADC */
	s32 batt_temp_adc;	/* Battery Temperature ADC value */
	s32 batt_temp_adc_cal;	/* Battery Temperature ADC value (calibrated) */
	s32 batt_current;	/* Battery current from ADC */
	u32 level;		/* formula */
	u32 charging_source;	/* 0: no cable, 1:usb, 2:AC */
	u32 charging_enabled;	/* 0: Disable, 1: Enable */
	u32 batt_health;	/* Battery Health (Authority) */
	u32 batt_is_full;       /* 0 : Not full 1: Full */
	u32 batt_is_recharging; /* 0 : Not recharging 1: Recharging */
	s32 batt_vol_adc_aver;	/* batt vol adc average */
#ifdef __TEST_MODE_INTERFACE__
	u32 batt_test_mode;	/* test mode */
	s32 batt_vol_aver;	/* batt vol average */
	s32 batt_temp_aver;	/* batt temp average */
	s32 batt_temp_adc_aver;	/* batt temp adc average */
	s32 batt_v_f_adc;	/* batt V_F adc */
#endif /* __TEST_MODE_INTERFACE__ */
};

/* lock to protect the battery info */
static DEFINE_MUTEX(work_lock);

struct s3c_battery_info {
	int present;
	int polling;
	unsigned int polling_interval;
	unsigned int device_state;

	struct battery_info bat_info;
	int max_soc_value;			//phj: the highest possible soc, measured when fully charged, default 96%
};
static struct s3c_battery_info s3c_bat_info;

struct adc_sample_info {
	unsigned int cnt;
	int total_adc;
	int average_adc;
	int adc_arr[ADC_TOTAL_COUNT];
	int index;
};
static struct adc_sample_info adc_sample[ENDOFADC];

static inline int s3c_adc_get_adc_data_ex(int channel) {
#ifndef __9BITS_RESOLUTION__
	return s3c_adc_get_adc_data(channel);
#else
#ifdef __REVERSE_TEMPER_ADC__
	if (channel == S3C_ADC_TEMPERATURE) 
		return (4096 - s3c_adc_get_adc_data(channel)) / 8;
	else
#endif /* __REVERSE_TEMPER_ADC__ */
		return s3c_adc_get_adc_data(channel) / 8;
#endif /* __9BITS_RESOLUTION__ */
}

#if defined(__AVG_TEMP_ADC__) || !defined(__FUEL_GAUGES_IC__)
static unsigned long calculate_average_adc(adc_channel_type channel, int adc)
{
	unsigned int cnt = 0;
	int total_adc = 0;
	int average_adc = 0;
	int index = 0;

	cnt = adc_sample[channel].cnt;
	total_adc = adc_sample[channel].total_adc;

	if (adc < 0 || adc == 0) {
		if (adc<0)
			dev_err(dev, "%s: invalid adc : %d\n", __func__, adc);
		adc = adc_sample[channel].average_adc;
	}

	if( cnt < ADC_TOTAL_COUNT ) {
		adc_sample[channel].adc_arr[cnt] = adc;
		adc_sample[channel].index = cnt;
		adc_sample[channel].cnt = ++cnt;

		total_adc += adc;
		average_adc = total_adc / cnt;
	} else {
#if 0
		if (channel == S3C_ADC_VOLTAGE &&
				!s3c_bat_info.bat_info.charging_enabled && 
				adc > adc_sample[channel].average_adc) {
			dev_dbg(dev, "%s: adc over avg : %d\n", __func__, adc);
			return adc_sample[channel].average_adc;
		}
#endif
		index = adc_sample[channel].index;
		if (++index >= ADC_TOTAL_COUNT)
			index = 0;

		total_adc = (total_adc - adc_sample[channel].adc_arr[index]) + adc;
		average_adc = total_adc / ADC_TOTAL_COUNT;

		adc_sample[channel].adc_arr[index] = adc;
		adc_sample[channel].index = index;
	}

	adc_sample[channel].total_adc = total_adc;
	adc_sample[channel].average_adc = average_adc;

	dev_dbg(dev, "%s: ch:%d adc=%d, avg_adc=%d\n",
			__func__, channel, adc, average_adc);
	return average_adc;
}
#endif /* defined(__AVG_TEMP_ADC__) || !defined(__FUEL_GAUGES_IC__) */

static int s3c_bat_get_adc_data(adc_channel_type adc_ch)
{
	int adc_arr[ADC_DATA_ARR_SIZE];
	int adc_max = 0;
	int adc_min = 0;
	int adc_total = 0;
	int i;

	for (i = 0; i < ADC_DATA_ARR_SIZE; i++) {
		adc_arr[i] = s3c_adc_get_adc_data_ex(adc_ch);
		dev_dbg(dev, "%s: adc_arr = %d\n", __func__, adc_arr[i]);
		if (i != 0) {
			if (adc_arr[i] > adc_max) 
				adc_max = adc_arr[i];
			else if (adc_arr[i] < adc_min)
				adc_min = adc_arr[i];
		} else {
			adc_max = adc_arr[0];
			adc_min = adc_arr[0];
		}
		adc_total += adc_arr[i];
	}

	dev_dbg(dev, "%s: adc_max = %d, adc_min = %d\n",
			__func__, adc_max, adc_min);
	return (adc_total - adc_max - adc_min) / (ADC_DATA_ARR_SIZE - 2);
}


static unsigned long s3c_read_temp(struct power_supply *bat_ps)
{
	int adc = 0;

	dev_dbg(dev, "%s\n", __func__);

	adc = s3c_bat_get_adc_data(S3C_ADC_TEMPERATURE);
	dev_dbg(dev, "%s: adc = %d\n", __func__, adc);

#ifdef __TEST_DEVICE_DRIVER__
	switch (bat_temper_state) {
	case 0:
		break;
	case 1:
		adc = TEMP_HIGH_BLOCK;
		break;
	case 2:
		adc = TEMP_LOW_BLOCK;
		break;
	default:
		break;
	}
#endif /* __TEST_DEVICE_DRIVER__ */

	s3c_bat_info.bat_info.batt_temp_adc = adc;

#ifdef __AVG_TEMP_ADC__
	return calculate_average_adc(S3C_ADC_TEMPERATURE, adc);
#else /* __AVG_TEMP_ADC__ */
	return adc;
#endif /* __AVG_TEMP_ADC__ */
}

static u32 s3c_get_bat_health(void)
{
	return s3c_bat_info.bat_info.batt_health;
}

static void s3c_set_bat_health(u32 batt_health)
{
	s3c_bat_info.bat_info.batt_health = batt_health;
}

static int is_over_abs_time(void)
{
	unsigned int total_time;

	if (!start_time_msec)
		return 0;

	if (s3c_bat_info.bat_info.batt_is_recharging)
		total_time = TOTAL_RECHARGING_TIME;
	else
		total_time = TOTAL_CHARGING_TIME;

	total_time_msec = jiffies_to_msecs(jiffies) - start_time_msec;
	if (total_time_msec > total_time)
		return 1;
	else
		return 0;
}

static void check_recharging_bat(int fg_vcell)
{
	static int cnt = 0;

	if (s3c_get_bat_health() != POWER_SUPPLY_HEALTH_GOOD) {
		cnt = 0;
		return;
	}

	if (s3c_bat_info.bat_info.batt_is_full &&
		!s3c_bat_info.bat_info.charging_enabled &&
		(fg_vcell <= RECHARGE_COND_VOLTAGE || 
			fg_vcell <= FULL_CHARGE_COND_VOLTAGE)) {
		if (++cnt >= 10) {
			dev_info(dev, "%s: recharging(vcell:%d)\n", __func__,
					fg_vcell);
			s3c_bat_info.bat_info.batt_is_recharging = 1;
			s3c_set_chg_en(ENABLE);
			cnt = 0;
		}
	} else {
		cnt = 0;
	}
}


unsigned int fg_vcell_table[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

unsigned int avg_fg_vcell(int last_fg_vcell)
{
	unsigned int result = 0, min_adc, max_adc;
	int i;

	for(i=0; i<10; i++)
	{
		if(i==9)
			fg_vcell_table[i]= (unsigned int) last_fg_vcell;
		else
		{
			fg_vcell_table[i] = fg_vcell_table[i+1];
			if(!fg_vcell_table[i])
				fg_vcell_table[i] = (unsigned int) last_fg_vcell;
		}
		
		result += fg_vcell_table[i];

		if(!i)
		{
			min_adc = fg_vcell_table[i];
			max_adc = fg_vcell_table[i];
		}
		else
		{
			min_adc = min(min_adc, fg_vcell_table[i]);
			max_adc = max(max_adc, fg_vcell_table[i]);
		}
	}

		result -= (min_adc + max_adc);
		result = result/8;

	dev_dbg(dev, "%s: last_fg_vcell=%d, avg_result=%d\n", __func__, last_fg_vcell, result);

	return result;
}

static int s3c_get_bat_level(struct power_supply *bat_ps)
{
	int fg_soc = -1;
	int fg_vcell = -1;

	if ((fg_soc = fg_read_soc()) < 0) {
		dev_err(dev, "%s: Can't read soc!!!\n", __func__);
		fg_soc = s3c_bat_info.bat_info.level;
	}
	
	if ((fg_vcell = fg_read_vcell()) < 0) {
		dev_err(dev, "%s: Can't read vcell!!!\n", __func__);
		fg_vcell = s3c_bat_info.bat_info.batt_vol;
	} else
		s3c_bat_info.bat_info.batt_vol = fg_vcell;

	if (is_over_abs_time()) {
		//phj:here we can reach the MAXIMUM of soc for a given battery
		if (fg_soc)
			s3c_bat_info.max_soc_value=fg_soc;
		fg_soc = 100;
		s3c_bat_info.bat_info.batt_is_full = 1;
		dev_info(dev, "%s: charging time is over\n", __func__);
		s3c_set_chg_en(DISABLE);
		goto __end__;
	}

//	fg_soc = (fg_soc*100)/s3c_bat_info.max_soc_value;
//	if (fg_soc > 100)	fg_soc = 100;
//	fg_soc = 140 * (1 - exp(-fg_soc / 70)) - 4;  /* implemented in fg_tab */
	fg_soc = fg_tab[fg_soc];

	check_recharging_bat(avg_fg_vcell(fg_vcell));

__end__:
	dev_dbg(dev, "%s: fg_vcell = %d, fg_soc = %d, is_full = %d\n",
			__func__, fg_vcell, fg_soc, 
			s3c_bat_info.bat_info.batt_is_full);
	return fg_soc;
}

static int s3c_get_bat_vol(struct power_supply *bat_ps)
{
	return s3c_bat_info.bat_info.batt_vol;
}

#if (defined __TEST_MODE_INTERFACE__ && defined __BATTERY_V_F__)
static void s3c_get_v_f_adc(void)
{
	s3c_bat_info.bat_info.batt_v_f_adc
		= s3c_bat_get_adc_data(S3C_ADC_V_F);
	dev_info(dev, "%s: vf=%d\n", __func__,
			s3c_bat_info.bat_info.batt_v_f_adc);
}
#endif /* __TEST_MODE_INTERFACE__ && __BATTERY_V_F__ */

static inline int gpio_get_value_ex(unsigned int pin)
{
	return gpio_get_value(pin);
}

static inline void gpio_set_value_ex(unsigned int pin, unsigned int level)
{
	gpio_set_value(pin, level);
}

static void s3c_set_time_for_charging(int mode) {
	if (mode) {
		/* record start time for abs timer */
		start_time_msec = jiffies_to_msecs(jiffies);
		dev_info(dev, "%s: start_time(%d)\n", __func__,
				start_time_msec);
	} else {
		/* initialize start time for abs timer */
		start_time_msec = 0;
		total_time_msec = 0;
		dev_info(dev, "%s: reset abs timer\n", __func__);
	}
}

static void s3c_set_chg_en(int enable)
{
	int chg_en_val = gpio_get_value_ex(gpio_chg_en);

	if (enable) {
		if (chg_en_val == GPIO_LEVEL_HIGH) {
			gpio_set_value_ex(gpio_chg_en, GPIO_LEVEL_LOW);
//			dev_info(dev, "%s: gpio_chg_en(0)\n", __func__);
			s3c_set_time_for_charging(1);
		}
	} else {
		if (chg_en_val == GPIO_LEVEL_LOW) {
			gpio_set_value_ex(gpio_chg_en, GPIO_LEVEL_HIGH);
//			dev_info(dev, "%s: gpio_chg_en(1)\n", __func__);
			s3c_set_time_for_charging(0);
			s3c_bat_info.bat_info.batt_is_recharging = 0;
		}
	}
	s3c_bat_info.bat_info.charging_enabled = enable;
	dev_info(dev, "%s: charging %sbled\n", __func__,enable?"ena":"disa");

}

static void s3c_temp_control(int mode) {
	int adc = s3c_bat_info.bat_info.batt_temp_adc_aver;
	dev_info(dev, "%s: temp_adc=%d\n", __func__, adc);

	switch (mode) {
	case POWER_SUPPLY_HEALTH_GOOD:
		dev_info(dev, "%s: GOOD\n", __func__);
		s3c_set_bat_health(mode);
		break;
	case POWER_SUPPLY_HEALTH_OVERHEAT:
		dev_info(dev, "%s: OVERHEAT\n", __func__);
		s3c_set_bat_health(mode);
		break;
	case POWER_SUPPLY_HEALTH_COLD:
		dev_info(dev, "%s: COLD\n", __func__);
		s3c_set_bat_health(mode);
		break;
	default:
		break;
	}
	schedule_work(&cable_work);
}

static int s3c_get_bat_temp(struct power_supply *bat_ps)
{
	int temp = 0;
	int array_size = 0;
	int i = 0;
	int temp_adc = s3c_read_temp(bat_ps);
	int health = s3c_get_bat_health();
#ifdef __TEST_MODE_INTERFACE__
	s3c_bat_info.bat_info.batt_temp_adc_aver = temp_adc;
#endif /* __TEST_MODE_INTERFACE__ */

#ifndef __REVERSE_TEMPER_ADC__
	if (temp_adc <= TEMP_HIGH_BLOCK) {
		if (health != POWER_SUPPLY_HEALTH_OVERHEAT &&
				health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			s3c_temp_control(POWER_SUPPLY_HEALTH_OVERHEAT);
	} else if (temp_adc >= TEMP_HIGH_RECOVER &&
			temp_adc <= TEMP_LOW_RECOVER) {
		if (health == POWER_SUPPLY_HEALTH_OVERHEAT ||
				health == POWER_SUPPLY_HEALTH_COLD)
			s3c_temp_control(POWER_SUPPLY_HEALTH_GOOD);
	} else if (temp_adc >= TEMP_LOW_BLOCK) {
		if (health != POWER_SUPPLY_HEALTH_COLD &&
				health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			s3c_temp_control(POWER_SUPPLY_HEALTH_COLD);
	}

	array_size = ARRAY_SIZE(temper_table);
	for (i = 0; i < (array_size - 1); i++) {
		if (i == 0) {
			if (temp_adc >= temper_table[0][0]) {
				temp = temper_table[0][1];
				break;
			} else if (temp_adc <= temper_table[array_size-1][0]) {
				temp = temper_table[array_size-1][1];
				break;
			}
		}

		if (temper_table[i][0] > temp_adc &&
				temper_table[i+1][0] <= temp_adc) {
			temp = temper_table[i+1][1];
		}
	}
#else /* __REVERSE_TEMPER_ADC__ */
	if (temp_adc >= TEMP_HIGH_BLOCK) {
		if (health != POWER_SUPPLY_HEALTH_OVERHEAT &&
				health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			s3c_temp_control(POWER_SUPPLY_HEALTH_OVERHEAT);
	} else if (temp_adc <= TEMP_HIGH_RECOVER &&
			temp_adc >= TEMP_LOW_RECOVER) {
		if (health == POWER_SUPPLY_HEALTH_OVERHEAT ||
				health == POWER_SUPPLY_HEALTH_COLD)
			s3c_temp_control(POWER_SUPPLY_HEALTH_GOOD);
	} else if (temp_adc <= TEMP_LOW_BLOCK) {
		if (health != POWER_SUPPLY_HEALTH_COLD &&
				health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
			s3c_temp_control(POWER_SUPPLY_HEALTH_COLD);
	}

	array_size = ARRAY_SIZE(temper_table);
	for (i = 0; i < (array_size - 1); i++) {
		if (i == 0) {
			if (temp_adc <= temper_table[0][0]) {
				temp = temper_table[0][1];
				break;
			} else if (temp_adc >= temper_table[array_size-1][0]) {
				temp = temper_table[array_size-1][1];
				break;
			}
		}

		if (temper_table[i][0] < temp_adc &&
				temper_table[i+1][0] >= temp_adc) {
			temp = temper_table[i+1][1];
		}
	}
#endif /* __REVERSE_TEMPER_ADC__ */
	dev_dbg(dev, "%s: temp = %d, adc = %d\n",
			__func__, temp, temp_adc);

#ifdef __TEST_MODE_INTERFACE__
       	s3c_bat_info.bat_info.batt_temp_aver = temp;
#endif /* __TEST_MODE_INTERFACE__ */
	return temp;
}

static int s3c_bat_get_charging_status(void)
{
        charger_type_t charger = CHARGER_BATTERY; 
        int ret = 0;
        
        charger = s3c_bat_info.bat_info.charging_source;
        
        switch (charger) {
        case CHARGER_BATTERY:
                ret = POWER_SUPPLY_STATUS_NOT_CHARGING;
                break;
        case CHARGER_USB:
        case CHARGER_AC:
		if (s3c_get_bat_health() != POWER_SUPPLY_HEALTH_GOOD)
			ret = POWER_SUPPLY_STATUS_DISCHARGING;
		else {
			if (s3c_bat_info.bat_info.batt_is_full)
				ret = POWER_SUPPLY_STATUS_FULL;
			else
				ret = POWER_SUPPLY_STATUS_CHARGING;
		}
                break;
	case CHARGER_DISCHARGE:
		ret = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
        default:
                ret = POWER_SUPPLY_STATUS_UNKNOWN;
        }

	dev_dbg(dev, "%s: %s\n", __func__, status_text[ret]);
        return ret;
}

static int s3c_bat_get_property(struct power_supply *bat_ps, 
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	dev_dbg(bat_ps->dev, "%s: psp = %d\n", __func__, psp);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = s3c_bat_get_charging_status();
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = s3c_get_bat_health();
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = s3c_bat_info.present;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = s3c_bat_info.bat_info.level;
		dev_dbg(dev, "%s: level = %d\n", __func__, 
				val->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = s3c_bat_info.bat_info.batt_temp;
		dev_dbg(bat_ps->dev, "%s: temp = %d\n", __func__, 
				val->intval);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int s3c_power_get_property(struct power_supply *bat_ps, 
		enum power_supply_property psp, 
		union power_supply_propval *val)
{
	charger_type_t charger;
	
	dev_dbg(bat_ps->dev, "%s: psp = %d\n", __func__, psp);

	charger = s3c_bat_info.bat_info.charging_source;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (bat_ps->type == POWER_SUPPLY_TYPE_MAINS)
			val->intval = (charger == CHARGER_AC ? 1 : 0);
		else if (bat_ps->type == POWER_SUPPLY_TYPE_USB)
			val->intval = (charger == CHARGER_USB ? 1 : 0);
		else
			val->intval = 0;
		break;
	default:
		return -EINVAL;
	}
	
	return 0;
}

#define SEC_BATTERY_ATTR(_name)								\
{											\
        .attr = { .name = #_name, .mode = S_IRUGO | S_IWUGO, .owner = THIS_MODULE },	\
        .show = s3c_bat_show_property,							\
        .store = s3c_bat_store,								\
}

static struct device_attribute s3c_battery_attrs[] = {
        SEC_BATTERY_ATTR(batt_vol),
        SEC_BATTERY_ATTR(batt_vol_adc),
        SEC_BATTERY_ATTR(batt_vol_adc_cal),
        SEC_BATTERY_ATTR(batt_temp),
        SEC_BATTERY_ATTR(batt_temp_adc),
        SEC_BATTERY_ATTR(batt_temp_adc_cal),
	SEC_BATTERY_ATTR(batt_vol_adc_aver),
#ifdef __TEST_MODE_INTERFACE__
	/* test mode */
	SEC_BATTERY_ATTR(batt_test_mode),
	/* average */
	SEC_BATTERY_ATTR(batt_vol_aver),
	SEC_BATTERY_ATTR(batt_temp_aver),
	SEC_BATTERY_ATTR(batt_temp_adc_aver),
	SEC_BATTERY_ATTR(batt_v_f_adc),
#endif /* __TEST_MODE_INTERFACE__ */
	SEC_BATTERY_ATTR(charging_source),
	SEC_BATTERY_ATTR(fg_soc),
	SEC_BATTERY_ATTR(reset_soc),
	SEC_BATTERY_ATTR(max_soc),
};

enum {
    BATT_VOL = 0,
    BATT_VOL_ADC,
    BATT_VOL_ADC_CAL,
    BATT_TEMP,
    BATT_TEMP_ADC,
    BATT_TEMP_ADC_CAL,
	BATT_VOL_ADC_AVER,
#ifdef __TEST_MODE_INTERFACE__
	BATT_TEST_MODE,
	BATT_VOL_AVER,
	BATT_TEMP_AVER,
	BATT_TEMP_ADC_AVER,
	BATT_V_F_ADC,
#endif /* __TEST_MODE_INTERFACE__ */
	BATT_CHARGING_SOURCE,
	BATT_FG_SOC,
	BATT_RESET_SOC,
	BATT_MAX_SOC,
#ifdef S3C_CALL_STATE
	BATT_VOICE_CALL_2G,
	BATT_VOICE_CALL_3G,
#endif
};

static int s3c_bat_create_attrs(struct device * dev)
{
        int i, rc;
        
        for (i = 0; i < ARRAY_SIZE(s3c_battery_attrs); i++) {
                rc = device_create_file(dev, &s3c_battery_attrs[i]);
                if (rc)
                        goto s3c_attrs_failed;
        }
        goto succeed;
        
s3c_attrs_failed:
        while (i--)
                device_remove_file(dev, &s3c_battery_attrs[i]);
succeed:        
        return rc;
}

static ssize_t s3c_bat_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
        int i = 0;
        const ptrdiff_t off = attr - s3c_battery_attrs;

        switch (off) {
        case BATT_VOL:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
                               s3c_bat_info.bat_info.batt_vol);
                break;
        case BATT_VOL_ADC:
		s3c_bat_info.bat_info.batt_vol_adc = 0;
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
                               s3c_bat_info.bat_info.batt_vol_adc);
                break;
        case BATT_VOL_ADC_CAL:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
                               s3c_bat_info.bat_info.batt_vol_adc_cal);
                break;
        case BATT_TEMP:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
                               s3c_bat_info.bat_info.batt_temp);
                break;
        case BATT_TEMP_ADC:
		s3c_bat_info.bat_info.batt_temp_adc = 
			s3c_bat_get_adc_data(S3C_ADC_TEMPERATURE);
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
                               s3c_bat_info.bat_info.batt_temp_adc);
                break;	
#ifdef __TEST_MODE_INTERFACE__
	case BATT_TEST_MODE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
		s3c_bat_info.bat_info.batt_test_mode);
		break;
#endif /* __TEST_MODE_INTERFACE__ */
        case BATT_TEMP_ADC_CAL:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
                               s3c_bat_info.bat_info.batt_temp_adc_cal);
                break;
        case BATT_VOL_ADC_AVER:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.bat_info.batt_vol_adc_aver);
		break;
#ifdef __TEST_MODE_INTERFACE__
	case BATT_VOL_AVER:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.bat_info.batt_vol_aver);
		break;
	case BATT_TEMP_AVER:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.bat_info.batt_temp_aver);
		break;
	case BATT_TEMP_ADC_AVER:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.bat_info.batt_temp_adc_aver);
		break;
	case BATT_V_F_ADC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.bat_info.batt_v_f_adc);
		break;
#endif /* __TEST_MODE_INTERFACE__ */
	case BATT_CHARGING_SOURCE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.bat_info.charging_source);
		break;

	case BATT_FG_SOC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			fg_read_soc());
		break;
	case BATT_MAX_SOC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			s3c_bat_info.max_soc_value);
		break;

        default:
                i = -EINVAL;
        }       
        
        return i;
}

static void s3c_bat_set_vol_cal(int batt_cal)
{
	int max_cal = 4096;
#ifdef __9BITS_RESOLUTION__
	max_cal = 512;
#endif /* __9BITS_RESOLUTION__ */

	if (!batt_cal)
		return;

	if (batt_cal >= max_cal) {
		dev_err(dev, "%s: invalid battery_cal(%d)\n", __func__, batt_cal);
		return;
	}

	batt_max = batt_cal + BATT_MAXIMUM;
	batt_full = batt_cal + BATT_FULL;
	batt_safe_rech = batt_cal + BATT_SAFE_RECHARGE;
	batt_almost = batt_cal + BATT_ALMOST_FULL;
	batt_high = batt_cal + BATT_HIGH;
	batt_medium = batt_cal + BATT_MED;
	batt_low = batt_cal + BATT_LOW;
	batt_critical = batt_cal + BATT_CRITICAL;
	batt_min = batt_cal + BATT_MINIMUM;
	batt_off = batt_cal + BATT_OFF;
}

#ifdef COMPENSATE_BOOTING
static void s3c_bat_compensate_booting(int mode)
{
	if (mode)
		s3c_bat_set_compesation(1, OFFSET_BOOTING, COMPENSATE_BOOTING);
	else
		queue_work(batt_drv_wqueue, &compensate_boot_work);
}
#endif /* COMPENSATE_BOOTING */

static ssize_t s3c_bat_store(struct device *dev, 
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int x = 0;
	int ret = 0;
	const ptrdiff_t off = attr - s3c_battery_attrs;

        switch (off) {
        case BATT_VOL_ADC_CAL:
		if (sscanf(buf, "%d\n", &x) == 1) {
			s3c_bat_info.bat_info.batt_vol_adc_cal = x;
			s3c_bat_set_vol_cal(x);
			ret = count;
		}
		dev_info(dev, "%s: batt_vol_adc_cal = %d\n", __func__, x);
                break;
        case BATT_TEMP_ADC_CAL:
		if (sscanf(buf, "%d\n", &x) == 1) {
			s3c_bat_info.bat_info.batt_temp_adc_cal = x;
			ret = count;
		}
		dev_info(dev, "%s: batt_temp_adc_cal = %d\n", __func__, x);
                break;
#ifdef __TEST_MODE_INTERFACE__
	case BATT_TEST_MODE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			s3c_bat_info.bat_info.batt_test_mode = x;
			ret = count;
		}
		if (s3c_bat_info.bat_info.batt_test_mode) {
			s3c_bat_info.polling_interval = POLLING_INTERVAL_TEST;			
			if (s3c_bat_info.polling) {
				del_timer_sync(&polling_timer);
				mod_timer(&polling_timer, jiffies + 
					msecs_to_jiffies(s3c_bat_info.polling_interval));
			}
			s3c_bat_status_update(
				&s3c_power_supplies_test[CHARGER_BATTERY]);
		} else {
			s3c_bat_info.polling_interval = POLLING_INTERVAL;		
			if (s3c_bat_info.polling) {
				del_timer_sync(&polling_timer);
				mod_timer(&polling_timer,jiffies + 
					msecs_to_jiffies(s3c_bat_info.polling_interval));
			}
			s3c_bat_status_update(
				&s3c_power_supplies_test[CHARGER_BATTERY]);
		}
		dev_info(dev, "%s: batt_test_mode = %d\n", __func__, x);
		break;
#endif /* __TEST_MODE_INTERFACE__ */
	case BATT_RESET_SOC:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 1)
				fg_reset_soc();
			ret = count;
		}
		dev_info(dev, "%s: Reset SOC:%d\n", __func__, x);
		break;
	case BATT_MAX_SOC:
		if (sscanf(buf, "%d\n", &x)) {
			if (x && x<=100)
				s3c_bat_info.max_soc_value=x;
			ret = count;
		}
		dev_info(dev, "%s: Max SOC:%d\n", __func__, x);
		break;
#ifdef S3C_CALL_STATE
	case BATT_VOICE_CALL_2G:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if(x)
				call_state |= OFFSET_VOICE_CALL_2G;
			else
				call_state &= ~OFFSET_VOICE_CALL_2G; 
			ret = count;
		}
		printk("%s: voice call 2G = %d\n", __func__, x);
		break;
	case BATT_VOICE_CALL_3G:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if(x)
				call_state |= OFFSET_VOICE_CALL_3G;
			else
				call_state &= ~OFFSET_VOICE_CALL_3G; 
			ret = count;
		}
		printk("%s: voice call 3G = %d\n", __func__, x);
		break;
#endif
        default:
                ret = -EINVAL;
        }       
	return ret;
}

#ifdef __TEST_DEVICE_DRIVER__
#define SEC_TEST_ATTR(_name)								\
{											\
        .attr = { .name = #_name, .mode = S_IRUGO | S_IWUGO, .owner = THIS_MODULE },	\
        .show = s3c_test_show_property,							\
        .store = s3c_test_store,							\
}

static struct device_attribute s3c_test_attrs[] = {
        SEC_TEST_ATTR(pm),
        SEC_TEST_ATTR(usb),
        SEC_TEST_ATTR(bt_wl),
        SEC_TEST_ATTR(tflash),
        SEC_TEST_ATTR(audio),
        SEC_TEST_ATTR(lcd),
        SEC_TEST_ATTR(suspend_lock),
        SEC_TEST_ATTR(control_tmp),
};

enum {
        TEST_PM = 0,
	USB_OFF,
	BT_WL_OFF,
	TFLASH_OFF,
	AUDIO_OFF,
	LCD_CHECK,
	SUSPEND_LOCK,
	CTRL_TMP,
};

static int s3c_test_create_attrs(struct device * dev)
{
        int i, rc;
        
        for (i = 0; i < ARRAY_SIZE(s3c_test_attrs); i++) {
                rc = device_create_file(dev, &s3c_test_attrs[i]);
                if (rc)
                        goto s3c_attrs_failed;
        }
        goto succeed;
        
s3c_attrs_failed:
        while (i--)
                device_remove_file(dev, &s3c_test_attrs[i]);
succeed:        
        return rc;
}

static void s3c_lcd_check(void)
{
	unsigned char reg_buff = 0;
	if (Get_MAX8698_PM_REG(ELDO6, &reg_buff)) {
		pr_info("%s: VLCD 1.8V (%d)\n", __func__, reg_buff);
	}
	if ((Get_MAX8698_PM_REG(ELDO7, &reg_buff))) {
		pr_info("%s: VLCD 2.8V (%d)\n", __func__, reg_buff);
	}
}

static void s3c_usb_off(void)
{
	unsigned char reg_buff = 0;
	if (Get_MAX8698_PM_REG(ELDO3, &reg_buff)) {
		pr_info("%s: OTGI 1.2V off(%d)\n", __func__, reg_buff);
		if (reg_buff)
			Set_MAX8698_PM_REG(ELDO3, 0);
	}
	if ((Get_MAX8698_PM_REG(ELDO8, &reg_buff))) {
		pr_info("%s: OTG 3.3V off(%d)\n", __func__, reg_buff);
		if (reg_buff)
			Set_MAX8698_PM_REG(ELDO8, 0);
	}
}

static void s3c_bt_wl_off(void)
{
	unsigned char reg_buff = 0;
	if (Get_MAX8698_PM_REG(ELDO4, &reg_buff)) {
		pr_info("%s: BT_WL 2.6V off(%d)\n", __func__, reg_buff);
		if (reg_buff)
			Set_MAX8698_PM_REG(ELDO4, 0);
	}
}

static void s3c_tflash_off(void)
{
	unsigned char reg_buff = 0;
	if (Get_MAX8698_PM_REG(ELDO5, &reg_buff)) {
		pr_info("%s: TF 3.0V off(%d)\n", __func__, reg_buff);
		if (reg_buff)
			Set_MAX8698_PM_REG(ELDO5, 0);
	}
}

static void s3c_audio_off(void)
{
	pr_info("%s: Turn off audio power, amp\n", __func__);
	amp_enable(0);
	audio_power(0);
}

static void s3c_test_pm(void)
{
	/* PMIC */
	s3c_usb_off();
	s3c_bt_wl_off();
	s3c_tflash_off();
	s3c_lcd_check();

	/* AUDIO */
	s3c_audio_off();

	/* GPIO */
}

static ssize_t s3c_test_show_property(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
        int i = 0;
        const ptrdiff_t off = attr - s3c_test_attrs;

        switch (off) {
        case TEST_PM:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 0);
		s3c_test_pm();
                break;
        case USB_OFF:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 1);
		s3c_usb_off();
                break;
        case BT_WL_OFF:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 2);
		s3c_bt_wl_off();
                break;
        case TFLASH_OFF:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 3);
		s3c_tflash_off();
                break;
        case AUDIO_OFF:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 4);
		s3c_audio_off();
                break;
        case LCD_CHECK:
                i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", 5);
		s3c_lcd_check();
                break;
        default:
                i = -EINVAL;
        }       
        
        return i;
}

static ssize_t s3c_test_store(struct device *dev, 
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int mode = 0;
	int ret = 0;
	const ptrdiff_t off = attr - s3c_test_attrs;

        switch (off) {
        case SUSPEND_LOCK:
		if (sscanf(buf, "%d\n", &mode) == 1) {
			dev_dbg(dev, "%s: suspend_lock(%d)\n", __func__, mode);
			if (mode) 
                		wake_lock(&wake_lock_for_dev);
			else
                		wake_lock_timeout(
						&wake_lock_for_dev, HZ / 2);
			ret = count;
		}
                break;
	case CTRL_TMP:
		if (sscanf(buf, "%d\n", &mode) == 1) {
			dev_info(dev, "%s: control tmp(%d)\n", __func__, mode);
			bat_temper_state = mode;
			ret = count;
		}
		break;
        default:
                ret = -EINVAL;
        }       

	return ret;
}
#endif /* __TEST_DEVICE_DRIVER__ */

static enum power_supply_property s3c_battery_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
};

static enum power_supply_property s3c_power_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *supply_list[] = {
	"battery",
};

static struct power_supply s3c_power_supplies[] = {
	{
		.name = "battery",
		.type = POWER_SUPPLY_TYPE_BATTERY,
		.properties = s3c_battery_properties,
		.num_properties = ARRAY_SIZE(s3c_battery_properties),
		.get_property = s3c_bat_get_property,
	},
	{
		.name = "usb",
		.type = POWER_SUPPLY_TYPE_USB,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = s3c_power_properties,
		.num_properties = ARRAY_SIZE(s3c_power_properties),
		.get_property = s3c_power_get_property,
	},
	{
		.name = "ac",
		.type = POWER_SUPPLY_TYPE_MAINS,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = s3c_power_properties,
		.num_properties = ARRAY_SIZE(s3c_power_properties),
		.get_property = s3c_power_get_property,
	},
};

static int s3c_cable_status_update(int status)
{
	int chng;
	charger_type_t source=CHARGER_UNKNOWN;

	dev_dbg(dev, "%s\n", __func__);

	if(!s3c_battery_initial)
		return -EPERM;

	switch(status) {
	case CHARGER_BATTERY:
		dev_dbg(dev, "%s: cable NOT PRESENT\n", __func__);
		source = CHARGER_BATTERY;
		break;
	case CHARGER_USB:
		dev_dbg(dev, "%s: cable USB\n", __func__);
		source = CHARGER_USB;
		break;
	case CHARGER_AC:
		dev_dbg(dev, "%s: cable AC\n", __func__);
		source = CHARGER_AC;
		break;
	case CHARGER_DISCHARGE:
		dev_dbg(dev, "%s: Discharge\n", __func__);
		source = CHARGER_DISCHARGE;
		break;
	default:
		dev_err(dev, "%s: Not supported status\n", __func__);
	}
	if (source != s3c_bat_info.bat_info.charging_source) chng=1;

	s3c_bat_info.bat_info.charging_source=source;

	if (source == CHARGER_USB || source == CHARGER_AC) {
		wake_lock(&vbus_wake_lock);
	} else {
    /* give userspace some time to see the uevent and update
    * LED state or whatnot...   */
		if (gpio_get_value(gpio_ta_connected)) 
			wake_lock_timeout(&vbus_wake_lock, HZ * 5);
	}
    /* if the power source changes, all power supplies may change state */
    if (chng) {
    	printk("Charging src: %s ,param:%x\n",
    		source==CHARGER_BATTERY?"battery":(source==CHARGER_USB?"usb":(source==CHARGER_AC?"ac":
    			(source==CHARGER_DISCHARGE?"discharge":"unknown"))),
    			fg_read_param() );
		power_supply_changed(&s3c_power_supplies[CHARGER_BATTERY]);
		power_supply_changed(&s3c_power_supplies[CHARGER_USB]);
		power_supply_changed(&s3c_power_supplies[CHARGER_AC]);
    }
	dev_dbg(dev, "%s: call power_supply_changed\n", __func__);
	return source==CHARGER_UNKNOWN?-EINVAL:0;
}

static void s3c_bat_status_update(struct power_supply *bat_ps)
{
	int old_level, old_temp, old_is_full;
	dev_dbg(dev, "%s ++\n", __func__);

	if(!s3c_battery_initial)
		return;

	mutex_lock(&work_lock);
	old_temp = s3c_bat_info.bat_info.batt_temp;
	old_level = s3c_bat_info.bat_info.level; 
	old_is_full = s3c_bat_info.bat_info.batt_is_full;
	s3c_bat_info.bat_info.batt_temp = s3c_get_bat_temp(bat_ps);

	s3c_bat_info.bat_info.level = s3c_get_bat_level(bat_ps);
	if (!s3c_bat_info.bat_info.charging_enabled &&
			!s3c_bat_info.bat_info.batt_is_full) {
		if (s3c_bat_info.bat_info.level > old_level)
			s3c_bat_info.bat_info.level = old_level;
	}
	s3c_bat_info.bat_info.batt_vol = s3c_get_bat_vol(bat_ps);

#if (defined __TEST_MODE_INTERFACE__ && defined __BATTERY_V_F__)
	if (s3c_bat_info.bat_info.batt_test_mode == 1)
		s3c_get_v_f_adc();
#endif /* __TEST_MODE_INTERFACE__ && __BATTERY_V_F__ */

	if (old_level != s3c_bat_info.bat_info.level 
			|| old_temp != s3c_bat_info.bat_info.batt_temp
			|| old_is_full != s3c_bat_info.bat_info.batt_is_full
			|| force_update) {
		force_update = 0;
		power_supply_changed(bat_ps);
		dev_dbg(dev, "%s: call power_supply_changed\n", __func__);
	}

	mutex_unlock(&work_lock);
	dev_dbg(dev, "%s --\n", __func__);
}

static charger_type_t b_status = 0;

#ifdef __CHECK_BATTERY_V_F__
static unsigned int s3c_bat_check_v_f(void)
{
	unsigned int rc = 0;
	int adc = 0;
	
	adc = s3c_bat_get_adc_data(S3C_ADC_V_F);

	dev_info(dev, "%s: V_F ADC = %d\n", __func__, adc);

	if (adc <= BATT_VF_MAX && adc >= BATT_VF_MIN) {
		if (s3c_get_bat_health() == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
			s3c_set_bat_health(POWER_SUPPLY_HEALTH_GOOD);
			s3c_set_chg_en(ENABLE);
			b_status=0;
		}	
		rc = 1;
	} else {
#ifndef PHONE_B7610
		dev_info(dev, "%s: Unauthorized battery!\n", __func__);
		s3c_set_bat_health(POWER_SUPPLY_HEALTH_UNSPEC_FAILURE);
		rc = 0;
#endif
	}
	return rc;
}
#endif /* __CHECK_BATTERY_V_F__ */


static void s3c_cable_check_status(void)
{
	charger_type_t ostatus=b_status;

	mutex_lock(&work_lock);

	if (!gpio_get_value(gpio_ta_connected)) {
		if (get_usb_power_state())
			b_status = CHARGER_USB;
		else
			b_status = CHARGER_AC;

		if (s3c_get_bat_health() != POWER_SUPPLY_HEALTH_GOOD) {
			dev_info(dev, "%s: Unhealth battery state!\n", __func__);
#ifndef PHONE_B7610
			s3c_set_chg_en(DISABLE);
#endif
		} else 
			s3c_set_chg_en(ENABLE);

		dev_dbg(dev, "%s: status : %s\n", __func__, 
				(b_status == CHARGER_USB) ? "USB" : "AC");
	} else {
		u32 health = s3c_get_bat_health();

		b_status = CHARGER_BATTERY;
		s3c_set_chg_en(DISABLE);

		if (health == POWER_SUPPLY_HEALTH_OVERHEAT ||
				health == POWER_SUPPLY_HEALTH_COLD) {
			s3c_set_bat_health(POWER_SUPPLY_HEALTH_GOOD);
		}
	}
	dev_dbg(dev, "%s: gpio_chg_en %s\n", __func__, 
			gpio_get_value_ex(gpio_chg_en)?"disabled":"enabled");
	if (ostatus!=b_status)
		s3c_cable_status_update(b_status);
	mutex_unlock(&work_lock);
}

static void s3c_bat_work(struct work_struct *work)
{
	dev_dbg(dev, "%s\n", __func__);

	s3c_bat_status_update(
			&s3c_power_supplies[CHARGER_BATTERY]);
}

static void s3c_cable_work(struct work_struct *work)
{
	dev_dbg(dev, "%s\n", __func__);
#ifdef __CHECK_BATTERY_V_F__
		s3c_bat_check_v_f();
#endif /* __CHECK_BATTERY_V_F__ */
	s3c_cable_check_status();
}

#ifdef COMPENSATE_BOOTING
static void s3c_compensate_boot_work(struct work_struct *work)
{
	dev_info(dev, "%s ++\n", __func__);
	msleep(50000); /* 50sec */
	s3c_bat_set_compesation(0, OFFSET_BOOTING, COMPENSATE_BOOTING);
	dev_info(dev, "%s --\n", __func__);
}
#endif /* COMPENSATE_BOOTING */

#ifdef CONFIG_PM
static int s3c_bat_suspend(struct platform_device *pdev, 
		pm_message_t state)
{
	dev_info(dev, "%s\n", __func__);

	if (s3c_bat_info.polling)
		del_timer_sync(&polling_timer);

	flush_scheduled_work();
	disable_irq(IRQ_TA_CONNECTED_N);
	disable_irq(IRQ_TA_CHG_N);
	return 0;
}

static int s3c_bat_resume(struct platform_device *pdev)
{
	dev_info(dev, "%s\n", __func__);
//	wake_lock(&vbus_wake_lock);
	enable_irq(IRQ_TA_CONNECTED_N);
	enable_irq(IRQ_TA_CHG_N);
	schedule_work(&bat_work);
	schedule_work(&cable_work);

	if (s3c_bat_info.polling)
		mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
	return 0;
}
#else
#define s3c_bat_suspend NULL
#define s3c_bat_resume NULL
#endif /* CONFIG_PM */

static void polling_timer_func(unsigned long unused)
{
	dev_dbg(dev, "%s\n", __func__);
	schedule_work(&bat_work);

	mod_timer(&polling_timer,
		  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
}

static void cable_timer_func(unsigned long unused)
{
	dev_info(dev, "%s : intr cnt = %d\n", __func__, cable_intr_cnt);
	cable_intr_cnt = 0;
	schedule_work(&cable_work);
}

static irqreturn_t s3c_cable_changed_isr(int irq, void *power_supply)
{
	dev_dbg(dev, "%s: irq=0x%x, gpio_ta_connected=%x\n", __func__, irq,
			gpio_get_value(gpio_ta_connected));

	if (!s3c_battery_initial)
		return IRQ_HANDLED;

	s3c_bat_info.bat_info.batt_is_full = 0;
	cable_intr_cnt++;
	if (timer_pending(&cable_timer))
		del_timer(&cable_timer);

	cable_timer.expires = jiffies + msecs_to_jiffies(50);
	add_timer(&cable_timer);

	/*
	 * Wait a bit before reading ac/usb line status and setting charger,
	 * because ac/usb status readings may lag from irq.
	 */
	if (s3c_bat_info.polling)
		mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));

	return IRQ_HANDLED;
}

static irqreturn_t s3c_cable_charging_isr(int irq, void *power_supply)
{
	int chg_ing = gpio_get_value(gpio_chg_ing);
	dev_dbg(dev, "%s: irq=0x%x, gpio_chg_ing=%d\n", __func__, irq, chg_ing);

	if (!s3c_battery_initial)
		return IRQ_HANDLED;
#ifndef __DISABLE_CHG_ING_INTR__
	if (chg_ing && !gpio_get_value(gpio_ta_connected) &&
			s3c_bat_info.bat_info.charging_enabled &&
			s3c_get_bat_health() == POWER_SUPPLY_HEALTH_GOOD) {
		s3c_set_chg_en(DISABLE);
		s3c_bat_info.bat_info.batt_is_full = 1;
		force_update = 1;
		full_charge_flag = 1;
	}

	schedule_work(&bat_work);
	/*
	 * Wait a bit before reading ac/usb line status and setting charger,
	 * because ac/usb status readings may lag from irq.
	 */
	if (s3c_bat_info.polling)
		mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
#endif /* __DISABLE_CHG_ING_INTR__ */

	return IRQ_HANDLED;
}

static int __devinit s3c_bat_probe(struct platform_device *pdev)
{
	int i;
	int ret = 0;

	dev = &pdev->dev;
	dev_info(dev, "%s\n", __func__);

	s3c_bat_info.present = 1;
	s3c_bat_info.polling = 1;
	s3c_bat_info.polling_interval = POLLING_INTERVAL;
	s3c_bat_info.device_state = 0;

	s3c_bat_info.bat_info.batt_vol_adc_aver = 0;
#ifdef __TEST_MODE_INTERFACE__
	s3c_bat_info.bat_info.batt_vol_aver = 0;
	s3c_bat_info.bat_info.batt_temp_aver = 0;
	s3c_bat_info.bat_info.batt_temp_adc_aver = 0;
	s3c_bat_info.bat_info.batt_v_f_adc = 0;

	s3c_bat_info.bat_info.batt_test_mode = 0;
 	s3c_power_supplies_test = s3c_power_supplies;
#endif /* __TEST_MODE_INTERFACE__ */
	s3c_bat_info.bat_info.batt_id = 0;
	s3c_bat_info.bat_info.batt_vol = 0;
	s3c_bat_info.bat_info.batt_vol_adc = 0;
	s3c_bat_info.bat_info.batt_vol_adc_cal = 0;
	s3c_bat_info.bat_info.batt_temp = 0;
	s3c_bat_info.bat_info.batt_temp_adc = 0;
	s3c_bat_info.bat_info.batt_temp_adc_cal = 0;
	s3c_bat_info.bat_info.batt_current = 0;
	s3c_bat_info.bat_info.level = 100;
	s3c_bat_info.bat_info.charging_source = CHARGER_BATTERY;
	s3c_bat_info.bat_info.charging_enabled = 0;
	s3c_bat_info.bat_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
	s3c_bat_info.max_soc_value=96;
	memset(adc_sample, 0x00, sizeof adc_sample);

	batt_max = BATT_CAL + BATT_MAXIMUM;
	batt_full = BATT_CAL + BATT_FULL;
	batt_safe_rech = BATT_CAL + BATT_SAFE_RECHARGE;
	batt_almost = BATT_CAL + BATT_ALMOST_FULL;
	batt_high = BATT_CAL + BATT_HIGH;
	batt_medium = BATT_CAL + BATT_MED;
	batt_low = BATT_CAL + BATT_LOW;
	batt_critical = BATT_CAL + BATT_CRITICAL;
	batt_min = BATT_CAL + BATT_MINIMUM;
	batt_off = BATT_CAL + BATT_OFF;

	INIT_WORK(&bat_work, s3c_bat_work);
	INIT_WORK(&cable_work, s3c_cable_work);

#ifdef COMPENSATE_BOOTING
	INIT_WORK(&compensate_boot_work, s3c_compensate_boot_work);
	batt_drv_wqueue = create_singlethread_workqueue("batt_drv_workqueue");
#endif /* COMPENSATE_BOOTING */

	/* init power supplier framework */
	for (i = 0; i < ARRAY_SIZE(s3c_power_supplies); i++) {
		ret = power_supply_register(&pdev->dev, 
				&s3c_power_supplies[i]);
		if (ret) {
			dev_err(dev, "Failed to register"
					"power supply %d,%d\n", i, ret);
			goto __end__;
		}
	}

	/* create sec detail attributes */
	s3c_bat_create_attrs(s3c_power_supplies[CHARGER_BATTERY].dev);

#ifdef __TEST_DEVICE_DRIVER__
	s3c_test_create_attrs(s3c_power_supplies[CHARGER_AC].dev);
#endif /* __TEST_DEVICE_DRIVER__ */

	/* Request IRQ */ 
	set_irq_type(IRQ_TA_CONNECTED_N, IRQ_TYPE_EDGE_BOTH);
	ret = request_irq(IRQ_TA_CONNECTED_N, s3c_cable_changed_isr,
			  IRQF_DISABLED,
			  DRIVER_NAME,
			  &s3c_power_supplies[CHARGER_BATTERY]);
	if(ret)
		goto __end__;

	set_irq_type(IRQ_TA_CHG_N, IRQ_TYPE_EDGE_BOTH);
	ret = request_irq(IRQ_TA_CHG_N, s3c_cable_charging_isr,
			  IRQF_DISABLED,
			  DRIVER_NAME,
			  &s3c_power_supplies[CHARGER_BATTERY]);

	if (ret)
		goto __ta_connected_irq_failed__;

	if (s3c_bat_info.polling) {
		dev_dbg(dev, "%s: will poll for status\n", 
				__func__);
		setup_timer(&polling_timer, polling_timer_func, 0);
		mod_timer(&polling_timer,
			  jiffies + msecs_to_jiffies(s3c_bat_info.polling_interval));
	}

	setup_timer(&cable_timer, cable_timer_func, 0);

	s3c_battery_initial = 1;
	force_update = 0;
	full_charge_flag = 0;

	s3c_bat_status_update(
			&s3c_power_supplies[CHARGER_BATTERY]);

#ifdef __CHECK_BATTERY_V_F__
		s3c_bat_check_v_f();
#endif /* __CHECK_BATTERY_V_F__ */
	s3c_cable_check_status();
__end__:
	return ret;
__ta_connected_irq_failed__:
	free_irq(IRQ_TA_CONNECTED_N, 
			&s3c_power_supplies[CHARGER_BATTERY]);
	return ret;
}

static int __devexit s3c_bat_remove(struct platform_device *pdev)
{
	int i;
	dev_info(dev, "%s\n", __func__);

	if (s3c_bat_info.polling)
		del_timer_sync(&polling_timer);

	free_irq(IRQ_TA_CONNECTED_N, 
			&s3c_power_supplies[CHARGER_BATTERY]);
	free_irq(IRQ_TA_CHG_N, &s3c_power_supplies[CHARGER_BATTERY]);

	for (i = 0; i < ARRAY_SIZE(s3c_power_supplies); i++) {
		power_supply_unregister(&s3c_power_supplies[i]);
	}
 
	return 0;
}

static struct platform_driver s3c_bat_driver = {
	.driver.name	= DRIVER_NAME,
	.driver.owner	= THIS_MODULE,
	.probe		= s3c_bat_probe,
	.remove		= __devexit_p(s3c_bat_remove),
	.suspend	= s3c_bat_suspend,
	.resume		= s3c_bat_resume,
};

/* Initailize GPIO */
static void s3c_bat_init_hw(void)
{
	s3c_gpio_cfgpin(gpio_ta_connected, 
			S3C_GPIO_SFN(gpio_ta_connected_af));
	s3c_gpio_setpull(gpio_ta_connected, S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(gpio_chg_ing, 
			S3C_GPIO_SFN(gpio_chg_ing_af));
	s3c_gpio_setpull(gpio_chg_ing, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(gpio_chg_en, 
			S3C_GPIO_SFN(gpio_chg_en_af));
	s3c_gpio_setpull(gpio_chg_en, S3C_GPIO_PULL_NONE); 
	gpio_set_value_ex(gpio_chg_en, GPIO_LEVEL_HIGH);
}

static int __init s3c_bat_init(void)
{
	pr_info("%s\n", __func__);
	s3c_bat_init_hw();

	wake_lock_init(&vbus_wake_lock, WAKE_LOCK_SUSPEND, "vbus_present");
#if (defined __TEST_DEVICE_DRIVER__  || defined __ALWAYS_AWAKE_DEVICE__)
	wake_lock_init(&wake_lock_for_dev, WAKE_LOCK_SUSPEND, "wake_lock_dev");
#endif /* __TEST_DEVICE_DRIVER__ || __ALWAYS_AWAKE_DEVICE__ */

	if (i2c_add_driver(&fg_i2c_driver))
		pr_err("%s: Can't add fg i2c drv\n", __func__);
	return platform_driver_register(&s3c_bat_driver);
}

static void __exit s3c_bat_exit(void)
{
	pr_info("%s\n", __func__);
	i2c_del_driver(&fg_i2c_driver);
	platform_driver_unregister(&s3c_bat_driver);
}

module_init(s3c_bat_init);
module_exit(s3c_bat_exit);

MODULE_AUTHOR("Minsung Kim <ms925.kim@samsung.com>/O2Droid");
MODULE_DESCRIPTION("S3C6410 battery driver");
MODULE_LICENSE("GPL");
