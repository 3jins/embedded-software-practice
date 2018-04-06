#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "ku_ipc.h"

MODULE_LICENSE("GPL");

static dev_t dev_num;
static struct cdev *char_dev;


int ku_msgget(int key, int msgflg) {
	printk("msgget\n");
	printk("%d %d\n", key, msgflg);

	return 0;
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch(cmd) {
		case IOCTL_MSGGET:
		{
			//struct msgget_str* get_arg = (struct msgget_str*)arg;
			//return ku_msgget(get_arg->key, get_arg->msgflg);
		}
		case IOCTL_MSGCLOSE:
		case IOCTL_MSGSND:
		case IOCTL_MSGRCV:
			break;
	}
	return 0;
}

static int ku_ipc_open(struct inode *inode, struct file *file) {
	printk("open\n");
	return 0;
}

static int ku_ipc_release(struct inode *inode, struct file *file) {
	printk("close\n");
	return 0;
}

struct file_operations ku_ipc_fops = {
	.unlocked_ioctl = ku_ipc_ioctl,
	.open = ku_ipc_open,
	.release = ku_ipc_release,
};

static int __init ku_ipc_init(void) {
	printk("init\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	char_dev = cdev_alloc();
	cdev_init(char_dev, &ku_ipc_fops);
	cdev_add(char_dev, dev_num, 1);

	return 0;
}

static void __exit ku_ipc_exit(void) {
	printk("exit\n");

	cdev_del(char_dev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
