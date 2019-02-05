# Jean Claude Zarate <jzarate@wpi.edu> and James Scherick
obj-m := cs3013_project2.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc procAncestry.c -o procAncestry
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm procAncestry
