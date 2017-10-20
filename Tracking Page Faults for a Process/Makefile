obj-m +=jprobe.o
KERNELDIR= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.*
