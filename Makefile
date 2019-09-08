#   Group - 10
#	Vishal Gupta (15CS30039)
#	Vishesh Agarwal (15CS30040)
#	Kernel Version - 4.15.0-29-generic

obj-m += partb_A1.o
obj-m += partb_A2.o

all:
	make -C /usr/src/linux-headers-$(shell uname -r) SUBDIRS=$(shell pwd) modules

clean:
	make -C /usr/src/linux-headers-$(shell uname -r) SUBDIRS=$(shell pwd) clean
