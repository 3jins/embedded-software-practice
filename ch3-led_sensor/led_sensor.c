#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "led_sensor_dev"
#define SENSOR 17
#define LED 12


static int irq_num;
static struct timer_list led_timer;

static int simple_sensor_open(struct inode *inode, struct file* file) {
	printk("open\n");
	enable_irq(irq_num);
	return 0;
}

static int simple_sensor_release(struct inode *inode, struct file* file) {
	printk("close\n");
	disable_irq(irq_num);
	return 0;
}

/* Turn on/off the light */
static void lumos(unsigned long on) {
	printk("lumos %d\n", on);
	gpio_set_value(LED, on);
}

/* Turn off the light after 2 secs */
static void led_timer_fun(void) {
	printk("led_timer_fun\n");
	// TODO: Should avoid the duplicated timer addition
	led_timer.function = lumos;
	led_timer.data = 0;
	led_timer.expires = jiffies + (2*HZ);
	add_timer(&led_timer);
}

struct file_operations simple_sensor_fops = {
	.open = simple_sensor_open,
	.release = simple_sensor_release,
};

/* Turn on the light and set timer when detect sth moving */
static irqreturn_t simple_sensor_isr(int irq, void* dev_id) {
	printk("detect\n");
	lumos(1);
	led_timer_fun();
	return IRQ_HANDLED;
}

static dev_t dev_num;
static struct cdev *char_dev;

static int __init simple_sensor_init(void) {
	int ret;
	printk("init\n");
	
	/* Allocate character dev */
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	char_dev = cdev_alloc();
	cdev_init(char_dev, &simple_sensor_fops);
	cdev_add(char_dev, dev_num, 1);

	/* Request GPIO and interrupt handler of the sensor*/
	gpio_request_one(SENSOR, GPIOF_IN, "sensor");
	irq_num = gpio_to_irq(SENSOR);
	ret = request_irq(irq_num, simple_sensor_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
	if(ret) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	}
	else {
		disable_irq(irq_num);
	}

	/* Request GPIO of the LED */
	gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED");

	/* Initialize the timer */
	init_timer(&led_timer);

	return 0;
}

static void __exit simple_sensor_exit(void) {
	printk("exit\n");
	cdev_del(char_dev);
	unregister_chrdev_region(dev_num, 1);

	free_irq(irq_num, NULL);
	gpio_free(SENSOR);
}

module_init(simple_sensor_init);
module_exit(simple_sensor_exit);
