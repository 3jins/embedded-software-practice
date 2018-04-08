#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include "ku_ipc.h"

MODULE_LICENSE("GPL");

static dev_t dev_num;
static struct cdev *char_dev;
static int msqid = 0;
spinlock_t ku_ipc_lock;
struct msgbuf_str *msgbuf;
int i = 0;

struct msgbuf_str {
	long type;
	char text;	
};
struct msgq_str {
	struct list_head list;
	struct msgbuf_str msgbuf;
};
struct msgq_list_str {
	struct list_head list;
	int key;
	int msqid;
	struct msgq_str msgq;
	int length;
};
struct msgq_list_str msgq_list;

int get_msgqid(int key) {
	struct list_head *pos;
	struct msgq_list_str* tmp_msgq;
	int msgq_id = 0;
	list_for_each(pos, &msgq_list.list) {
		tmp_msgq = (struct msgq_list_str*) list_entry(pos, struct msgq_list_str, list);
		if(tmp_msgq->key == key) return msgq_id;
		msgq_id++;
	}
	return -1;
}

struct msgq_list_str* get_msgq(int id) {
	struct list_head *pos;
	struct msgq_list_str* tmp_msgq;
	int tmp_id = 0;
	list_for_each(pos, &msgq_list.list) {
		tmp_msgq = (struct msgq_list_str*) list_entry(pos, struct msgq_list_str, list);
		if(tmp_id == id) return tmp_msgq;
		tmp_id++;
	}
	return -1;
}

/* Test code */
void print_messages_in_msgq(int msqid) {
	struct list_head *pos = 0;
	list_for_each(pos, &msgq_list.list) {
		struct msgq_list_str* tmp = list_entry(pos, struct msgq_list_str, list);
		printk("msqid: %d\n", tmp->msqid);
		if(tmp->msqid == msqid) {
			struct list_head *pos2 = 0;
			list_for_each(pos2, &((tmp->msgq).list)) {
//				struct msgq_str* tmp2 = list_entry(pos2, struct msgq_str, list);
//				printk("???");
//				printk("%c\n", tmp2->msgbuf.text);
//				printk("%d\n", tmp2->msgbuf.type);
			}
		}
	}
}

int ku_msgget(int key, int msgflg) {
	int msgq_id = get_msgqid(key);
	printk("msgget\n");

	if(msgq_id >= 0) {
		/* Deal with duplication problem */
		switch(msgflg) {
			case IPC_CREAT:
				printk("Key %d is duplicated, so you have to use the existing message queue.\n", key);
				return msgq_id;
			case IPC_EXCL:
				printk("Key %d is duplicated, so you can't use this key.\n", key);
				return -1;
			default:
				printk("Only 1 or 2 can be the value of msgflg\n");
				return -1;
		}
	}
	else {
		/* Make a linked list and store/return the address */
		struct msgq_list_str* new_msgq = (struct msgq_list_str*)kmalloc(sizeof(struct msgq_list_str), GFP_KERNEL);
		new_msgq->key = key;
		new_msgq->msqid = msqid++;
		list_add_tail(&new_msgq->list, &msgq_list.list);
		// If I put 'hello', the msgq will look like this : head->[h]->[e]->[l]->[l]->[o]->head
		printk("Succeeded to create a message queue with key %d!\n", key);

		//print_messages_in_msgq(msqid - 1);
		return msqid - 1;
	}
	return -1;
}

int ku_msgclose(int msqid) {
	struct msgq_list_str *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	printk("msgclose\n");

	list_for_each_safe(pos, q, &msgq_list.list) {
		tmp = list_entry(pos, struct msgq_list_str, list);
		printk("%d %d\n", tmp->msqid, msqid);
		//print_messages_in_msgq(msqid);

		if(tmp->msqid == msqid) {
			list_del(pos);
			kfree(tmp);
			printk("Deleted msq %d.\n", msqid);
			return 0;
		}
	}
	printk("no msgq %d\n", msqid);
	return -1;
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int result = -1;
	switch(cmd) {
		case IOCTL_MSGGET:
			{
				struct msgget_str *get_arg = (struct msgget_str*)arg;
				result = ku_msgget(
						get_arg->key, 
						get_arg->msgflg);
				break;
			}
		case IOCTL_MSGCLOSE:
			{
				struct msgclose_str *close_arg = (struct msgclose_str*)arg;
				result = ku_msgclose(close_arg->msqid);
				break;
			}
	}
	printk("%d\n", result);
	return result;
}

static int ku_ipc_open(struct inode *inode, struct file *file) {
	printk("open\n");
	return 0;
}

static int ku_ipc_release(struct inode *inode, struct file *file) {
	printk("release\n");
	return 0;
}

static int ku_ipc_write(struct file *file, const char *buf, size_t len, loff_t *lof) {
	struct msgq_list_str *tmp = 0;
	struct list_head *pos = 0;

	struct msgsnd_str *kbuf = (struct msgsnd_str *)kmalloc(sizeof(struct msgsnd_str), GFP_KERNEL);
	int result = copy_from_user(kbuf, (struct msgsnd_str *)buf, sizeof(struct msgsnd_str));
	printk("write\n");

	/* Exception handling */
	if(kbuf->msgsz > KUIPC_MAXVOL) {
		kfree(kbuf);
		printk("Too long\n");
		return -1;
	}
	if(kbuf->msgsz < 0) {
		kfree(kbuf);
		printk("Size is less than 0\n");
		return -1;
	}

	/* Find a msgq by mqid */
	list_for_each(pos, &msgq_list.list) {
		tmp = list_entry(pos, struct msgq_list_str, list);
		/* Gacha */
		if(tmp->msqid == kbuf->msqid) {
			spin_lock(&ku_ipc_lock);
			if(tmp->length >= KUIPC_MAXMSG) {
				if(kbuf->msgflg == IPC_NOWAIT) {
					printk("Too many\n");
					kfree(kbuf);
					spin_unlock(&ku_ipc_lock);
					return -1;
				} else {
					spin_unlock(&ku_ipc_lock);
					while(tmp->length >= KUIPC_MAXMSG);
					spin_lock(&ku_ipc_lock);
				}
			}

			struct msgq_str new_msgq;
			INIT_LIST_HEAD(&new_msgq.list);
			for(i=0; i<len; i++) {
				struct msgq_str new_msg;
				struct msgbuf_str msg = {
					.type = *((long*)(kbuf->msgp)),
					.text = *((char*)(kbuf->msgp) + i),
				};
				//printk("%c\n", msg.text);
				new_msg.msgbuf = msg; 
				list_add(&new_msg.list, &new_msgq.list);
			}
			spin_unlock(&ku_ipc_lock);

			kfree(kbuf);
			printk("write complete\n");
			return 0;
		}
	}

	/* No msgq */
	printk("No msgq\n");
	kfree(kbuf);
	return -1;
}

static int ku_ipc_read(struct file *file, char *buf, size_t len, loff_t *lof) {
	printk("read\n");
	return 0;
}

struct file_operations ku_ipc_fops = {
	.unlocked_ioctl = ku_ipc_ioctl,
	.open = ku_ipc_open,
	.release = ku_ipc_release,
	.write = ku_ipc_write,
	.read = ku_ipc_read,
};

static int __init ku_ipc_init(void) {
	printk("init\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	char_dev = cdev_alloc();
	cdev_init(char_dev, &ku_ipc_fops);
	cdev_add(char_dev, dev_num, 1);

	INIT_LIST_HEAD(&msgq_list.list);

	return 0;
}

static void __exit ku_ipc_exit(void) {
	printk("exit\n");

	cdev_del(char_dev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);
