#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/hashtable.h>
#include<linux/slab.h>


struct fake_device
{
	int pid;
	char data[100];
	struct hlist_node my_hlist;
};

struct cdev *mcdev;
int major_number;
int ret;

dev_t dev_num;

#define DEVICE_NAME "newtestdevice"

DEFINE_HASHTABLE(a,3);
// hash_init(a);

int device_open(struct inode* inode, struct file* filp)
{
	// if(down_interruptible(&virtual_device.sem) != 0)
	// {
	// 	printk(KERN_ALERT "newtestdevice: couldn't lock device during open\n");
	// 	return -1;
	// }
	int pid = (int)task_pid_nr(current);
	
	struct fake_device temp = {
		.pid = pid,
		.data = "abc\0",
		.my_hlist = 0
	};

	hash_add(a, &temp.my_hlist, temp.pid);
	printk(KERN_INFO "newtestdevice: device opened\n");
	printk(KERN_INFO "The process id is %d\n", pid);
	return 0;
} 

int device_close(struct inode* inode, struct file* filp)
{
	// up(&virtual_device.sem);
	int bkt;
	struct fake_device *curr;
	int pid = (int)task_pid_nr(current);
	// hash_for_each(a, bkt, curr, my_hlist){
	//     // printk(KERN_INFO "data=%d is in bucket %d\n", current->data, bkt);
	//     if(curr->pid == pid)
	//     	hash_del(&curr->my_hlist);
	// }
	printk(KERN_INFO "newtestdevice: closing device\n");
	return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufsize, loff_t* curroffset)
{
	printk(KERN_INFO "newtestdevice: reading data\n");
	int bkt;
	struct fake_device *curr = kmalloc(sizeof(*curr), GFP_KERNEL);
	int pid = (int)task_pid_nr(current);
	ret = 0;
	hash_for_each(a, bkt, curr, my_hlist){
	    printk(KERN_INFO "r: data=%s is in bucket %d\n", curr->data, bkt);
	    // if(curr->pid == pid)
	    // 	ret = copy_to_user(bufStoreData, curr->data, bufsize);
	}
	
	return ret;
}

ssize_t device_write(struct file *filp, const char* bufSourceData, size_t bufsize, loff_t* curroffset)
{
	printk(KERN_INFO "newtestdevice: writing data\n");
	int bkt;
	struct fake_device *curr ;//= kmalloc(sizeof(*curr), GFP_KERNEL);
	int pid = (int)task_pid_nr(current);
	printk(KERN_INFO "pid is %d\n", pid);
	ret = 0;
	// hash_for_each_possible(a, curr, my_hlist, pid){
	hash_for_each(a,bkt,curr,my_hlist){
	    printk(KERN_INFO "w: data=%s is in with key %d\n", curr->data, curr->pid);
	    // if(curr->pid == pid)
	    // ret = copy_from_user(curr->data, bufSourceData, bufsize);
	}
	// printk(KERN_INFO "data = %s\n", a[3].data);
	printk(KERN_INFO "write done\n");
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
		printk(KERN_ALERT "newtestdevice: failed to allocate a major number\n");
		return ret;
	}

	major_number = MAJOR(dev_num);
	printk(KERN_INFO "newtestdevice: major number allocated is %d\n", major_number);
	printk(KERN_INFO "newtestdevice: use \"mknod /dev/%s c %d 0\" for device file", DEVICE_NAME, major_number);

	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	ret = cdev_add(mcdev, dev_num, 1);
	if(ret < 0)
	{
		printk(KERN_ALERT "newtestdevice: device addition failed\n");
		return ret;
	}

	// sema_init(&virtual_device.sem, 1);
	return 0;
}

static void driver_exit(void)
{
	cdev_del(mcdev);

	unregister_chrdev_region(dev_num, 1);
	printk(KERN_ALERT "newtestdevice: device unloaded\n");			
}

module_init(driver_entry);
module_exit(driver_exit);