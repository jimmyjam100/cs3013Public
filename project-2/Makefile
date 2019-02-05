# Jean Claude Zarate <jzarate@wpi.edu>
obj-m := cs3013_project2.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
i:
	sudo rmmod cs3013_project2 & make && sudo insmod cs3013_project2.ko
