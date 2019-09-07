#include<linux/kernel.h>
#include<linux/module.h>
/* We're doing kernel work */
/* Specifically, a module */
void cleanup_module()
{
	printk("<1>Short is the life of a kernel module\n");
}
