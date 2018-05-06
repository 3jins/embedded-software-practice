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
struct ku_pir_data kernel_data;

spinlock_t ku_pir_lock;
wait_queue_head_t ku_pir_wq;

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

int close(int fd) {
	int max_fd = sizeof(fds) / sizeof(int);
	if(max_fd < fd || fds[fd] == 0) {
		printk("There is no fd %d", fd);
		return -1;
	}
	else {
		fds[fd] = 0;
		return 0;
	}
}

int insertData(struct ku_pir_data *arg) {
	int max_fd = sizeof(fds) / sizeof(int);
	struct ku_pir_data data = *arg;
//	int ret = copy_from_user(&data, (struct ku_pir_data *)arg, sizeof(struct ku_pir_data));		// IS IT NECESSARY?

	for(i = 0; i < max_fd; i++) {
		struct ku_pir_data_list data_list;
		struct ku_pir_data_list *new_data;

		if(fds[i] == 0) continue;

		data_list = *(struct ku_pir_data_list*)fds[i];
		new_data = (struct ku_pir_data_list*)kmalloc(sizeof(struct ku_pir_data_list), GFP_KERNEL);
		new_data->data = data;

		spin_lock(&ku_pir_lock);
		list_add_tail(&new_data->list, &data_list.list);
		spin_unlock(&ku_pir_lock);

		wake_up_interruptible(&ku_pir_wq);
	}

//	return ret;
	return 0;
}

static long ku_pir_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	long ret = 0L;

	switch(cmd){
		case KU_IOCTL_OPEN:
			ret = open();
			break;
		case KU_IOCTL_CLOSE:
			ret = close(*(int *)arg);
			break;
		case KU_IOCTL_READ:
			break;
		case KU_IOCTL_FLUSH:
			break;
		case KU_IOCTL_INSERT:
			ret = insertData((struct ku_pir_data *) arg);
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
