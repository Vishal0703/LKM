#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>

#define DRIVER_AUTHOR "Vishal Gupta"
#define DRIVER_DESC "Devil"

static int __init hello_4_init(void)
{
	printk(KERN_ALERT "Hello World 4.\n");
	return 0;
}

static void __exit hello_4_exit(void)
{
	printk(KERN_ALERT "Goodbye World 4.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("testdevice");

module_init(hello_4_init);
module_exit(hello_4_exit);