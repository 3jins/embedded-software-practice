#include <stdio.h>
#include <sys/fcntl.h>

int main(void) {
	int fd, ret;

	fd = open("/dev/simple_block_dev", O_RDWR);
	ret = read(fd, NULL, 0);
	if(ret >= 0) {
		printf("Success to consume: %d\n", ret);
	}
	else {
		printf("Fail to consume\n");
	}

	close(fd);

	return 0;
}
