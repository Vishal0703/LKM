#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/string.h>
#include<linux/sort.h>
#include<linux/slab.h>

#define MAX_PROC 100
#define FILE_NAME "partb_2_15CS30039"
#define MAX_INP
#define DRIVER_AUTHOR "15CS30039"

#define PB2_SET_TYPE _IOW(0x10, 0x31, int32_t*)
#define PB2_SET_ORDER _IOW(0x10, 0x32, int32_t*)
#define PB2_GET_INFO _IOR(0x10, 0x33, int32_t*)
#define PB2_GET_OBJ _IOR(0x10, 0x34, int32_t*)

struct int_tree
{
	int node;
	struct int_tree *left;
	struct int_tree *right;
};

struct char_tree
{
	char node[100];
	struct char_tree *left;
	struct char_tree *right;
};

struct data_holder
{
	struct char_tree* ct;
	struct int_tree* it;
	struct char_tree* head_ct;
	struct int_tree* head_it;
	struct char_tree* tail_ct;
	struct int_tree* tail_it;
	int pid;
	int mode;
	char tm;
	int curr_read;
};

struct obj_info 
{
	 int32_t deg1cnt; //number of nodes with degree 1 (in or out)
	 int32_t deg2cnt; //number of nodes with degree 2 (in or out)
	 int32_t deg3cnt; //number of nodes with degree 3 (in or out)
	 int32_t maxdepth;//maximum number of intermediate nodes from root to a leaf
	 int32_t mindepth;//minimum number of intermediate nodes from root to a leaf
};

struct search_obj 
{
	 unsigned char objtype; // either 0xFF or 0xF0 represent integer or string
	 char found; // if found==0 then found else no found
	 int32_t int_obj; // value of integer object. valid only if objtype == 0xFF
	 char str[100]; // value of string object. valid only if objtype == 0xF0
	 int32_t len; // length of string object. valid only if objtype == 0xF0
};

struct data_holder proc_data[MAX_PROC];
int ret;
static struct proc_dir_entry *OPF;


void initialize_data(void)
{
	int i,j;
	for(i = 0; i<MAX_PROC; i++)
	{
		proc_data[i].ct = NULL;
		proc_data[i].it = NULL;
		proc_data[i].head_ct = NULL;
		proc_data[i].head_it = NULL;
		proc_data[i].tail_ct = NULL;
		proc_data[i].tail_it = NULL;
		proc_data[i].pid = -1;
		proc_data[i].mode = -1;
		proc_data[i].tm = 'i';
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

void calculate_info(struct obj_info* temp)
{

}


void inorder_int(struct int_tree* root, struct int_tree** head)
{
	if(root==NULL)
	{
		printk(KERN_INFO "root null\n");
		return;
	}
	inorder_int(root->left, head);
	struct int_tree* temp = kmalloc(sizeof(struct int_tree), GFP_KERNEL);
	temp->node = root->node;
	temp->left = NULL;
	if(*head==NULL)
	{
		*head = temp;
		printk(KERN_INFO "Head allocated with value %d\n", (*head)->node);
	}
	else
	{
		struct int_tree* last = *head;
		while(last->left)
		{
			printk(KERN_INFO "value is : %d\n", last->node);
			last = last->left;
		}
		last->left = temp;
	}
	inorder_int(root->right, head);
}

void insert_int(struct int_tree** root, int val)
{
	if(*root == NULL)
	{
		struct int_tree* t = kmalloc(sizeof(struct int_tree), GFP_KERNEL);
		t->node = val;
		t->left = t->right = NULL;
		*root = t;
		return;
	}
	else if(val >= (*root)->node)
		insert_int(&((*root)->right), val);
	else
		insert_int(&((*root)->left), val);
}

void insert_str(struct char_tree* root, char temp[])
{
	if(root == NULL)
	{
		struct char_tree* t = kmalloc(sizeof(struct char_tree), GFP_KERNEL);
		strcpy(t->node, temp);
		root = t;
		return;
	}
	else if(strcmp(temp, root->node) == 1 || strcmp(temp, root->node) == 0)
		insert_str(root->right, temp);
	else
		insert_str(root->left, temp);
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

// void sort_data(int id)
// {
// 	if(proc_data[id].mode == 0)
// 		sort(proc_data[id].arr, proc_data[id].max_num, sizeof(int), &compare_int, NULL);
// 	else
// 		sort(proc_data[id].data, proc_data[id].max_num, sizeof(char)*100, &compare_str, NULL);
// }

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
	proc_data[id].ct = NULL;
	proc_data[id].it = NULL;
	proc_data[id].head_ct = NULL;
	proc_data[id].head_it = NULL;
	proc_data[id].tail_ct = NULL;
	proc_data[id].tail_it = NULL;
	proc_data[id].pid = -1;
	proc_data[id].mode = -1;
	proc_data[id].tm = 'i';
	proc_data[id].curr_read = 0;
	printk(KERN_INFO "pfs: closing device for process %d\n", pid);
	return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	char temp[100];
	struct int_tree* l = NULL;
	printk(KERN_INFO "tree_pfs: reading data for process %d\n", pid);
	if(proc_data[id].tm == 'i')
	{
		if(proc_data[id].mode == 0 && proc_data[id].curr_read == 0)
		{
			inorder_int(proc_data[id].it, &(proc_data[id].head_it));
			printk(KERN_INFO "tree_pfs: inorder done\n");
			if(proc_data[id].head_it == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read\n");
				return -1;
			}
			printk(KERN_INFO "tree_ pfs: value read is %d\n", (proc_data[id].head_it)->node);
			int_to_char(temp, (proc_data[id].head_it)->node);
			proc_data[id].head_it = (proc_data[id].head_it)->left;
			proc_data[id].curr_read++;
		}
		else if(proc_data[id].mode == 0 && proc_data[id].curr_read != 0)
		{
			if(proc_data[id].head_it == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read\n");
				return -1;
			}
			printk(KERN_INFO "tree_ pfs: value read is %d\n", (proc_data[id].head_it)->node);
			int_to_char(temp, proc_data[id].head_it->node);
			proc_data[id].head_it = (proc_data[id].head_it)->left;
		}
	}
	// else if(proc_data[id].tm == 'p')
	// 	// preorder();
	// else
	// 	// postorder();
	ret = copy_to_user(bufStoreData, temp, bufsize);
	// if(proc_data[id].curr_write != proc_data[id].max_num)
	// 	return -1;
	// else if(proc_data[id].curr_read == proc_data[id].max_num)
	// {
	// 	printk(KERN_ALERT "pfs: No more data to read for process %d\n", pid);
	// 	return -1;
	// }
	// else
	// {
	// 	if(proc_data[id].mode == 0)
	// 		int_to_char(temp, proc_data[id].arr[proc_data[id].curr_read]);
	// 	else
	// 		strcpy(temp, proc_data[id].data[proc_data[id].curr_read]);
	// 	proc_data[id].curr_read++;
	// }
	// ret = copy_to_user(bufStoreData, temp, bufsize);
	// return ret;
}

ssize_t device_write(struct file *filp, const char* bufSourceData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	char temp[100];
	ret = copy_from_user(temp, bufSourceData, bufsize);

	if(proc_data[id].mode == 0)
		insert_int(&proc_data[id].it, char_to_int(temp));
	else
		insert_str(proc_data[id].ct, temp);
	
	printk(KERN_INFO "tree_pfs: node insertion comlete for process %d\n", pid);
	return ret;
}


static long device_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	unsigned char t;
	char tm;
	struct obj_info temp;
	struct search_obj st;
	switch(cmd)
	{
		case PB2_SET_TYPE:
						copy_from_user(&t, (unsigned char *)arg, sizeof(t));
						proc_data[id].ct = NULL;
						proc_data[id].it = NULL;
						proc_data[id].head_ct = NULL;
						proc_data[id].head_it = NULL;
						proc_data[id].pid = pid;
						proc_data[id].tm = 'i';
						proc_data[id].curr_read = 0;
						if(t==0xFF)
							proc_data[id].mode = 0;
						else if(t==0xF0)
							proc_data[id].mode = 1;
						else
							return -1;
						return 0;

		case PB2_SET_ORDER:
						copy_from_user(&tm, (char *)arg, sizeof(tm));
						if(tm == '0')
							proc_data[id].tm = 'i';
						else if(tm == '1')
							proc_data[id].tm = 'p';
						else if(tm == '2')
							proc_data[id].tm = 's';
						else
							return -1;

						if(proc_data[id].mode == 0)
							proc_data[id].head_it = proc_data[id].it;
						else
							proc_data[id].head_ct = proc_data[id].ct;

						return 0; 

		case PB2_GET_INFO:
						copy_from_user(&temp, (struct obj_info *)arg, sizeof(temp));
						// calculate_info(temp);
						copy_to_user((struct obj_info *)arg, &temp, sizeof(temp));

		case PB2_GET_OBJ:
						copy_from_user(&st, (struct search_obj *)arg, sizeof(st));
						// find_obj(st);
						copy_to_user((struct search_obj *)arg, &st, sizeof(st));


	}
}



struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl
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