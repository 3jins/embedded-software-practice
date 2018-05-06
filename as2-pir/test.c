#include <stdio.h>
#include "ku_pir_lib.c"
#include <sys/types.h>
#include <unistd.h>

int main(void) {
	int fd = ku_pir_open();
	struct ku_pir_data *data;
	int pid;

	// scenario 1 : single process
	ku_pir_insertData(fd, 123, 1);
	ku_pir_insertData(fd, 1234, 1);
	ku_pir_insertData(fd, 12349, 0);
	ku_pir_read(fd, data);
	printf("%lu %c\n", data->timestamp, data->rf_flag);
	ku_pir_read(fd, data);
	printf("%lu %c\n", data->timestamp, data->rf_flag);
	ku_pir_read(fd, data);
	printf("%lu %c\n", data->timestamp, data->rf_flag);
	ku_pir_close(fd);

	// scenario 2 : dual process
//	pid = fork();
//	if(pid == 0) {	// child
//		ku_pir_insertData(fd, 923, 1);
//		ku_pir_insertData(fd, 9234, 1);
//		ku_pir_insertData(fd, 92349, 0);
//		ku_pir_read(fd, data);
//		printf("%lu %c\n", data->timestamp, data->rf_flag);
//		ku_pir_read(fd, data);
//		printf("%lu %c\n", data->timestamp, data->rf_flag);
//		ku_pir_read(fd, data);
//		printf("%lu %c\n", data->timestamp, data->rf_flag);
//		ku_pir_close(fd);
//	}
//	else {	// parent
//		ku_pir_insertData(fd, 123, 1);
//		ku_pir_insertData(fd, 1234, 1);
//		ku_pir_insertData(fd, 12349, 0);
//		ku_pir_read(fd, data);
//		printf("%lu %c\n", data->timestamp, data->rf_flag);
//		ku_pir_read(fd, data);
//		printf("%lu %c\n", data->timestamp, data->rf_flag);
//		ku_pir_read(fd, data);
//		printf("%lu %c\n", data->timestamp, data->rf_flag);
//		ku_pir_close(fd);
//	}

	return 0;
}
