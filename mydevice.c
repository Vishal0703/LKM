#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/string.h>


struct fake_device
{
	char data[100];
	struct semaphore sem;
}virtual_device;

struct cdev *mcdev;
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "mytestdevice"


int device_open(struct inode* inode, struct file* filp)
{
	if(down_interruptible(&virtual_device.sem) != 0)
	{
		printk(KERN_ALERT "mytestdevice: couldn't lock device during open\n");
		return -1;
	}
	printk(KERN_INFO "mytestdevice: device opened\n");
	printk(KERN_INFO "The process id is %d\n", (int)task_pid_nr(current));
	return 0;
} 

int device_close(struct inode* inode, struct file* filp)
{
	up(&virtual_device.sem);
	printk(KERN_INFO "mytestdevice: closing device\n");
	return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufsize, loff_t* curroffset)
{
	printk(KERN_INFO "mytestdevice: reading data\n");
	ret = copy_to_user(bufStoreData, virtual_device.data, bufsize);
	return ret;
}

ssize_t device_write(struct file *filp, const char* bufSourceData, size_t bufsize, loff_t* curroffset)
{
	printk(KERN_INFO "mytestdevice: writing data\n");
	ret = copy_from_user(virtual_device.data, bufSourceData, bufsize);
	return ret;
}


struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.read = device_read,
	.write = device_write
};

static int driver_entry(void)
{
	ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
	if(ret < 0)
	{
		printk(KERN_ALERT "mytestdevice: failed to allocate a major number\n");
		return ret;
	}

	major_number = MAJOR(dev_num);
	printk(KERN_INFO "mytestdevice: major number allocated is %d\n", major_number);
	printk(KERN_INFO "mytestdevice: use \"mknod /dev/%s c %d 0\" for device file", DEVICE_NAME, major_number);

	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	ret = cdev_add(mcdev, dev_num, 1);
	if(ret < 0)
	{
		printk(KERN_ALERT "mytestdevice: device addition failed\n");
		return ret;
	}

	sema_init(&virtual_device.sem, 1);
	return 0;
}

static void driver_exit(void)
{
	cdev_del(mcdev);

	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "mytestdevice: device unloaded\n");			
}

module_init(driver_entry);
module_exit(driver_exit);