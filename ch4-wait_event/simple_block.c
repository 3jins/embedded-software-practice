#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_block_dev"

spinlock_t my_lock;
static int my_data;
static dev_t dev_num;
static struct cdev *char_dev;

wait_queue_head_t my_wq;

static int simple_block_read(struct file *file, char *buf, size_t len, loff_t *loff) {
//	wait_event(my_wq, my_data > 0);	// non-exclusive
//	wait_event_interruptible_exclusive(my_wq, my_data > 0);	// exclusive
	int ret = wait_event_timeout(my_wq, my_data > 0, 2 * HZ);
	if(ret == 0) {
		printk("Timeout\n");
		return -1;
	}
	spin_lock(&my_lock);
	my_data--;
	spin_unlock(&my_lock);
	printk("Consume data (time remain: %d)\n", ret);
	return my_data;
}

static int simple_block_write(struct file *file, const char *buf, size_t len, loff_t *loff) {
	spin_lock(&my_lock);
	my_data++;
	spin_unlock(&my_lock);
	printk("Produce data\n");
	wake_up(&my_wq);
	return my_data;
}

static int simple_block_open(struct inode *inode, struct file *file) {
	return 0;
}

static int simple_block_release(struct inode *inode, struct file *file) {
	return 0;
}

struct file_operations simple_block_fops = {
	.read = simple_block_read, 
	.write = simple_block_write,
	.open = simple_block_open,  
	.release = simple_block_release, 
};

static int __init simple_block_init(void) {
	printk("init\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	char_dev = cdev_alloc();
	cdev_init(char_dev, &simple_block_fops);
	cdev_add(char_dev, dev_num, 1);

	spin_lock_init(&my_lock);
	init_waitqueue_head(&my_wq);

	return 0;
}

static void __init simple_block_exit(void) {
	printk("exit\n");
	cdev_del(char_dev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(simple_block_init);
module_exit(simple_block_exit);
