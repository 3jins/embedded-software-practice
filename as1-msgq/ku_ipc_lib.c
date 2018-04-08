#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "ku_ipc.h"

int ku_msgget(int key, int msgflg) {
	struct msgget_str msgget_arg = {
		.key = key,
		.msgflg = msgflg,
	};

	int dev = open("/dev/ku_ipc_dev", O_RDWR);
	printf("what are you doin %d\n", dev);
	int result = ioctl(dev, IOCTL_MSGGET, &msgget_arg);
	close(dev);

	return result;
}

int ku_msgclose(int msqid) {
	struct msgclose_str msgclose_arg = {
		.msqid = msqid,
	};

	int dev = open("/dev/ku_ipc_dev", O_RDWR);
	int result = ioctl(dev, IOCTL_MSGCLOSE, &msgclose_arg);
	close(dev);

	return result;
}

int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg) {
	struct msgsnd_str msgsnd_arg = {
		.msgp = msgp,
		.msqid = msqid,
		.msgsz = msgsz,
		.msgflg = msgflg,
	};

	int dev = open("/dev/ku_ipc_dev", O_RDWR);
	int result = write(dev, (char*)&msgsnd_arg, msgsz);
	close(dev);

	return result;
}

int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtype, int msgflg) {
	struct msgrcv_str msgrcv_arg = {
		.msqid = msqid,
		.msgsz = msgsz,
		.msgtype = msgtype,
		.msgflg = msgflg,
	};

	int dev = open("/dev/ku_ipc_dev", O_RDWR);
	read(dev, msgp, msgsz);
	int result = ioctl(dev, IOCTL_MSGRCV, &msgrcv_arg);
	close(dev);

	return result;
}

