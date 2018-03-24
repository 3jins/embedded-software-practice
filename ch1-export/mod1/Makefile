#KDIR := ~/rp/rp-kernel/linux-rpi
KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

KBUILD_EXTRA_SYMBOLS := $(PWD)/Module.symvers
obj-m := ch1_mod.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
