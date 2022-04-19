obj-m += tictactoe.o

.PHONY: build clean load unload

build:
	make -C /lib/modules/$(shell uname -r)/build modules M=$(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build clean M=$(PWD)

load:
	sudo insmod tictactoe.ko
unload:
	-sudo rmmod tictactoe
