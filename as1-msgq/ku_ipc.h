#ifndef __KU_IPC_H__
#define __KU_IPC_H__

#define KUIPC_MAXMSG 1024
#define KUIPC_MAXVOL 128 
#define IPC_CREAT 00001000
#define IPC_EXCL 00002000
#define IPC_NOWAIT 00004000
#define MSG_NOERROR 00008000 

#define DEV_NAME "ku_ipc_dev"

// ioctl code & seq#s
#define IOCTL_CODE		'z'
#define IOCTL_SEQ_START 0x80
#define IOCTL_MSGGET	_IOWR(IOCTL_CODE, IOCTL_SEQ_START + 0, struct msgget_str*)
#define IOCTL_MSGCLOSE	_IOWR(IOCTL_CODE, IOCTL_SEQ_START + 1, struct msgclose_str*)
#define IOCTL_MSGSND	_IOWR(IOCTL_CODE, IOCTL_SEQ_START + 2, struct msgsnd_str*)
#define IOCTL_MSGRCV	_IOWR(IOCTL_CODE, IOCTL_SEQ_START + 3, struct msgrcv_str*)

/* data structures */
struct msgget_str {
	int key;
	int msgflg;
};
struct msgclose_str {
	int msqid;
};
struct msgsnd_str {
	void *msgp;
	int msqid;
	int msgsz;
	int msgflg;
};
struct msgrcv_str {
	int msqid;
	int msgsz;
	int msgtype;
	int msgflg;
};

/* util functions */
int arrlen(void* arr) {
	if(sizeof(arr) == 0) return 0;
	return sizeof(arr) / sizeof(arr[0]);
}

#endif
