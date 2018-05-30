#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define STEPS 8
#define CIRCLE 512
#define DELAY 2000

void rotate(int round, int delay, int isClockwise) {
	int i, j;
	int steps1[] = {1, 1, 0, 0, 0, 0, 0, 1};
	int steps2[] = {0, 1, 1, 1, 0, 0, 0, 0};
	int steps3[] = {0, 0, 0, 1, 1, 1, 0, 0};
	int steps4[] = {0, 0, 0, 0, 0, 1, 1, 1};

	if(isClockwise) {
		for(i=0; i<round; i++) {
			for(j=0; j<STEPS; j++) {
				gpio_set_value(PIN1, steps1[j]);
				gpio_set_value(PIN2, steps2[j]);
				gpio_set_value(PIN3, steps3[j]);
				gpio_set_value(PIN4, steps4[j]);
				udelay(delay);
			}
		}
	}
	else {
		for(i=0; i<round; i++) {
			for(j=STEPS - 1; j>=0; j--) {
				gpio_set_value(PIN1, steps1[j]);
				gpio_set_value(PIN2, steps2[j]);
				gpio_set_value(PIN3, steps3[j]);
				gpio_set_value(PIN4, steps4[j]);
				udelay(delay);
			}
		}
	}
}

void moveDegree(int degree, int delay, int isClockwise) {
	int round = (int)(CIRCLE * degree / 360);
	rotate(round, delay, isClockwise);
}

static int __init motor_init(void) {
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

	moveDegree(90, DELAY, 1);
	moveDegree(90, DELAY, 0);

	return 0;
}

static void __exit motor_exit(void) {
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);
}

module_init(motor_init);
module_exit(motor_exit);
