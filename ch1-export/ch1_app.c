#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define MOD1_DEV_NAME "challenge_export_mod1_dev"
#define MOD2_DEV_NAME "challenge_export_mod2_dev"

#define IOCTL_START_NUM 0X80
#define IOCTL_INCREASE_MY_VAR IOCTL_START_NUM+1

#define IOCTL_NUM 'z'
#define INCREASE_MY_VAR _IOWR(IOCTL_NUM, IOCTL_INCREASE_MY_VAR, unsigned long *)

void main() {
	int module1, module2;
	unsigned long value = 0;
	char dev_loc1[255] = "/dev/";
	char dev_loc2[255] = "/dev/";
	strcat(dev_loc1, MOD1_DEV_NAME);
	strcat(dev_loc2, MOD2_DEV_NAME);

	module1 = open(dev_loc1, O_RDWR);
	module2 = open(dev_loc2, O_RDWR);
	ioctl(module1, INCREASE_MY_VAR, &value);
	ioctl(module1, INCREASE_MY_VAR, &value);
	close(module1);
	close(module2);
}
