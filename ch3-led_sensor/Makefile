KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := led_sensor.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
