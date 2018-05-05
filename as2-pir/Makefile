KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := ku_pir.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
#	arm-linux-gnueabi-gcc -o reader reader.c
#	arm-linux-gnueabi-gcc -o reader_rcu reader_rcu.c

copy:
	scp mknod.sh ku_pir.ko pi@10.10.10.13:~/as/as2-ku_pir

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
#	rm reader
#	rm reader_rcu
