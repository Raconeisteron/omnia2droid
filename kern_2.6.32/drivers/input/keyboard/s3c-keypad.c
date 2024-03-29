/* drivers/input/keyboard/s3c-keypad.c
 *
 * Driver core for Samsung SoC onboard UARTs.
 *
 * Kim Kyoungil, Copyright (c) 2006-2009 Samsung Electronics
 *      http://www.samsungsemi.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef PHONE_I8000
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/irq.h>

#include <linux/io.h>
#include <linux/irq.h>
#include <mach/hardware.h>
#include <asm/delay.h>
#include <asm/irq.h>

#include <plat/regs-gpio.h>
#include <plat/regs-keypad.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#ifdef CONFIG_CPU_FREQ
#include <plat/s3c64xx-dvfs.h>
#endif

#include "s3c-keypad.h"	

#include <plat/pm.h>	

#undef S3C_KEYPAD_DEBUG 
//#define S3C_KEYPAD_DEBUG 

#ifdef S3C_KEYPAD_DEBUG
#define DPRINTK(x...) printk(KERN_INFO "S3C-Keypad " x)
#define INPUT_REPORT_KEY(a,b,c) do {				\
		printk(KERN_ERR "%s:%d input_report_key(%x, %d, %d)\n", \
		       __func__, __LINE__, a, b, c);			\
		input_report_key(a,b,c);				\
	} while (0)
#else
#define DPRINTK(x...)		/* !!!! */
#define INPUT_REPORT_KEY	input_report_key
#endif

#define DEVICE_NAME "s3c-keypad"	

#define TRUE 1
#define FALSE 0


static struct timer_list keypad_timer;
//static struct timer_list powerkey_timer;
static int is_timer_on = FALSE;
static struct clk *keypad_clock;
struct resource *keypad_mem = NULL;		
struct resource *keypad_irq = NULL;	

struct resource *res = NULL;
struct input_dev *input_dev;
struct s3c_keypad *s3c_keypad;

static u32 keymask[KEYPAD_COLUMNS];
static u32 prevmask[KEYPAD_COLUMNS];

extern struct task_struct *find_task_by_pid(pid_t nr);
//static int keylock = 0;
//static unsigned int process_id = 0;

static int in_sleep = 0;


void earjack_report_key(unsigned int keycode, int value)
{
    input_report_key(s3c_keypad->dev, KEY_SEND, value);
    //printk(DCM_INP, "[KEY] earjack key %s\n", value ? "pressed" : "released");   
}
EXPORT_SYMBOL(earjack_report_key);

static int keypad_scan(void)
{

	u32 col, cval, rval;

	for (col = 0; col < KEYPAD_COLUMNS; col++) {

		cval = KEYCOL_DMASK & ~((1 << col) | (1 << (col + 8)));	

		writel(cval, key_base + S3C_KEYIFCOL);	
		udelay(KEYPAD_DELAY);

		rval =~(readl(key_base + S3C_KEYIFROW)) & ((1 << KEYPAD_ROWS) -1);
		keymask[col] = rval;
	}

	writel(KEYIFCOL_CLEAR, key_base + S3C_KEYIFCOL);

	return 0;
}

static void keypad_timer_handler(unsigned long data)
{
	u32 press_mask;
	u32 release_mask;
	u32 restart_timer = 0;
	int i, col;
	struct s3c_keypad *pdata = (struct s3c_keypad *)data;
	struct input_dev *dev = pdata->dev;

	keypad_scan();

	for (col = 0; col < KEYPAD_COLUMNS; col++) {
		press_mask = ((keymask[col] ^ prevmask[col]) & keymask[col]);
		release_mask = ((keymask[col] ^ prevmask[col]) & prevmask[col]);
/*
#ifdef CONFIG_CPU_FREQ
#if USE_PERF_LEVEL_KEYPAD
		if (press_mask || release_mask)
			set_dvfs_target_level(LEV_400MHZ);
#endif
#endif
*/
		i = col * KEYPAD_ROWS;

		while (press_mask) {
			if (press_mask & 1) {
				input_report_key(dev, pdata->keycodes[i], 1);
				DPRINTK("\nkey Pressed  : key %d map %d\n", i,
					pdata->keycodes[i]);
			}
			press_mask >>= 1;
			i++;
		}

		i = col * KEYPAD_ROWS;

		while (release_mask) {
			if (release_mask & 1) {
				input_report_key(dev, pdata->keycodes[i], 0);
				DPRINTK("\nkey Released : %d  map %d\n", i,
					pdata->keycodes[i]);

			}
			release_mask >>= 1;
			i++;
		}
		prevmask[col] = keymask[col];

		restart_timer |= keymask[col];
	}

	if (restart_timer) {
		mod_timer(&keypad_timer, jiffies + HZ / 10);
	} else {
		writel(KEYIFCON_INIT, key_base + S3C_KEYIFCON);
		is_timer_on = FALSE;
	}

}



static irqreturn_t s3c_keypad_isr(int irq, void *dev_id)
{

	/* disable keypad interrupt and schedule for keypad timer handler */
	writel(readl(key_base + S3C_KEYIFCON) & ~(INT_F_EN | INT_R_EN), key_base + S3C_KEYIFCON);

	keypad_timer.expires = jiffies;
	if (is_timer_on == FALSE) {
		add_timer(&keypad_timer);
		is_timer_on = TRUE;
	} else {
		mod_timer(&keypad_timer, keypad_timer.expires);
	}
	/*Clear the keypad interrupt status */
	writel(KEYIFSTSCLR_CLEAR, key_base + S3C_KEYIFSTSCLR);

	return IRQ_HANDLED;
}

static irqreturn_t s3c_keygpio_isr(int irq, void *dev_id)
{
	unsigned int key_status;
	static unsigned int prev_key_status= 0;
	struct s3c_keypad *pdata = (struct s3c_keypad *)dev_id;
	struct input_dev *dev = pdata->dev;

	// Beware that we may not obtain exact key up/down status at
	// this point.
	key_status = gpio_get_value(GPIO_nPOWER);

	// If ISR is called and up/down status remains the same, we
	// must have lost one and should report that first with
	// upside/down.
	if (in_sleep) {
		if (key_status == prev_key_status) {
			INPUT_REPORT_KEY(dev, 26, key_status ? 1 : 0); 
		}
		in_sleep = 0;
	}

	INPUT_REPORT_KEY(dev, 26, key_status ? 0 : 1);

	prev_key_status = key_status;
	//printk(KERN_DEBUG "s3c_keygpio_isr pwr key_status =%d\n", key_status);

	return IRQ_HANDLED;
}

static int s3c_keygpio_isr_setup(void *pdev)
{
	int ret;

	//PWR key
	s3c_gpio_cfgpin(GPIO_nPOWER, S3C_GPIO_SFN(GPIO_nPOWER_AF));
	s3c_gpio_setpull(GPIO_nPOWER, S3C_GPIO_PULL_NONE);

	set_irq_type(IRQ_EINT(5), IRQ_TYPE_EDGE_BOTH);

	// stk.lim: Add IRQF_DISABLED to eliminate any possible race
	// regarding key status
	ret = request_irq(IRQ_EINT(5), s3c_keygpio_isr, IRQF_DISABLED
			  | IRQF_SAMPLE_RANDOM, "key gpio", (void *)pdev);

	if (ret) {
		printk("request_irq failed (IRQ_KEYPAD (gpio)) !!!\n");
		ret = -EIO;
	}

	return ret;

}

static ssize_t keyshort_test(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	int count;

	if (!gpio_get_value(GPIO_KBR0) || !gpio_get_value(GPIO_KBR1)
	    || !gpio_get_value(GPIO_KBR2) || !gpio_get_value(GPIO_nPOWER)) {
		count = sprintf(buf, "PRESS\n");
		//printk("keyshort_test: PRESS\n");
	} else {
		count = sprintf(buf, "RELEASE\n");
		//printk("keyshort_test: RELEASE\n");
	}

	return count;
}

static DEVICE_ATTR(key_pressed, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH,
		   keyshort_test, NULL);

static int __init s3c_keypad_probe(struct platform_device *pdev)
{
	struct resource *res, *keypad_mem, *keypad_irq;
	struct input_dev *input_dev;
	struct s3c_keypad *s3c_keypad;
	int ret, size;
	int key, code;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource specified\n");
		return -ENOENT;
	}

	size = (res->end - res->start) + 1;

	keypad_mem = request_mem_region(res->start, size, pdev->name);
	if (keypad_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_req;
	}

	key_base = ioremap(res->start, size);
	if (key_base == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		ret = -ENOMEM;
		goto err_map;
	}

	keypad_clock = clk_get(&pdev->dev, "keypad");
	if (IS_ERR(keypad_clock)) {
		dev_err(&pdev->dev, "failed to find keypad clock source\n");
		ret = PTR_ERR(keypad_clock);
		goto err_clk;
	}

	clk_enable(keypad_clock);

	s3c_keypad = kzalloc(sizeof(struct s3c_keypad), GFP_KERNEL);
	input_dev = input_allocate_device();

	if (!s3c_keypad || !input_dev) {
		ret = -ENOMEM;
		goto err_alloc;
	}



	platform_set_drvdata(pdev, s3c_keypad);
	s3c_keypad->dev = input_dev;

	writel(KEYIFCON_INIT, key_base + S3C_KEYIFCON);
	writel(KEYIFFC_DIV, key_base + S3C_KEYIFFC);

	/* Set GPIO Port for keypad mode and pull-up disable */
	s3c_setup_keypad_cfg_gpio(KEYPAD_ROWS, KEYPAD_COLUMNS);




	writel(KEYIFCOL_CLEAR, key_base + S3C_KEYIFCOL);

	/* create and register the input driver */
	set_bit(EV_KEY, input_dev->evbit);
	/*Commenting the generation of repeat events */
	//set_bit(EV_REP, input_dev->evbit);
	s3c_keypad->nr_rows = KEYPAD_ROWS;
	s3c_keypad->no_cols = KEYPAD_COLUMNS;
	s3c_keypad->total_keys = MAX_KEYPAD_NR;

	for (key = 0; key < s3c_keypad->total_keys; key++) {
		code = s3c_keypad->keycodes[key] = keypad_keycode[key];
		if (code <= 0)
			continue;
		set_bit(code & KEY_MAX, input_dev->keybit);
	}

	printk("%s, keypad row number is %d, column is %d",__FUNCTION__, s3c_keypad->nr_rows, s3c_keypad->no_cols);

	set_bit(26 & KEY_MAX, input_dev->keybit);

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "s3c-keypad/input0";

	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	input_dev->keycode = keypad_keycode;

	keypad_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	ret = input_register_device(input_dev);
	if (ret) {
		printk("Unable to register s3c-keypad input device!!!\n");
		goto out;
	}

	/* Scan timer init */
	init_timer(&keypad_timer);
	keypad_timer.function = keypad_timer_handler;
	keypad_timer.data = (unsigned long)s3c_keypad;

	/* For IRQ_KEYPAD */
	if (keypad_irq == NULL) {
		dev_err(&pdev->dev, "no irq resource specified\n");
		ret = -ENOENT;
		goto err_irq;
	}

	ret = request_irq(IRQ_KEYPAD, s3c_keypad_isr, IRQF_SAMPLE_RANDOM, //PREVIOUS OLD ONE
		DEVICE_NAME, (void *) pdev);
	if (ret) {
		printk("request_irq failed (IRQ_KEYPAD) !!!\n");
		ret = -EIO;
		goto err_irq;
	}
	s3c_keygpio_isr_setup((void *)s3c_keypad);
	printk(DEVICE_NAME " Initialized\n");

	if (device_create_file(&pdev->dev, &dev_attr_key_pressed) < 0) {
		printk("%s s3c_keypad_probe\n", __FUNCTION__);
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_key_pressed.attr.name);
	}

	return 0;

 out:
	free_irq(keypad_irq->start, input_dev);
	free_irq(keypad_irq->end, input_dev);

 err_irq:
	input_free_device(input_dev);
	kfree(s3c_keypad);

 err_alloc:
	clk_disable(keypad_clock);
	clk_put(keypad_clock);

 err_clk:
	iounmap(key_base);

 err_map:
	release_resource(keypad_mem);
	kfree(keypad_mem);

 err_req:
	return ret;
}


static int s3c_keypad_remove(struct platform_device *pdev)
{
	struct input_dev *input_dev = platform_get_drvdata(pdev);
	writel(KEYIFCON_CLEAR, key_base + S3C_KEYIFCON);

	if (keypad_clock) {
		clk_disable(keypad_clock);
		clk_put(keypad_clock);
		keypad_clock = NULL;
	}

	input_unregister_device(input_dev);
	iounmap(key_base);
	kfree(pdev->dev.platform_data);
	free_irq(IRQ_KEYPAD, (void *)pdev);

	del_timer(&keypad_timer);
	printk(DEVICE_NAME " Removed.\n");
	return 0;
}

#ifdef CONFIG_PM


static struct sleep_save s3c_keypad_save[] = {
	SAVE_ITEM(KEYPAD_ROW_GPIOCON),
	SAVE_ITEM(KEYPAD_COL_GPIOCON),
	SAVE_ITEM(KEYPAD_ROW_GPIOPUD),
	SAVE_ITEM(KEYPAD_COL_GPIOPUD),
};

static unsigned int keyifcon, keyiffc;
static int s3c_keypad_suspend(struct platform_device *dev, pm_message_t state)
{
	keyifcon = readl(key_base + S3C_KEYIFCON);
	keyiffc = readl(key_base + S3C_KEYIFFC);

	s3c6410_pm_do_save(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));

	//writel(~(0xfffffff), KEYPAD_ROW_GPIOCON);
	//writel(~(0xfffffff), KEYPAD_COL_GPIOCON);

	disable_irq(IRQ_KEYPAD);

	clk_disable(keypad_clock);

	in_sleep = 1;

	return 0;
}

static int s3c_keypad_resume(struct platform_device *dev)
{
//	struct s3c_keypad *s3c_keypad =
//	    (struct s3c_keypad *)platform_get_drvdata(dev);
//	struct input_dev *iDev = s3c_keypad->dev;
//	unsigned int key_temp_data = 0;

	printk(KERN_DEBUG "++++ %s\n", __FUNCTION__);

	clk_enable(keypad_clock);

	writel(KEYIFCON_INIT, key_base + S3C_KEYIFCON);
	writel(keyiffc, key_base + S3C_KEYIFFC);
	writel(KEYIFCOL_CLEAR, key_base + S3C_KEYIFCOL);

	s3c6410_pm_do_restore(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));

	enable_irq(IRQ_KEYPAD);
	printk(KERN_DEBUG "---- %s\n", __FUNCTION__);
	return 0;
}
#else
#define s3c_keypad_suspend NULL
#define s3c_keypad_resume  NULL
#endif				/* CONFIG_PM */

static struct platform_driver s3c_keypad_driver = {
	.probe		= s3c_keypad_probe,
	.remove		= s3c_keypad_remove,
	.suspend	= s3c_keypad_suspend,
	.resume		= s3c_keypad_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= DEVICE_NAME,
	},
};

static int __init s3c_keypad_init(void)
{
	int ret;

	ret = platform_driver_register(&s3c_keypad_driver);
	
	if(!ret)
	   printk(KERN_INFO "S3C Keypad Driver\n");

	return ret;
}

static void __exit s3c_keypad_exit(void)
{
	platform_driver_unregister(&s3c_keypad_driver);
}

module_init(s3c_keypad_init);
module_exit(s3c_keypad_exit);
#endif
#ifdef PHONE_B7610
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>

#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/delay.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <plat/regs-gpio.h>
#include <plat/regs-keypad.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <plat/gpio-cfg.h>
#include "s3c-keypad.h"
#include "s3c-keypad-board.h"

#ifdef CONFIG_CPU_FREQ
#include <plat/s3c64xx-dvfs.h>
#endif

#ifdef CONFIG_KERNEL_DEBUG_SEC
#include <linux/kernel_sec_common.h>
#endif
typedef enum
{
	UPLOAD_CAUSE_INIT           = 0x00000000,

}kernel_sec_upload_cause_type;

#undef S3C_KEYPAD_DEBUG
//#define S3C_KEYPAD_DEBUG

#ifdef S3C_KEYPAD_DEBUG
#define DPRINTK(x...) printk("S3C-Keypad " x)
#else
#define DPRINTK(x...)		/* !!!! */
#endif

#define DEVICE_NAME "s3c-keypad"

#define TRUE 1
#define FALSE 0

#define FIRST_SCAN_INTERVAL    	(1)
#define SCAN_INTERVAL    	(HZ/50)
#if defined(CONFIG_MOUSE_OPTJOY)
extern void set_lock_oj_event(int num);
#endif
extern struct class *sec_class;
struct device *kpd_dev;
#if defined(CONFIG_MACH_MAX)
static int keypad_wakeup = 1; // default any key
#else
static int keypad_wakeup = 0;
#endif

extern int extra_eint0pend;
extern unsigned int extra_wakeup_stat;

struct input_dev *fake_slide_dev;
static int power_key_pressed = 0;

static ssize_t talk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "[Keypad] keypad wakeup : %s.\n", (keypad_wakeup?"ENABLE":"DISABLE"));
	return sprintf(buf, "%s", buf);
}

static ssize_t talk_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	if(strncmp(buf, "0", 1) == 0 ) {
		keypad_wakeup = 0;
		printk("[Keypad] keypad wakeup disable.\n");
	}
	else if(strncmp(buf, "1", 1) == 0) {
		keypad_wakeup = 1;
		printk("[Keypad] keypad wakeup enable.\n");
	}
	return size;
}

static ssize_t slide_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	if (strncmp(buf, "p", 1) == 0 || strncmp(buf, "P", 1) == 0) {
		input_report_switch(fake_slide_dev, SW_LID, 1);
		input_sync(fake_slide_dev);
		printk("[Keypad] fake slide event portrait.\n");
	} else if (strncmp(buf, "l", 1) == 0 || strncmp(buf, "L", 1) == 0) {
		input_report_switch(fake_slide_dev, SW_LID, 0);
		input_sync(fake_slide_dev);
		printk("[Keypad] fake slide event landscape.\n");
	} else if (strncmp(buf, "e", 1) == 0 || strncmp(buf, "E", 1) == 0) {
		input_report_key(fake_slide_dev, KEYCODE_ENDCALL, 1);
	   	input_report_key(fake_slide_dev, KEYCODE_ENDCALL, 0);
		printk("[Keypad] fake slide event (END key).\n");
	}

	return size;
}

static ssize_t keyshort_test(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count, i;
	int mask=0;
	u32 cval=0,rval=0;

	if( !gpio_get_value(S3C64XX_GPN(5)) ) {
		mask |= 0x1;
	}

	for (i=0; i<KEYPAD_COLUMNS; i++) {
		cval = KEYCOL_DMASK & ~((1 << i) | (1 << (i+ 8))); // clear that column number and
		writel(cval, key_base+S3C_KEYIFCOL); // make that Normal output.
								   // others shuld be High-Z output.
		udelay(KEYPAD_DELAY);
		rval = ~(readl(key_base+S3C_KEYIFROW)) & ((1<<KEYPAD_ROWS)-1) ;
		writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);
		if(rval) {
			mask |=0x100;
		}
	}

	if(mask) {
		count = sprintf(buf, "PRESS\n");
	}
	else {
		count = sprintf(buf, "RELEASE\n");
	}
	return count;
}

static DEVICE_ATTR(talk, S_IRUGO | S_IWUGO, talk_show, talk_store);
static DEVICE_ATTR(slide, S_IRUGO | S_IWUGO, NULL, slide_store);
static DEVICE_ATTR(key_pressed, S_IRUGO | S_IWUGO, keyshort_test, NULL);

static struct timer_list keypad_timer;
static struct clk *keypad_clock;
static u32 prevmask_low = 0, prevmask_high = 0;

static struct timer_list gpiokey_timer;
static int gpiokey_cnt = 0;

static int keypad_scan(u32 *keymask_low, u32 *keymask_high)
{
	u32 i,cval,rval;

	for (i=0; i<KEYPAD_COLUMNS; i++) {
		cval = KEYCOL_DMASK & ~((1 << i) | (1 << (i+ 8)));   // clear that column number and
		writel(cval, key_base+S3C_KEYIFCOL);               // make that Normal output.
								   // others shuld be High-Z output.

		udelay(KEYPAD_DELAY);

		rval = ~(readl(key_base+S3C_KEYIFROW)) & ((1<<KEYPAD_ROWS)-1) ;

#if (KEYPAD_COLUMNS>4)
		if (i < 4)
			*keymask_low |= (rval << (i * 8));
		else
			*keymask_high |= (rval << ((i-4) * 8));
#else
		*keymask_low |= (rval << (i * 8));
#endif
	}

	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

	return 0;
}


static void process_input_report (struct s3c_keypad *s3c_keypad, u32 prevmask, u32 keymask, u32 index)
{
	struct input_dev              *dev = s3c_keypad->dev;
	int i=0;
	u32 press_mask = ((keymask ^ prevmask) & keymask);
	u32 release_mask = ((keymask ^ prevmask) & prevmask);

	i = 0;
	while (press_mask) {
		if (press_mask & 1) {
			input_report_key(dev, GET_KEYCODE(i+index),1);
			DPRINTK(": Pressed (index: %d, Keycode: %d) line : %d\n", i+index, GET_KEYCODE(i+index),__LINE__);
			DPRINTK(": Pressed (index: %d, Keycode: %d)\n", i+index, GET_KEYCODE(i+index),__LINE__);
#if !defined(CONFIG_MACH_MAX)&&!defined(CONFIG_MACH_VITAL)
#if defined(CONFIG_MOUSE_OPTJOY)
			if(i+index==40)
				set_lock_oj_event(1);
#endif
#endif
		}
		press_mask >>= 1;
		i++;
	}

	i = 0;
	while (release_mask) {
		if (release_mask & 1) {
			input_report_key(dev,GET_KEYCODE(i+index),0);
			DPRINTK(": Released (index: %d, Keycode: %d) line : %d\n", i+index, GET_KEYCODE(i+index),__LINE__);
			DPRINTK(": Released (index: %d, Keycode: %d)\n", i+index, GET_KEYCODE(i+index),__LINE__);
#if !defined(CONFIG_MACH_MAX)&&!defined(CONFIG_MACH_VITAL)
#if defined(CONFIG_MOUSE_OPTJOY)
			if(i+index==40)
				set_lock_oj_event(0);
#endif
#endif
		}
		release_mask >>= 1;
		i++;
	}
}

static inline void process_special_key (struct s3c_keypad *s3c_keypad, u32 keymask_low, u32 keymask_high)
{
	struct input_dev              *dev = s3c_keypad->dev;
	struct s3c_keypad_extra       *extra = s3c_keypad->extra;
	struct s3c_keypad_special_key *special_key = extra->special_key;
	static int prev_bitmask = 0;
	int i;

	for (i=0; i<extra->special_key_num; i++, special_key+=1)
	{
	    if (keymask_low == special_key->mask_low
		    && keymask_high == special_key->mask_high
		    && !(prev_bitmask & (1<<i))) {
        	input_report_key(dev, special_key->keycode, 1);
			DPRINTK(": Pressed (Keycode: %d, SPECIAL KEY) line : %d\n", special_key->keycode, __LINE__);
			DPRINTK(": Pressed (Keycode: %d, SPECIAL KEY)\n", special_key->keycode,__LINE__);
			prev_bitmask |= (1<<i);
			continue;
		}
		if ((prev_bitmask & (1<<i))
 		    && keymask_low == 0
	    	    && keymask_high == 0)
		{
	       	input_report_key(dev, special_key->keycode, 0);
			DPRINTK(": Released (Keycode: %d, SPECIAL KEY) line : %d\n", special_key->keycode, __LINE__);
			DPRINTK(": Released (Keycode: %d, SPECIAL KEY)\n", special_key->keycode,__LINE__);
			prev_bitmask ^= (1<<i);
		}
	}
}

#if 0/*defined (CONFIG_MACH_VINSQ) || defined (CONFIG_MACH_VITAL)*/
#define DEVELOPE_RELEASE
#endif
static void keypad_timer_handler(unsigned long data)
{
	struct s3c_keypad *s3c_keypad = (struct s3c_keypad *)data;
	u32 keymask_low = 0, keymask_high = 0;

	keypad_scan(&keymask_low, &keymask_high);

	process_special_key(s3c_keypad, keymask_low, keymask_high);

#ifdef CONFIG_KERNEL_DEBUG_SEC
//	DPRINTK("%s L(0x%x) H(0x%x)\n", __func__, keymask_low, keymask_high);
#if defined (CONFIG_MACH_MAX)
	if(keymask_low == 0x10202) // vol down key + ok key + camera Key
#elif defined (CONFIG_MACH_VITAL)
	if(keymask_high == 0x1000100) // vol down key (0x1000000) + voice key (0x100)
#else /* CONFIG_MACH_VINSQ */
	if((keymask_low == 0x1) && (keymask_high == 0x100)) // vol down key + ok key
#endif
	{
#if defined (DEVELOPE_RELEASE)
		if (kernel_sec_viraddr_wdt_reset_reg)
		{
			kernel_sec_save_final_context(); // Save the final context.
#ifdef CONFIG_KERNEL_DEBUG_SEC
			kernel_sec_set_upload_cause(UPLOAD_CAUSE_FORCED_UPLOAD);
			kernel_sec_hw_reset(FALSE); // Reboot.
#endif
		}
#endif
	}
	else
#endif
#if defined (CONFIG_MACH_MAX) // Requested by QA
// CAM_FULL : keymask_low = 0x30000
// VOLUME_UP : keymask_low = 0x100
// VOLUME_DOWN : keymask_low = 0x200
	if((keymask_low == 0x30300)
	&& power_key_pressed)
#elif defined (CONFIG_MACH_VITAL)
// CAM_FULL : keymask_low = 0x1010000
// VOLUME_UP : keymask_low = 0x1
	if((keymask_low == 0x1010001)
	&& power_key_pressed)
#else /* CONFIG_MACH_VINSQ */
// CAM_FULL : keymask_low = 0x1010000
// VOLUME_UP : keymask_high = 0x1000000
	if((keymask_low == 0x1010000)
	&& (keymask_high == 0x1000000)
	&& power_key_pressed)
#endif
	{
#ifdef CONFIG_KERNEL_DEBUG_SEC
		kernel_sec_set_upload_cause(UPLOAD_CAUSE_INIT);
		kernel_sec_hw_reset(FALSE); 	 // Reboot.
#endif
	}

	if (keymask_low != prevmask_low) {
		process_input_report (s3c_keypad, prevmask_low, keymask_low, 0);
		prevmask_low = keymask_low;
	}
#if (KEYPAD_COLUMNS>4)
	if (keymask_high != prevmask_high) {
		process_input_report (s3c_keypad, prevmask_high, keymask_high, 32);
		prevmask_high = keymask_high;
	}
#endif

	if (keymask_low | keymask_high) {
		mod_timer(&keypad_timer,jiffies + SCAN_INTERVAL);
	} else {
		writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	}
}

static irqreturn_t s3c_keypad_isr(int irq, void *dev_id)
{
#ifdef  CONFIG_CPU_FREQ
	set_dvfs_perf_level();
#endif
	/* disable keypad interrupt and schedule for keypad timer handler */
	writel(readl(key_base+S3C_KEYIFCON) & ~(INT_F_EN|INT_R_EN), key_base+S3C_KEYIFCON);

	keypad_timer.expires = jiffies + FIRST_SCAN_INTERVAL;
	mod_timer(&keypad_timer,keypad_timer.expires);

	/*Clear the keypad interrupt status*/
	writel(KEYIFSTSCLR_CLEAR, key_base+S3C_KEYIFSTSCLR);

	return IRQ_HANDLED;
}

static irqreturn_t slide_int_handler(int irq, void *dev_id)
{
	struct s3c_keypad       *s3c_keypad = (struct s3c_keypad *) dev_id;
	struct s3c_keypad_slide *slide      = s3c_keypad->extra->slide;
	int state;
	int test = 0;

#ifdef  CONFIG_CPU_FREQ
	set_dvfs_perf_level();
#endif
	test = gpio_get_value(slide->gpio);
	state = test ^ slide->state_upset;
	printk("[SLIDE] gpio(%d-%d) state_upset(%d) state(%d) LINE(%d)\n",
		slide->gpio, test, slide->state_upset, state, __LINE__);

#if defined(CONFIG_MACH_VINSQ) || defined(CONFIG_MACH_MAX) || defined(CONFIG_MACH_VITAL)
    input_report_switch(s3c_keypad->dev, SW_LID, !state);
#else
    input_report_switch(s3c_keypad->dev, SW_LID, state);
#endif
	input_sync(s3c_keypad->dev);

	return IRQ_HANDLED;
}

#if defined(CONFIG_MACH_VINSQ) || defined(CONFIG_MACH_MAX) || defined(CONFIG_MACH_VITAL)
static int notify_pwr_key_pressed(int pressed, int keycode)
{
#if 0
    extern int tsp_preprocess_suspend(void);
    if(pressed==0 && keycode == 249)
{
        //notify tsp
        tsp_preprocess_suspend();
        return true;
    }
#endif
    return false;
}
#endif

static void gpiokey_timer_handler(unsigned long data)
{
	struct s3c_keypad *s3c_keypad = (struct s3c_keypad *)data;
	struct input_dev           *dev = s3c_keypad->dev;
	struct s3c_keypad_extra    *extra = s3c_keypad->extra;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;
	static int state = 0;
	int state_check;
	power_key_pressed = 0;

	if(!gpiokey_cnt) {
		state = gpio_get_value(gpio_key->gpio);
		DPRINTK("[KPD_TEST] first gpio level: %d\n",  state);
		gpiokey_cnt++;
		mod_timer(&gpiokey_timer, jiffies + 2);
		return;
	}

	state_check = gpio_get_value(gpio_key->gpio);
	DPRINTK("[KPD_TEST] gpio level: %d, cnt: %d\n",  state_check, gpiokey_cnt);
	if(state != state_check) {
		DPRINTK("[KPD_TEST] unstable gpio detected..\n");
		return;
	}
	if(gpiokey_cnt < 5) {
		gpiokey_cnt++;
		mod_timer(&gpiokey_timer, jiffies + 2);
	}else {
        if(!state) {
#if defined(CONFIG_MACH_VINSQ) || defined(CONFIG_MACH_MAX) || defined(CONFIG_MACH_VITAL)
   	        notify_pwr_key_pressed(state, gpio_key->keycode);
#endif
			power_key_pressed = 1;
			input_report_key(dev, gpio_key->keycode, 1);
			DPRINTK(": Pressed (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
			DPRINTK(": Pressed (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
//       	   	printk("[KPD_DBG] Pressed (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
   	   	}
   		else  {
			input_report_key(dev, gpio_key->keycode, 0);
			DPRINTK(": Released (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
			DPRINTK(": Released (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
//   		    printk("[KPD_DBG] Released (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
      	}
	}
	return;
}

static irqreturn_t gpio_int_handler(int irq, void *dev_id)
{
	struct s3c_keypad          *s3c_keypad = (struct s3c_keypad *) dev_id;

	DPRINTK(": gpio interrupt (IRQ: %d)\n", irq);

#ifdef  CONFIG_CPU_FREQ
	set_dvfs_perf_level();
#endif

	if(timer_pending(&gpiokey_timer))
		del_timer(&gpiokey_timer);

	gpiokey_timer.expires = jiffies + 2;
	gpiokey_cnt = 0;
	add_timer(&gpiokey_timer);

	return IRQ_HANDLED;
}

static int __init s3c_keypad_probe(struct platform_device *pdev)
{
	struct resource *res, *keypad_mem, *keypad_irq = NULL;
	struct input_dev *input_dev;
	struct s3c_keypad *s3c_keypad;
	int ret, size, key;
	struct s3c_keypad_extra    	*extra = NULL;
	struct s3c_keypad_slide    	*slide = NULL;
	struct s3c_keypad_special_key    *special_key;
	struct s3c_keypad_gpio_key 	*gpio_key;
	int i;
	char * input_dev_name;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev,"no memory resource specified\n");
		return -ENOENT;
	}

	size = (res->end - res->start) + 1;

	keypad_mem = request_mem_region(res->start, size, pdev->name);
	if (keypad_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		ret = -ENOENT;
		goto err_req;
	}

	key_base = ioremap(res->start, size);
	if (key_base == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		ret = -ENOMEM;
		goto err_map;
	}

	keypad_clock = clk_get(&pdev->dev, "keypad");
	if (IS_ERR(keypad_clock)) {
		dev_err(&pdev->dev, "failed to find keypad clock source\n");
		ret = PTR_ERR(keypad_clock);
		goto err_clk;
	}

	clk_enable(keypad_clock);

	s3c_keypad = kzalloc(sizeof(struct s3c_keypad), GFP_KERNEL);
	input_dev = input_allocate_device();
	input_dev_name = (char *)kmalloc(sizeof("s3c-keypad-revxxxx"), GFP_KERNEL);

	if (!s3c_keypad || !input_dev || !input_dev_name) {
		ret = -ENOMEM;
		goto out;
	}

	platform_set_drvdata(pdev, s3c_keypad);

	DPRINTK(": system_rev 0x%04x\n", system_rev);
	for (i=0; i<sizeof(s3c_keypad_extra)/sizeof(struct s3c_keypad_extra); i++)
	{
		//if (s3c_keypad_extra[i].board_num == g_board_num) {
		if (s3c_keypad_extra[i].board_num == system_rev) {
			extra = &s3c_keypad_extra[i];
			sprintf(input_dev_name, "%s%s%04x", DEVICE_NAME, "-rev", system_rev);
			DPRINTK(": board rev 0x%04x is detected!\n", s3c_keypad_extra[i].board_num);
			break;
		}
	}

	/* Default revison */
	if(!extra)
	{
		extra = &s3c_keypad_extra[0];
#if defined(CONFIG_MACH_VINSQ) || defined(CONFIG_MACH_MAX) || defined(CONFIG_MACH_VITAL)
		sprintf(input_dev_name, "%s%s", DEVICE_NAME, "-rev0050");
#else
		sprintf(input_dev_name, "%s%s", DEVICE_NAME, "-rev0000");
#endif
		DPRINTK(": failed to detect board rev. set Default revison!\n");
	}
	DPRINTK(": input device name: %s.\n", input_dev_name);

	s3c_keypad->dev = input_dev;
	fake_slide_dev = input_dev;
	s3c_keypad->extra = extra;
	slide = extra->slide;
	special_key = extra->special_key;
	gpio_key = extra->gpio_key;

	writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	writel(KEYIFFC_DIV, key_base+S3C_KEYIFFC);

	/* Set GPIO Port for keypad mode and pull-up disable*/
	s3c_setup_keypad_cfg_gpio(KEYPAD_ROWS, KEYPAD_COLUMNS);

	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

	for(key = 0; key < 64; key++){
        	input_set_capability(input_dev, EV_KEY, key+1);
	}

	for (i=0; i<extra->special_key_num; i++ ){
        	input_set_capability(input_dev, EV_KEY, (special_key+i)->keycode);
	}

	for (i=0; i<extra->gpio_key_num; i++ ){
        	input_set_capability(input_dev, EV_KEY, (gpio_key+i)->keycode);
	}

	if (extra->slide != NULL)
   {
       input_set_capability(input_dev, EV_SW, SW_LID);
#if defined(CONFIG_MACH_VINSQ) || defined(CONFIG_MACH_MAX) || defined(CONFIG_MACH_VITAL)
       input_dev->sw[SW_LID] = 1;  //vinsq.boot
#endif
   }

	input_dev->name = input_dev_name;
	input_dev->phys = "s3c-keypad/input0";

	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0001;

	/* Scan timer init */
	init_timer(&keypad_timer);
	keypad_timer.function = keypad_timer_handler;
	keypad_timer.data = (unsigned long)s3c_keypad;

	init_timer(&gpiokey_timer);
	gpiokey_timer.function = gpiokey_timer_handler;
	gpiokey_timer.data = (unsigned long)s3c_keypad;


	/* For IRQ_KEYPAD */
	keypad_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (keypad_irq == NULL) {
		dev_err(&pdev->dev, "no irq resource specified\n");
		ret = -ENOENT;
		goto err_clk;
	}

	if (slide != NULL)
	{
		s3c_gpio_cfgpin(slide->gpio, S3C_GPIO_SFN(slide->gpio_af));
		s3c_gpio_setpull(slide->gpio, S3C_GPIO_PULL_NONE);

		set_irq_type(slide->eint, IRQ_TYPE_EDGE_BOTH);

		ret = request_irq(slide->eint, slide_int_handler, IRQF_DISABLED,
		    "s3c_keypad gpio key", (void *)s3c_keypad);
		if (ret) {
			printk(KERN_ERR "request_irq(%d) failed (IRQ for SLIDE) !!!\n", slide->eint);
			ret = -EIO;
			goto err_irq;
		}
	}

	for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
	{
		s3c_gpio_cfgpin(gpio_key->gpio, S3C_GPIO_SFN(gpio_key->gpio_af));
		s3c_gpio_setpull(gpio_key->gpio, S3C_GPIO_PULL_NONE);

		set_irq_type(gpio_key->eint, IRQ_TYPE_EDGE_BOTH);

		ret = request_irq(gpio_key->eint, gpio_int_handler, IRQF_DISABLED,
			    "s3c_keypad gpio key", (void *)s3c_keypad);
		if (ret) {
			printk(KERN_ERR "request_irq(%d) failed (IRQ for GPIO KEY) !!!\n", gpio_key->eint);
			ret = -EIO;
			goto err_irq;
		}
	}

	ret = request_irq(keypad_irq->start, s3c_keypad_isr, IRQF_SAMPLE_RANDOM,
		DEVICE_NAME, (void *) pdev);
	if (ret) {
		printk("request_irq failed (IRQ_KEYPAD) !!!\n");
		ret = -EIO;
		goto err_irq;
	}

	ret = input_register_device(input_dev);
	if (ret) {
		printk("Unable to register s3c-keypad input device!!!\n");
		goto out;
	}

	keypad_timer.expires = jiffies + (HZ/10);
	mod_timer(&keypad_timer,keypad_timer.expires);

	printk( DEVICE_NAME " Initialized\n");
	return 0;

out:
	input_free_device(input_dev);
	kfree(s3c_keypad);

err_irq:
	free_irq(keypad_irq->start, input_dev);
	free_irq(keypad_irq->end, input_dev);

	if (slide != NULL)
		free_irq(extra->slide->eint, s3c_keypad);

	gpio_key = extra->gpio_key;
	for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
		free_irq(gpio_key->eint, s3c_keypad);

err_clk:
	clk_disable(keypad_clock);
	clk_put(keypad_clock);

err_map:
	iounmap(key_base);

err_req:
	release_resource(keypad_mem);
	kfree(keypad_mem);

	return ret;
}

static int s3c_keypad_remove(struct platform_device *pdev)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);
	struct input_dev  *dev        = s3c_keypad->dev;
	struct s3c_keypad_extra *extra = s3c_keypad->extra;
	struct s3c_keypad_slide *slide = extra->slide;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;

	writel(KEYIFCON_CLEAR, key_base+S3C_KEYIFCON);

	if(keypad_clock) {
		clk_disable(keypad_clock);
		clk_put(keypad_clock);
		keypad_clock = NULL;
	}

	free_irq(IRQ_KEYPAD, (void *) s3c_keypad);

	del_timer(&keypad_timer);

	if (slide)
	        free_irq(slide->eint, (void *) s3c_keypad);

	if (gpio_key)
	{
		int i;
		for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
        		free_irq(gpio_key->eint, (void *) s3c_keypad);
	}

	input_unregister_device(dev);
	iounmap(key_base);
	kfree(s3c_keypad);

	printk(DEVICE_NAME " Removed.\n");
	return 0;
}

#ifdef CONFIG_PM
#include <plat/pm.h>

static struct sleep_save s3c_keypad_save[] = {
	SAVE_ITEM(KEYPAD_ROW_GPIOCON),
	SAVE_ITEM(KEYPAD_COL_GPIOCON),
	SAVE_ITEM(KEYPAD_ROW_GPIOPUD),
	SAVE_ITEM(KEYPAD_COL_GPIOPUD),
};

static int s3c_keypad_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);
	struct s3c_keypad_extra *extra = s3c_keypad->extra;
	struct s3c_keypad_slide *slide = extra->slide;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;

	writel(KEYIFCON_CLEAR, key_base+S3C_KEYIFCON);

	s3c6410_pm_do_save(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));

#if 1//!defined (CONFIG_MACH_MAX)
	if(!keypad_wakeup) {
			writel(~(0x0fffffff), KEYPAD_ROW_GPIOCON);
			writel(~(0xffffffff), KEYPAD_COL_GPIOCON);
	}
#endif

#if 0
	if (!extra->wakeup_by_keypad)
	{
		writel(~(0x0fffffff), KEYPAD_ROW_GPIOCON);
		writel(~(0xffffffff), KEYPAD_COL_GPIOCON);
	}
#endif

	disable_irq(IRQ_KEYPAD);

	if (slide)
		disable_irq(slide->eint);

	if (gpio_key)
	{
		int i;
		for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
        		disable_irq(gpio_key->eint);
	}

	clk_disable(keypad_clock);

	return 0;
}

static int s3c_keypad_resume(struct platform_device *pdev)
{
	struct s3c_keypad *s3c_keypad = platform_get_drvdata(pdev);
	struct s3c_keypad_extra *extra = s3c_keypad->extra;
	struct s3c_keypad_slide *slide = extra->slide;
	struct s3c_keypad_gpio_key *gpio_key = extra->gpio_key;
	struct input_dev *dev = s3c_keypad->dev;

	clk_enable(keypad_clock);

	prevmask_low = 0;
	prevmask_high = 0;
	s3c_keypad_isr (0, NULL);

	enable_irq(IRQ_KEYPAD);

#if defined (CONFIG_MACH_MAX)
       if(extra_eint0pend == 0x200000) // Headset wakeup event
       {
		input_report_key(dev, 249, 1); // Power Key press
   	 	DPRINTK(": Force Power Key press for headset\n");

		input_report_key(dev, 249, 0); // Power Key release
		DPRINTK(": Force Power Key release for headset\n");
       }

       if(keypad_wakeup)
       {
           printk("%s, extra_wakeup_stat: 0x%08x\n", __FUNCTION__, extra_wakeup_stat);

           // exception for H/W Key pressing bug
           if(extra_wakeup_stat == 0x00000010) // Keypad wakeup event
           {
    		input_report_key(dev, 249, 1); // Power Key press
       	 	DPRINTK(": Force Power Key press\n");

    		input_report_key(dev, 249, 0); // Power Key release
    		DPRINTK(": Force Power Key release\n");
           }
       }
#endif

#if !defined (CONFIG_MACH_MAX)
	printk("%s, slide (%u) (%d)\n", __func__, extra->slide, slide->eint);
#endif

	if (slide)
	{
		enable_irq(slide->eint);
		slide_int_handler (slide->eint, (void *) s3c_keypad);
	}
	printk("%s, extra_eint0pend: 0x%08x\n", __func__, extra_eint0pend);

    if(gpio_key) {
		enable_irq(gpio_key->eint);

		if(extra_eint0pend & 0x00000020)
		{
			input_report_key(dev, gpio_key->keycode, 1);
	   	 	DPRINTK(": Pressed (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
        	DPRINTK(": Pressed with Resume (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
//        	printk("[KPD_DBG] Pressed with Resume (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);

//			printk("%s, gpio_key->gpio level: %d\n", __func__, gpio_get_value(gpio_key->gpio));
 			if(gpio_get_value(gpio_key->gpio))
			{
	   			input_report_key(dev, gpio_key->keycode, 0);
   				DPRINTK(": Released (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
   				DPRINTK(": Released with Resume (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
//				printk("[KPD_DBG] Released with Resume (Keycode: %d, GPIO KEY)\n", gpio_key->keycode);
			}
		}
	}

#if 0
	if (gpio_key)
	{
		int i;
		for (i=0; i<extra->gpio_key_num; i++, gpio_key+=1)
		{
       		enable_irq(gpio_key->eint);
			gpio_int_handler (gpio_key->eint, (void *) s3c_keypad);
		}

	}
#endif


	writel(KEYIFCON_INIT, key_base+S3C_KEYIFCON);
	writel(KEYIFFC_DIV, key_base+S3C_KEYIFFC);

	s3c6410_pm_do_restore(s3c_keypad_save, ARRAY_SIZE(s3c_keypad_save));

	writel(KEYIFCOL_CLEAR, key_base+S3C_KEYIFCOL);

	return 0;
}
#else
#define s3c_keypad_suspend NULL
#define s3c_keypad_resume  NULL
#endif /* CONFIG_PM */

static struct platform_driver s3c_keypad_driver = {
	.probe		= s3c_keypad_probe,
	.remove		= s3c_keypad_remove,
	.suspend	= s3c_keypad_suspend,
	.resume		= s3c_keypad_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-keypad",
	},
};

static int __init s3c_keypad_init(void)
{
	int ret;

	kpd_dev = device_create(sec_class, NULL, 0, NULL, "keypad");
	if (IS_ERR(kpd_dev))
		pr_err("Failed to create device(keypad)!\n");
	if (device_create_file(kpd_dev, &dev_attr_talk) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_talk.attr.name);
	if (device_create_file(kpd_dev, &dev_attr_slide) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_slide.attr.name);
	if (device_create_file(kpd_dev, &dev_attr_key_pressed) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_key_pressed.attr.name);

	ret = platform_driver_register(&s3c_keypad_driver);

	if(!ret)
	   printk(KERN_INFO "S3C Keypad Driver\n");

	return ret;
}

static void __exit s3c_keypad_exit(void)
{
	platform_driver_unregister(&s3c_keypad_driver);
}

module_init(s3c_keypad_init);
module_exit(s3c_keypad_exit);
#endif
MODULE_AUTHOR("Samsung");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("KeyPad interface for Samsung S3C");
