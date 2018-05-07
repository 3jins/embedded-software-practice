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

static int irq_num;

void init_data_queue_list(void) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	for(i = 0; i < max_fd; i++) {
		data_queue_list[i] = 0L;
	}
}

static int ku_pir_open(struct inode *inode, struct file* file) { return 0; }

static int ku_pir_release(struct inode *inode, struct file* file) { return 0; }

int open(void) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	for(i = 0; i < max_fd; i++) {
		if (data_queue_list[i] == 0) {
			// init a queue
			struct ku_pir_data_list data_list;
			INIT_LIST_HEAD(&data_list.list);

			spin_lock(&ku_pir_lock);	
			data_queue_list[i] = (long)&data_list;
			spin_unlock(&ku_pir_lock);
			return i;
		}
	}
	return -1;
}

int close(int fd) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	int ret = -1;
	struct ku_pir_data_list *tmp = 0;

	spin_lock(&ku_pir_lock);
	if(max_fd < fd || data_queue_list[fd] == 0) {
		printk("[close] There is no fd %d", fd);
		ret = -1;
	}
	else {
		spin_lock(&ku_pir_lock);
		list_del_rcu(&((struct ku_pir_data_list *)data_queue_list[fd])->list);
		kfree((struct ku_pir_data_list *)data_queue_list[fd]);
		synchronize_rcu();
		data_queue_list[fd] = 0;
		spin_unlock(&ku_pir_lock);
		ret = 0;
	}
	spin_unlock(&ku_pir_lock);

	return ret;
}

int get_queue_size(struct ku_pir_data_list *data_list) {
	struct ku_pir_data_list *tmp = 0;
	int result = 0;

	spin_lock(&ku_pir_lock);
	list_for_each_entry(tmp, &data_list->list, list){
		printk("[%d] %lu", tmp->fd, tmp->data.timestamp);
		printk("???%d", result);
		result++;
	}
	spin_unlock(&ku_pir_lock);
	printk("[get_queue_size] result: %d", result);

	return result;
}

void linked_list_pop(struct ku_pir_data_list *data_list, struct ku_pir_data* popped_data) {
	struct ku_pir_data_list *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	int ctu_result = -1;

	spin_lock(&ku_pir_lock);
	list_for_each_safe(pos, q, &data_list->list){
		tmp = list_entry(pos, struct ku_pir_data_list, list);
		ctu_result = copy_to_user(popped_data, &tmp->data, sizeof(struct ku_pir_data));
		if(ctu_result < 0) {
			printk("[linked_list_pop] Failed copy_to_user");
			return;
		}
		list_del(pos);
		kfree(tmp);
		break;
	}
	spin_unlock(&ku_pir_lock);
}

void read(struct ioctl_arg *arg) {
	int fd = arg->fd;
	int max_fd = sizeof(data_queue_list) / sizeof(int);

	if(max_fd < fd || data_queue_list[fd] == 0) {
		printk("[read] There is no fd %d", fd);
		return;
	}

	printk("[read] fd %d is waiting...", fd);
	wait_event_interruptible(ku_pir_wq, (get_queue_size((struct ku_pir_data_list *)data_queue_list[fd]) > 0));
	printk("[read] fd %d woke up!", fd);

	linked_list_pop((struct ku_pir_data_list *)data_queue_list[fd], arg->data);	
}

int insertData(struct ioctl_arg *arg) {
	int max_fd = sizeof(data_queue_list) / sizeof(int);
	//struct ioctl_arg local_arg = 
	//int ret = copy_from_user(&data, (struct ku_pir_data *)arg, sizeof(struct ku_pir_data));		// IS IT NECESSARY?
	int fd = arg->fd;
	struct ku_pir_data data = *(arg->data);
	struct ku_pir_data_list *data_list;
	struct ku_pir_data_list *new_data;

	if(max_fd < fd || data_queue_list[fd] == 0) {
		printk("[insertData] There is no fd %d", fd);
		return -1;
	}

	data_list = (struct ku_pir_data_list *)data_queue_list[fd];
	new_data = (struct ku_pir_data_list *)kmalloc(sizeof(struct ku_pir_data_list), GFP_KERNEL);
	new_data->data = data;

	spin_lock(&ku_pir_lock);
	list_add_tail(&new_data->list, &data_list->list);
	spin_unlock(&ku_pir_lock);

	wake_up_interruptible(&ku_pir_wq);

	//	return ret;
	return 0;
}

void flush(int fd) {
	struct ku_pir_data_list *tmp = 0;
	struct ku_pir_data_list *data_list = (struct ku_pir_data_list *)data_queue_list[fd];
	struct list_head *pos = 0;
	struct list_head *q = 0;

	spin_lock(&ku_pir_lock);
	list_for_each_safe(pos, q, &data_list->list){
		tmp = list_entry(pos, struct ku_pir_data_list, list);
		list_del(pos);
		kfree(tmp);
	}
	spin_unlock(&ku_pir_lock);
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
			read((struct ioctl_arg *) arg);
			break;
		case KU_IOCTL_FLUSH:
			flush(*(int *)arg);	
			break;
		case KU_IOCTL_INSERT:
			ret = insertData((struct ioctl_arg *) arg);
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

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_pir_init(void){
	int ret;

	printk("Init Module\n");

	/* Initialize data_queue_list */
	init_data_queue_list();

	/* Allocate character device */
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_pir_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if(ret<0){
		printk("[init] Fail to add a character device\n");
		return -1;
	}

	/* init list, waitqueue, spin_lock */
	//	INIT_LIST_HEAD(&kern_queues.list);
	init_waitqueue_head(&ku_pir_wq);
	spin_lock_init(&ku_pir_lock);

	/* init kernel file descriptor */
	//	kern_fd=0;

	/* requset GPIO and interrupt handler */
	gpio_request_one(KUPIR_SENSOR, GPIOF_IN, "sensor");
	irq_num = gpio_to_irq(KUPIR_SENSOR);
	//	ret = request_irq(irq_num, ku_pir_isr, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "ku_irq", NULL);
	//	if(ret){
	//		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
	//		free_irq(irq_num, NULL);
	//	}

	return ret;
}

static void __exit ku_pir_exit(void){
	struct queues *tmp;
	struct list_head *pos = 0;
	struct list_head *q = 0;

	printk("Exit Module\n");

	//	list_for_each_safe(pos, q, &kern_queues.list){
	//		tmp = list_entry(pos, struct queues, list);
	//		list_del(pos);
	//		kfree(tmp);
	//	}

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	//	disable_irq(irq_num);
	//	free_irq(irq_num, NULL);
	gpio_free(KUPIR_SENSOR);
}

module_init(ku_pir_init);
module_exit(ku_pir_exit);
