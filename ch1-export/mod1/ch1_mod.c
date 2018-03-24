#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DEV_NAME "challenge_export_mod1_dev"

#define IOCTL_START_NUM 0X80
#define IOCTL_INCREASE_MY_VAR IOCTL_START_NUM+1

#define IOCTL_NUM 'z'
#define INCREASE_MY_VAR _IOWR(IOCTL_NUM, IOCTL_INCREASE_MY_VAR, unsigned long *)

MODULE_LICENSE("GPL");

static dev_t dev_num;	// a.k.a. major number
static struct cdev *char_dev;

int my_var = 0;
EXPORT_SYMBOL(my_var);

static long mod1_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch(cmd) {
		case INCREASE_MY_VAR:
			my_var++;
			printk("I'm mod1 and the value of my_var is changed to %d by ch1_app\n", my_var);
			break;
		default:
			return -1;
	}
	return 0;
}

static int mod1_open(struct inode *inode, struct file *file) {
	printk("mod1 open\n");
	printk("I'm mod1 and the value of my_var is %d\n", my_var);
	return 0;
}

static int mod1_release(struct inode *inode, struct file *file) {
	printk("mod1 release\n");
	return 0;
}

struct file_operations mod1_fops = {
	.unlocked_ioctl = mod1_ioctl,
	.open = mod1_open,
	.release = mod1_release,
};

static int __init mod1_init(void) {
	printk("1st kernel module is loaded!\n");
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	char_dev = cdev_alloc();
	cdev_init(char_dev, &mod1_fops);
	cdev_add(char_dev, dev_num, 1);
	return 0;
}

static void __exit mod1_exit(void) {
	printk("1st kernel module will be removed!\n");
	cdev_del(char_dev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(mod1_init);
module_exit(mod1_exit);
