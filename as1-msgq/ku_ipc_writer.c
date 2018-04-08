#include <stdio.h>
#include <string.h>
#include "ku_ipc_lib.c"

int main() {
	int q1 = ku_msgget(100, IPC_CREAT);
	int q2 = ku_msgget(100, IPC_CREAT);
	int q3 = ku_msgget(100, IPC_EXCL);
	char *msg = "\'hello\' is too short for the test.";
	int snd_result = ku_msgsnd(q1, msg, strlen(msg), IPC_NOWAIT);
	int rcv_result = ku_msgrcv(q1, NULL, 10, sizeof(int), 10);
	printf("q1: %d\n", q1);
	int close_result = ku_msgclose(q1);

	//printf("[writer] %d %d %d %d %d %d\n", q1, q2, q3, close_result, snd_result, rcv_result);
	return 0;
}
