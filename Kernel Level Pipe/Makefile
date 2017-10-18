obj-m+=linepipe.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	gcc consumer.c -o consumer
	gcc producer.c -o producer

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
