#include <stdio.h>
#include "ku_ipc.h"

int main() {
	int q = ku_msgget(1, IPC_CREAT);
	int close_result = ku_msgclose(1);
	int snd_result = ku_msgsnd(q, NULL, 10, 10);
	int rcv_result = ku_msgrcv(q, NULL, 10, sizeof(int), 10);
	return 0;
}
