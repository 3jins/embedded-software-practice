#include <stdio.h>
#include <sys/fcntl.h>

int main(void) {
	int dev;
	char c;

	dev = open("/dev/led_sensor_dev", O_RDWR);
	scanf("%c", &c);	// Turn on until put enter
	close(dev);

	return 0;
}
