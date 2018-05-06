#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "ku_pir.h"

int ku_pir_open() {
	int dev = open("/dev/ku_pir_dev", O_RDWR);
	int fd = -1;

	if(dev>0) 
		fd = ioctl(dev, KU_IOCTL_OPEN, NULL);

	close(dev);
	return fd;
}

int ku_pir_close(int fd) {
	int dev = open("/dev/ku_pir_dev", O_RDWR);
	int ret = -1;

	ret = ioctl(dev, KU_IOCTL_CLOSE, &fd);

	close(dev);
	return ret;
}

void ku_pir_read(int fd, struct ku_pir_data *data) {
	int dev = open("/dev/ku_pir_dev", O_RDWR);
	struct ioctl_read_arg arg = {
		.data = data,
		.fd = fd,
	};

	ioctl(dev, KU_IOCTL_READ, &arg);

	close(dev);
}

void ku_pir_flush(int fd) {
	int dev = open("/dev/ku_pir_dev", O_RDWR);

	ioctl(dev, KU_IOCTL_FLUSH, &fd);

	close(dev);
}

int ku_pir_insertData(int fd, long unsigned int ts, char rf_flag) {
	int dev = open("/dev/ku_pir_dev", O_RDWR);
	int ret = -1;

	struct ku_pir_data data = {

	};

	ret = ioctl(dev, KU_IOCTL_INSERT, NULL);

	close(dev);
	return ret;
}
