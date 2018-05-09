#include <stdio.h>
#include <sys/fcntl.h>
#include "ku_pir_lib.c"

int main(void) {
	int fd;
	struct ku_pir_data data;
	int mode = -1;
	int end = 0;

	fd = ku_pir_open();
	printf("[%d]\n", fd);
//	ku_pir_insertData(fd, 12345, 1);

	while(!end) {
//		ku_pir_insertData(fd, 123, 1);
		ku_pir_read(fd, &data);
		printf("%lu %d\n", data.timestamp, data.rf_flag);
//		mode = getchar();
//
//		switch(mode) {
//			case '0':
//				printf("Bye\n");
//				end = 1;
//				break;
//			case '1':
//				printf("test\n");
//				ku_pir_read(fd, &data);
//				printf("%lu %d\n", data.timestamp, data.rf_flag);
//				break;
//			case '2':
//				ku_pir_flush(fd);
//				break;
//			default:
//				continue;
//		}
	}
	ku_pir_close(fd);
	return 0;
}
