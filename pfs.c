/* 
	Group - 10
	Vishal Gupta (15CS30039)
	Vishesh Agarwal (15CS30040)
	Kernel Version - 4.15.0-29-generic
*/	

#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/string.h>
#include<linux/sort.h>
#include<linux/errno.h>

#define MAX_PROC 100
#define FILE_NAME "partb_1_15CS30039"
#define MAX_INP
#define DRIVER_AUTHOR "15CS30039"

struct data_holder
{
	char data[100][100];
	int arr[100];
	int pid;
	int curr_write;
	int curr_read;
	int max_num;
	int mode;
	int sorted;
};

struct data_holder proc_data[MAX_PROC];
int ret;
static struct proc_dir_entry *OPF;


void initialize_data(void)
{
	int i,j;
	for(i = 0; i<MAX_PROC; i++)
	{
		for(j=0; j<100; j++)
			proc_data[i].data[j][0] = '\0';
		proc_data[i].pid = -1;
		proc_data[i].curr_write = -1;
		proc_data[i].max_num = -1;
		proc_data[i].mode = -1;
		proc_data[i].sorted = 0;
		proc_data[i].curr_read = 0;
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

int char_to_int(char* temp)
{
	int i = 0, num = 0;
	while(temp[i]!='\0')
	{
		num = num*10 + temp[i]-'0';
		i++;
	}
	return num;
}

void int_to_char(char temp[], int num)
{
	int d = num, count = 0, i = 0;
	
	while(d!=0)
	{ d = d/10;	count ++;}

	while(num!=0)
	{
		d = num%10;
		temp[count-i-1] = d +'0';
		num = num/10;
		i++;
	}
	temp[count] = '\0';
}

static int compare_int(const void *lhs, const void *rhs) 
{
    int lhs_integer = *(const int *)(lhs);
    int rhs_integer = *(const int *)(rhs);

    if (lhs_integer < rhs_integer) return -1;
    if (lhs_integer > rhs_integer) return 1;
    return 0;
}

static int compare_str(const void *lhs, const void *rhs) 
{
    char* lhs_integer = (const char *)(lhs);
    char* rhs_integer = (const char *)(rhs);

    return strcmp(lhs_integer,rhs_integer);
}

void sort_data(int id)
{
	if(proc_data[id].mode == 0)
		sort(proc_data[id].arr, proc_data[id].max_num, sizeof(int), &compare_int, NULL);
	else
		sort(proc_data[id].data, proc_data[id].max_num, sizeof(char)*100, &compare_str, NULL);
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
	printk(KERN_INFO "pfs: device opened\n");
	printk(KERN_INFO "pfs: The process id is %d and the index is %d\n", pid, id);
	return 0;
} 

int device_close(struct inode* inode, struct file* filp)
{
	int pid = (int)task_pid_nr(current), j=0;
	int id = find_proc(pid);
	proc_data[id].pid = -1;
	for(j=0; j<100; j++)
		proc_data[id].data[j][0] = '\0';
	proc_data[id].pid = -1;
	proc_data[id].curr_write = -1;
	proc_data[id].max_num = -1;
	proc_data[id].mode = -1;
	proc_data[id].sorted = 0;
	proc_data[id].curr_read = 0;
	printk(KERN_INFO "pfs: closing device for process %d\n", pid);
	return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	char temp[100];
	printk(KERN_INFO "pfs: reading data for process %d\n", pid);
	if(proc_data[id].curr_write != proc_data[id].max_num)
		return -EACCES;
	else if(proc_data[id].curr_read == proc_data[id].max_num)
	{
		printk(KERN_ALERT "pfs: No more data to read for process %d\n", pid);
		return -1;
	}
	else
	{
		if(proc_data[id].mode == 0)
			int_to_char(temp, proc_data[id].arr[proc_data[id].curr_read]);
		else
			strcpy(temp, proc_data[id].data[proc_data[id].curr_read]);
		proc_data[id].curr_read++;
	}
	ret = copy_to_user(bufStoreData, temp, bufsize);
	return 1;
}

ssize_t device_write(struct file *filp, const char* bufSourceData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	int cn = proc_data[id].curr_write;
	char temp[100];
	ret = copy_from_user(temp, bufSourceData, bufsize);

	if(cn == -1)
	{
		if(((unsigned char)temp[0]) == 0xFF)
			proc_data[id].mode = 0; 
		else if(((unsigned char)temp[0]) == 0xF0)
			proc_data[id].mode = 1;
		else
		{ 
			printk(KERN_ALERT "pfs: LKM Left Uninitialised for process %d\n", pid);
			return -EINVAL;
		} 
		// 	printk(KERN_ALERT "pfs: wrong mode\n");
		if(temp[1] > 100 || temp[1] < 1)
		{
			printk(KERN_ALERT "pfs: LKM Left Uninitialised for process %d\n", pid);
			return -EINVAL;
		}
		proc_data[id].curr_write = 0;
		proc_data[id].max_num = temp[1]; 
	}
	else if(cn == proc_data[id].max_num)
		return 0;
	else
	{
		if(proc_data[id].mode == 0)
			proc_data[id].arr[proc_data[id].curr_write] = char_to_int(temp);
		else
			strcpy(proc_data[id].data[proc_data[id].curr_write], temp);
		
		proc_data[id].curr_write++;
		ret = proc_data[id].max_num - proc_data[id].curr_write;
		printk(KERN_INFO "pfs: data written to process %d and objects remaining are %d\n", pid, ret);
	}

	if(!proc_data[id].sorted && proc_data[id].curr_write == proc_data[id].max_num)
	{
		sort_data(id);
		printk(KERN_INFO "pfs: data sorted for process %d\n", pid);
	}
	
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

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
module_init(driver_entry);
module_exit(driver_exit);