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
struct ku_pir_data_list {
	struct list_head list;
	struct ku_pir_data data;
	int fd;
};

spinlock_t ku_pir_lock;
wait_queue_head_t ku_pir_wq;

void init_fds(void) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	for(i = 0; i < max_fd; i++) {
		data_queue_list[i] = 0L;
	}
}

static int ku_pir_open(struct inode *inode, struct file* file) { 
	init_fds();
	return 0; 
}

static int ku_pir_release(struct inode *inode, struct file* file) { return 0; }

int open(void) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	for(i = 0; i < max_fd; i++) {
		if (data_queue_list[i] == 0) {
			// init a queue
			struct ku_pir_data_list data_list;
			INIT_LIST_HEAD(&data_list.list);
			data_queue_list[i] = &data_list;
			return i;
		}
	}
	return -1;
}

int close(int fd) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	if(max_fd < fd || data_queue_list[fd] == 0) {
		printk("There is no fd %d", fd);
		return -1;
	}
	else {
		data_queue_list[fd] = 0;
		return 0;
	}
}

int insertData(struct ku_pir_data *arg) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	struct ku_pir_data data = *arg;
	//	int ret = copy_from_user(&data, (struct ku_pir_data *)arg, sizeof(struct ku_pir_data));		// IS IT NECESSARY?

	for(i = 0; i < max_fd; i++) {
		struct ku_pir_data_list data_list;
		struct ku_pir_data_list *new_data;

		if(data_queue_list[i] == 0) continue;

		data_list = *(struct ku_pir_data_list*)data_queue_list[i];
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

int get_queue_size(int fd) {
	struct ku_pir_data_list *data_list = (struct ku_pir_data_list *)data_queue_list[i];
	struct ku_pir_data_list *tmp = 0;
	int result = 0;

	spin_lock(&ku_pir_lock);
	list_for_each_entry(tmp, &data_list->list, list){
		result += 1;
	}
	spin_unlock(&ku_pir_lock);

	return result;
}

void linked_list_pop(struct ku_pir_data_list * mylist, struct ku_pir_data* popped_data) {
	struct ku_pir_data_list *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	int ctu_result = -1;

	spin_lock(&ku_pir_lock);
	list_for_each_safe(pos, q, &mylist->list){
		tmp = list_entry(pos, struct ku_pir_data_list, list);
		ctu_result = copy_to_user(popped_data, &tmp->data, sizeof(struct ku_pir_data));
		if(ctu_result < 0) {
			printk("Failed copy_to_user");
			return;
		}
		list_del(pos);
		kfree(tmp);
		break;
	}
	spin_unlock(&ku_pir_lock);
}

void read(struct ioctl_read_arg *arg) {
	int fd = arg->fd;
	int max_fd = sizeof(data_queue_list) / sizeof(int);

	if(max_fd < fd || data_queue_list[fd] == 0) {
		printk("There is no fd %d", fd);
		return;
	}

	printk("fd %d is waiting...", fd);
	wait_event_interruptible(ku_pir_wq, (get_queue_size(fd) > 0));
	printk("fd %d woke up!", fd);

	linked_list_pop((struct ku_pir_data_list *)data_queue_list[fd], arg->data);	
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
			read((struct ioctl_read_arg *) arg);
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
