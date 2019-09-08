#   Group - 10
#	Vishal Gupta (15CS30039)
#	Vishesh Agarwal (15CS30040)
#	Kernel Version - 4.15.0-29-generic

obj-m += pfs.o
obj-m += tree_pfs.o

all:
	make -C /usr/src/linux-headers-$(shell uname -r) SUBDIRS=$(shell pwd) modules

clean:
	make -C /usr/src/linux-headers-$(shell uname -r) SUBDIRS=$(shell pwd) clean
