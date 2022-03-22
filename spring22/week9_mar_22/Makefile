obj-m += core.o num.o

.PHONY: build clean load unload

build:
	make -C /lib/modules/$(shell uname -r)/build modules M=$(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build clean M=$(PWD)

load:
	sudo insmod core.ko
	sudo insmod num.ko
unload:
	-sudo rmmod core num
