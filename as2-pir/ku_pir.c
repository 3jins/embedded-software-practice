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
struct ku_pir_data_list {
	struct list_head list;
	struct ku_pir_data data;
	int fd;
};
static struct ku_pir_data_list data_queue_list[MAX_FD];
char not_null_list[MAX_FD];

spinlock_t ku_pir_lock;
wait_queue_head_t ku_pir_wq;

static int irq_num;

void init_data_queue_list(void) {
	int max_fd = sizeof(data_queue_list) / sizeof(struct ku_pir_data_list) - 1;
	for(i = 0; i < max_fd; i++) {
		not_null_list[i] = 0;
	}
}

void test_print(int fd, char* flag){
	struct ku_pir_data_list *tmp = 0;

	spin_lock(&ku_pir_lock);
	printk("[test in %s] Printing all data in the queue", flag);

	list_for_each_entry(tmp, &data_queue_list[fd].list, list){
		if(tmp->fd < 0) continue;	// Start node
		printk("[test in %s] fd: %d, timestamp: %lu", flag, tmp->fd, tmp->data.timestamp);
	}
	spin_unlock(&ku_pir_lock);
}

int open(void) {
	int max_fd = sizeof(data_queue_list) / sizeof(struct ku_pir_data_list) - 1;

	for(i = 0; i < max_fd; i++) {
		if (not_null_list[i] == 0) {
			static struct ku_pir_data_list data_list;	// It needs to be declared STATIC. Otherwise, each node of linked list will be disappeared frequently.
			data_list.fd = -1;
			INIT_LIST_HEAD(&data_list.list);
//			printk("%d", &data_list);
//			printk("%d", data_list);
//			printk("%d", &data_list.list);
//			printk("%d", data_list.list);
//			printk("%d", data_list.list.next);
//			printk("%d", *data_list.list.next);

			spin_lock(&ku_pir_lock);
			data_queue_list[i] = data_list;
			not_null_list[i] = 1;
			spin_unlock(&ku_pir_lock);
			printk("[open] fd: %d", i);
			return i;
		}
	}
	return -1;
}

int close(int *fd_param) {
	int fd = *fd_param;
	int max_fd = sizeof(data_queue_list) / sizeof(struct ku_pir_data_list) - 1;
	int ret = -1;

	printk("[close] fd: %d", fd);
	spin_lock(&ku_pir_lock);
	if(max_fd < fd || not_null_list[fd] == 0) {
		printk("[close] There is no fd %d", fd);
		ret = -1;
	}
	else {
		not_null_list[i] = 0;
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
		printk("<<%d>>", tmp->fd);
		if(tmp->fd < 0) continue;	// Start node
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
	int ret = -1;

	spin_lock(&ku_pir_lock);
	list_for_each_safe(pos, q, &data_list->list){
		tmp = list_entry(pos, struct ku_pir_data_list, list);
		if(tmp->fd < 0) continue;	// Start node
		ret = copy_to_user(popped_data, &tmp->data, sizeof(struct ku_pir_data));
		list_del(pos);
		kfree(tmp);
		break;
	}
	spin_unlock(&ku_pir_lock);
}

void read(struct ioctl_arg *arg) {
	int fd = arg->fd;
	int max_fd = sizeof(data_queue_list) / sizeof(struct ku_pir_data_list) - 1;
	
	if(max_fd < fd || not_null_list[fd] == 0) {
		printk("[read] There is no fd %d", fd);
		return;
	}
//	test_print(fd, "read");

	printk("[read] fd %d is waiting...", fd);
	wait_event_interruptible(ku_pir_wq, (list_empty(&data_queue_list[fd].list) > 0));
//	get_queue_size(&data_queue_list[fd]);
//	wait_event_interruptible(ku_pir_wq, (get_queue_size(&data_queue_list[fd]) > 0));
	printk("[read] fd %d woke up!", fd);

	if(!list_empty(&data_queue_list[fd].list)) {
		linked_list_pop(&data_queue_list[fd], arg->data);
	}
}

int insertData(struct ioctl_arg *arg_param) {
	int max_fd = sizeof(data_queue_list) / sizeof(struct ku_pir_data_list) - 1;
	struct ioctl_arg *arg = (struct ioctl_arg *)kmalloc(sizeof(struct ioctl_arg *), GFP_KERNEL);
	int ret = copy_from_user(arg, arg_param, sizeof(struct ioctl_arg));	
	int fd = arg->fd;
	struct ku_pir_data *data = arg->data;
	struct ku_pir_data_list *data_list;
	struct ku_pir_data_list *new_data;

	printk("[insertData] fd: %d", fd);
	if(max_fd < fd || not_null_list[fd] == 0) {
		printk("[insertData] There is no fd %d", fd);
		return -1;
	}

	data_list = &data_queue_list[fd];
	new_data = (struct ku_pir_data_list *)kmalloc(sizeof(struct ku_pir_data_list), GFP_KERNEL);
	new_data->data = *data;
	new_data->fd = fd;

	spin_lock(&ku_pir_lock);
	list_add_tail(&new_data->list, &data_list->list);
	spin_unlock(&ku_pir_lock);
	
	test_print(fd, "insertData");

	wake_up_interruptible(&ku_pir_wq);

	return ret;
}

void flush(int *fd) {
	struct ku_pir_data_list *tmp = 0;
	struct ku_pir_data_list *data_list = &data_queue_list[*fd];
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
			ret = close((int *)arg);
			break;
		case KU_IOCTL_READ:
			read((struct ioctl_arg *) arg);
			break;
		case KU_IOCTL_FLUSH:
			flush((int *)arg);	
			break;
		case KU_IOCTL_INSERT:
			ret = insertData((struct ioctl_arg *) arg);
			break;
		default:
			break;
	}

	return ret;
};

static int ku_pir_open(struct inode *inode, struct file* file) { return 0; }

static int ku_pir_release(struct inode *inode, struct file* file) { return 0; }

struct file_operations ku_pir_fops =
{
	.open = ku_pir_open,
	.release = ku_pir_release,
	.unlocked_ioctl = ku_pir_ioctl,
};

/* ISR: It handles interrupts arisen by a pir sensor */
static irqreturn_t ku_pir_isr(int irq, void *dev_id) {
	struct ku_pir_data data;
	int max_fd = sizeof(data_queue_list) / sizeof(struct ku_pir_data_list) - 1;

	data.timestamp = jiffies;
	if(gpio_get_value(KUPIR_SENSOR) == IRQF_TRIGGER_RISING) {
		data.rf_flag = '0';
	}
	else {
		data.rf_flag = '1';
	}

	for(i=0; i<max_fd; i++) {
		struct ioctl_arg arg = {
			.data = &data,
			.fd = i,
		};
		insertData(&arg);
	}

	return IRQ_HANDLED;
}


static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_pir_init(void){
	int ret;

	printk("Init Module\n");

	/* Allocate character device */
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_pir_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if(ret<0){
		printk("[init] Fail to add a character device\n");
		return -1;
	}

	/* init list(array), waitqueue, spin_lock */
	init_data_queue_list();
	init_waitqueue_head(&ku_pir_wq);
	spin_lock_init(&ku_pir_lock);

	/* requset GPIO and interrupt handler */
	gpio_request_one(KUPIR_SENSOR, GPIOF_IN, "sensor");
	irq_num = gpio_to_irq(KUPIR_SENSOR);
	ret = request_irq(irq_num, ku_pir_isr, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "ku_pir_irq", NULL);
	if(ret){
		printk(KERN_ERR "Unable to request IRQ: %d\n", ret);
		free_irq(irq_num, NULL);
	}
	else {
		disable_irq(irq_num);
	}

	return 0;
}

static void __exit ku_pir_exit(void){
	printk("Exit Module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	gpio_free(KUPIR_SENSOR);
}

module_init(ku_pir_init);
module_exit(ku_pir_exit);
