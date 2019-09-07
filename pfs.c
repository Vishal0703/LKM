#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/string.h>

#define MAX_PROC 100
#define FILE_NAME "partb_1_15CS30039"

struct data_holder
{
	char data[100];
	int pid;
};

struct data_holder proc_data[MAX_PROC];
int ret;
static struct proc_dir_entry *OPF;


void initialize_data(void)
{
	int i;
	for(i = 0; i<MAX_PROC; i++)
	{
		proc_data[i].data[0] = '\0';
		proc_data[i].pid = -1;
	}
}

int find_proc(int pid)
{
	int i;
	for(i=0; i<MAX_PROC; i++)
	{
		if(proc_data[i].pid == pid)
			return i;
	}
	return -1;
}

int find_first_free(void)
{
	int i;
	for(i=0; i< MAX_PROC; i++)
	{
		if(proc_data[i].pid == -1)
			return i;
	}
	return -1;
}

int device_open(struct inode* inode, struct file* filp)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	if(id != -1)
	{
		printk(KERN_ALERT "File already opened by this process\n");
		return -1;
	}
	id = find_first_free();
	if(id == -1)
	{
		printk(KERN_ALERT "No more processes can be allocated currently to the file\n");
		return -1;
	}
	proc_data[id].pid = pid;
	proc_data[id].data[0] = '\0';
	printk(KERN_INFO "pfs: device opened\n");
	printk(KERN_INFO "pfs: The process id is %d\n", pid);
	printk(KERN_INFO "pfs: Index is %d\n", id);
	return 0;
} 

int device_close(struct inode* inode, struct file* filp)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	proc_data[id].pid = -1;
	proc_data[id].data[0] = '\0';
	printk(KERN_INFO "pfs: closing device\n");
	return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	printk(KERN_INFO "pfs: reading data\n");
	ret = copy_to_user(bufStoreData, proc_data[id].data, bufsize);
	return ret;
}

ssize_t device_write(struct file *filp, const char* bufSourceData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	printk(KERN_INFO "pfs: writing data to process = %d\n", pid);
	printk(KERN_INFO "pfs: Index is %d\n", id);
	ret = copy_from_user(proc_data[id].data, bufSourceData, bufsize);
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
	OPF = proc_create(FILE_NAME, 0666, NULL, &fops);
	initialize_data();
	printk(KERN_INFO "pfs: allocated pfs entry");
	return 0;
}

static void driver_exit(void)
{
	remove_proc_entry(FILE_NAME, NULL);
	printk(KERN_ALERT "pfs: device unloaded\n");			
}

module_init(driver_entry);
module_exit(driver_exit);