KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := ku_pir.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o test test.c
#	arm-linux-gnueabi-gcc -o reader_rcu reader_rcu.c

copy:
	scp test mknod.sh ku_pir.ko pi@10.10.10.13:~/as/as2-pir

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
	rm test 
#	rm reader_rcu
