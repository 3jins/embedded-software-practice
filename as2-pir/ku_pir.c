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

int i, j = 0;
//struct ku_pir_data_list pir_list;

void init_fds(void) {
	int max_fd = sizeof(fds) / sizeof(int);
	for(i = 0; i < max_fd; i++) {
		fds[i] = 0L;
	}
}

static int ku_pir_open(struct inode *inode, struct file* file) { 
	init_fds();
	return 0; 
}

static int ku_pir_release(struct inode *inode, struct file* file) { return 0; }

int open(void) {
	int max_fd = sizeof(fds) / sizeof(int);
	for(i = 0; i < max_fd; i++) {
		if (fds[i] == 0) {
			// init a queue
			struct ku_pir_data_list data_list;
			INIT_LIST_HEAD(&data_list.list);
			fds[i] = &data_list;
			return i;
		}
	}
	return -1;
}

static long ku_pir_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	long ret = 0L;

	switch(cmd){
		case KU_IOCTL_OPEN:
			ret = open();
			break;
		case KU_IOCTL_CLOSE:
			break;
		case KU_IOCTL_READ:
			break;
		case KU_IOCTL_FLUSH:
			break;
		case KU_IOCTL_INSERT:
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
