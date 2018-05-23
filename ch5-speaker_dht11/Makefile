KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := dht11.o #speaker.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules

copy:
	scp dht11.ko pi@10.10.10.13:~/ch/ch5-speaker_dht11
#	scp speaker.ko pi@10.10.10.13:~/ch/ch5-speaker_dht11

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
