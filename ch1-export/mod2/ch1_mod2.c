#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DEV_NAME "challenge_export_mod2_dev"

extern int my_var;

static dev_t dev_num;	// a.k.a. major number
static struct cdev *char_dev;

static int mod2_open(struct inode *inode, struct file *file) {
	printk("mod2 open\n");
	printk("I'm mod2 and my_var of mod1 is %d\n", my_var);
	return 0;
}

static int mod2_release(struct inode *inode, struct file *file) {
	printk("mod2 release\n");
	printk("I'm mod2 and my_var of mod1 is %d\n", my_var);
	return 0;
}

struct file_operations mod2_fops = {
	.open = mod2_open,
	.release = mod2_release,
};

static int __init mod2_init(void) {
	printk("2nd kernel module is loaded!\n");
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	char_dev = cdev_alloc();
	cdev_init(char_dev, &mod2_fops);
	cdev_add(char_dev, dev_num, 1);
	return 0;
}

static void __exit mod2_exit(void) {
	printk("2nd kernel module will be removed!\n");
	cdev_del(char_dev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(mod2_init);
module_exit(mod2_exit);
