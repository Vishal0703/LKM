# obj-m += hello-1.o
# obj-m += hello-2.o
# obj-m += hello-3.o
# obj-m += hello-4.o
# obj-m += hello-5.o
# obj-m += startstop.o
# startstop-objs := start.o stop.o
# obj-m += mydevice.o
obj-m += pfs.o
#obj-m += newdevice.o
# LINVER := $(shell uname -r)
# CWDD := $(shell pwd)
all:
	make -C /usr/src/linux-headers-$(shell uname -r) SUBDIRS=$(shell pwd) modules

clean:
	make -C /usr/src/linux-headers-$(shell uname -r) SUBDIRS=$(shell pwd) clean
