#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/rculist.h>
#include "ku_pir.h"

MODULE_LICENSE("GPL");

struct ku_pir_data_list pir_list;

static int ku_pir_open(struct inode *inode, struct file* file) { return 0; }
static int ku_pir_release(struct inode *inode, struct file* file) { return 0; }


static long ku_pir_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	long ret;

	ret = 0L;

	switch(cmd){
		case KU_IOCTL_INSERT:
			break;
		case KU_IOCTL_READ:
			break;
		case KU_IOCTL_OPEN:
			break;
		case KU_IOCTL_CLOSE:
			break;
		case KU_IOCTL_FLUSH:
			break;
		default:
			break;
	}

	return ret;
};

struct file_operations ku_pir_fops =
{
	.open = ku_pir_open,
	.release = ku_pir_release,
	.unlocked_ioctl = ku_pir_ioctl,
};
