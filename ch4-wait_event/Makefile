KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := simple_block.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc simple_block_reader.c -o sbr
	arm-linux-gnueabi-gcc simple_block_writer.c -o sbw

copy:
	scp mknod.sh simple_block.ko sbr sbw pi@10.10.10.13:~/ex/ex7-wait_event

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
	rm sbr
	rm sbw
