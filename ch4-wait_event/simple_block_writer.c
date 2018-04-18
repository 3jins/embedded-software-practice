#include <stdio.h>
#include <sys/fcntl.h>

int main(void) {
	int fd, ret;

	fd = open("/dev/simple_block_dev", O_RDWR);
	ret = write(fd, NULL, 0);
	if(ret >= 0) {
		printf("Success to produce: %d\n", ret);
	}
	else {
		printf("Failed to produce\n");
	}

	return 0;
}
