#include <stdio.h>
#include "ku_pir_lib.c"
#include <sys/types.h>
#include <unistd.h>

int main(void) {
	int fd1, fd2;
	struct ku_pir_data data;
	int pid;

	// scenario 1 : single process
//	printf("Scenario1\n");
//	fd1 = ku_pir_open();
//	ku_pir_insertData(fd1, 123, 1);
//	ku_pir_insertData(fd1, 1234, 1);
//	ku_pir_insertData(fd1, 12349, 0);
//	ku_pir_read(fd1, &data);
//	printf("%lu %d\n", data.timestamp, data.rf_flag);
//	ku_pir_read(fd1, &data);
//	printf("%lu %d\n", data.timestamp, data.rf_flag);
//	ku_pir_read(fd1, &data);
//	printf("%lu %d\n", data.timestamp, data.rf_flag);
//	ku_pir_close(fd1);

	// scenario 2 : dual process
	printf("Scenario2\n");
	pid = fork();
	if(pid == 0) {	// child
		fd1 = ku_pir_open();
//		ku_pir_insertData(fd1, 923, 1);
//		ku_pir_insertData(fd1, 9234, 1);
//		ku_pir_insertData(fd1, 92349, 0);
//		ku_pir_read(fd1, &data);
//		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		ku_pir_read(fd1, &data);
//		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		ku_pir_read(fd1, &data);
//		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		ku_pir_read(fd1, &data);
		ku_pir_close(fd1);
	}
	else {	// parent
		fd2 = ku_pir_open();
//		ku_pir_insertData(fd2, 123, 1);
//		ku_pir_insertData(fd2, 1234, 1);
//		ku_pir_insertData(fd2, 12349, 0);
//		ku_pir_read(fd2, &data);
//		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		ku_pir_read(fd2, &data);
//		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		ku_pir_read(fd2, &data);
//		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		ku_pir_read(fd2, &data);
		ku_pir_close(fd2);
	}

	return 0;
}
