#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define SPEAKER 17
#define C 1911
#define D 1702
#define E 1516 
#define F 1431
#define G 1275
#define A 1136
#define B 1012
#define HC 901

MODULE_LICENSE("GPL");


static void play(int note) {
	int i = 0;
	int limit = (int)(200000 / note);
	printk("%d\n", note);

	for(i=0; i<limit; i++) {
		gpio_set_value(SPEAKER, 1);
		udelay(note);
		gpio_set_value(SPEAKER, 0);
		udelay(note);
	}
}

static int __init simple_speaker_init(void) {
	int notes[] = {C, D, E, F, G, A, B, HC};
	int i = 0;
	printk("start");

	gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER");

	for(i = 0; i < 8; i++) {
		play(notes[i]);
		mdelay(500);
	}

	return 0;
}

static void __exit simple_speaker_exit(void) {
	gpio_set_value(SPEAKER, 0);
	gpio_free(SPEAKER);
}

module_init(simple_speaker_init);
module_exit(simple_speaker_exit);
