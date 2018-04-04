#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

#define LED1 4
#define LED2 5
#define LED3 6

static struct timer_list lumos_timer;

static void lumos(unsigned long data) {
	int leds[3] = {0, 0, 0};
	switch(lumos_timer.data) {
		case 0:
			leds[0] = 1;
			printk("1\n");
			break;
		case 1:
			leds[1] = 1;
			printk("2\n");
			break;
		case 2:
			leds[2] = 1;
			printk("3\n");
			break;
		default:
			// do nothing
			break;
	}
	gpio_set_value(LED1, leds[0]);
	gpio_set_value(LED2, leds[1]);
	gpio_set_value(LED3, leds[2]);

	lumos_timer.function = lumos;
	lumos_timer.data = (lumos_timer.data + 1) % 3;
	lumos_timer.expires = jiffies + HZ;
	add_timer(&lumos_timer);
}

static int __init ch2_mod_init(void) {
	printk("Init\n");
	init_timer(&lumos_timer);

	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	gpio_request_one(LED2, GPIOF_OUT_INIT_LOW, "LED2");
	gpio_request_one(LED3, GPIOF_OUT_INIT_LOW, "LED3");

	lumos_timer.function = lumos;
	lumos_timer.data = 0L;	// long
	lumos_timer.expires = jiffies + 2*HZ;
	add_timer(&lumos_timer);

	return 0;
}

static void __exit ch2_mod_exit(void) {
	printk("Exit\n");

	gpio_set_value(LED1, 0);
	gpio_set_value(LED2, 0);
	gpio_set_value(LED3, 0);
	gpio_free(LED1);
	gpio_free(LED2);
	gpio_free(LED3);

	del_timer(&lumos_timer);
}

module_init(ch2_mod_init);
module_exit(ch2_mod_exit);	
