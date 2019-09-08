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
#include<linux/slab.h>
#include<linux/errno.h>

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
	int i;
	for(i = 0; i<MAX_PROC; i++)
	{
		proc_data[i].ct = NULL;
		proc_data[i].it = NULL;
		proc_data[i].head_ct = NULL;
		proc_data[i].head_it = NULL;
		proc_data[i].pid = -1;
		proc_data[i].mode = -1;
		proc_data[i].tm = 'z';
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


int* ciaf_int(struct obj_info* temp, struct int_tree* node, struct int_tree* root)
{
	int *arr = kmalloc(3*sizeof(int), GFP_KERNEL); 
	int *l, *r;

	if(node == NULL)
	{
		arr[0] = 0; arr[1] = 0; arr[2] = 0;
		return arr;
	}
	l = ciaf_int(temp, node->left, root);
	r = ciaf_int(temp, node->right, root);

	if(node != root)
	{
		if(l[0]==0 && r[0]==0)
			temp->deg1cnt = temp->deg1cnt + 1;
		else if(l[0]!=0 && r[0]!=0)
			temp->deg3cnt = temp->deg3cnt + 1;
		else
			temp->deg2cnt = temp->deg2cnt + 1;
	}
	if(node == root)
	{
		if(l[0]!=0 && r[0]!=0)
			temp->deg2cnt = temp->deg2cnt + 1;
		else if( l[0]==0 || r[0]== 0)
			temp->deg1cnt = temp->deg1cnt + 1;
	}
	arr[0] = l[0] + r[0] + 1;
	arr[1] = min(l[1],r[1]) + 1;
	arr[2] = max(l[2],r[2]) + 1;
	return arr;
}

int* ciaf_str(struct obj_info* temp, struct char_tree* node, struct char_tree* root)
{
	int *arr = kmalloc(3*sizeof(int), GFP_KERNEL); 
	int *l, *r;

	if(node == NULL)
	{
		arr[0] = 0; arr[1] = 0; arr[2] = 0;
		return arr;
	}
	l = ciaf_str(temp, node->left, root);
	r = ciaf_str(temp, node->right, root);

	if(node != root)
	{
		if(l[0]==0 && r[0]==0)
			temp->deg1cnt = temp->deg1cnt + 1;
		else if(l[0]!=0 && r[0]!=0)
			temp->deg3cnt = temp->deg3cnt + 1;
		else
			temp->deg2cnt = temp->deg2cnt + 1;
	}
	if(node == root)
	{
		if(l[0]!=0 && r[0]!=0)
			temp->deg2cnt = temp->deg2cnt + 1;
		else if( l[0]==0 || r[0]== 0)
			temp->deg1cnt = temp->deg1cnt + 1;
	}
	arr[0] = l[0] + r[0] + 1;
	arr[1] = min(l[1],r[1]) + 1;
	arr[2] = max(l[2],r[2]) + 1;
	return arr;
}	


int calculate_info(struct obj_info* temp, int id)
{
	int *arr;
	if(proc_data[id].mode == 0)
		arr = ciaf_int(temp, proc_data[id].it, proc_data[id].it);
	else
		arr = ciaf_str(temp, proc_data[id].ct, proc_data[id].ct);
	temp->mindepth = arr[1]-1;
	temp->maxdepth = arr[2]-1;
	return max(arr[0],0);
}

void foaf_int(struct search_obj* temp, struct int_tree* root)
{
	temp->found = 0;
	while(root!=NULL)
	{
		if(root->node == temp->int_obj)
		{
			temp->found = 1;
			break;
		}
		else if(temp->int_obj > root->node)
			root = root->right;
		else
			root = root->left;
	}
}

void foaf_str(struct search_obj* temp, struct char_tree* root)
{
	temp->found = 0;
	while(root!=NULL)
	{
		if(strcmp(root->node,temp->str) == 0)
		{
			temp->found = 1;
			break;
		}
		else if(strcmp(temp->str, root->node) == 1)
			root = root->right;
		else
			root = root->left;
	}
}

void find_obj(struct search_obj* temp, int id)
{
	if(proc_data[id].mode == 0)
	{
		if(temp->objtype == 0xFF)
			foaf_int(temp, proc_data[id].it);
		else
			temp->found = 0;
	}
	else
	{
		if(temp->objtype == 0xF0)
			foaf_str(temp, proc_data[id].ct);
		else
			temp->found = 0;
	}
}

void inorder_int(struct int_tree* root, struct int_tree** head)
{
	if(root==NULL)
		return;
	inorder_int(root->left, head);
	struct int_tree* temp = kmalloc(sizeof(struct int_tree), GFP_KERNEL);
	temp->node = root->node;
	temp->left = NULL;
	if(*head==NULL)
		*head = temp;
	else
	{
		struct int_tree* last = *head;
		while(last->left)
			last = last->left;
		last->left = temp;
	}
	inorder_int(root->right, head);
}

void inorder_str(struct char_tree* root, struct char_tree** head)
{
	if(root==NULL)
		return;
	inorder_str(root->left, head);
	struct char_tree* temp = kmalloc(sizeof(struct char_tree), GFP_KERNEL);
	strcpy(temp->node ,root->node);
	temp->left = NULL;
	if(*head==NULL)
		*head = temp;
	else
	{
		struct char_tree* last = *head;
		while(last->left)
			last = last->left;
		last->left = temp;
	}
	inorder_str(root->right, head);
}

void preorder_int(struct int_tree* root, struct int_tree** head)
{
	if(root==NULL)
		return;
	struct int_tree* temp = kmalloc(sizeof(struct int_tree), GFP_KERNEL);
	temp->node = root->node;
	temp->left = NULL;
	if(*head==NULL)
		*head = temp;
	else
	{
		struct int_tree* last = *head;
		while(last->left)
			last = last->left;
		last->left = temp;
	}
	preorder_int(root->left, head);
	preorder_int(root->right, head);
}

void preorder_str(struct char_tree* root, struct char_tree** head)
{
	if(root==NULL)
		return;
	struct char_tree* temp = kmalloc(sizeof(struct char_tree), GFP_KERNEL);
	strcpy(temp->node ,root->node);
	temp->left = NULL;
	if(*head==NULL)
		*head = temp;
	else
	{
		struct char_tree* last = *head;
		while(last->left)
			last = last->left;
		last->left = temp;
	}
	preorder_str(root->left, head);
	preorder_str(root->right, head);
}

void postorder_int(struct int_tree* root, struct int_tree** head)
{
	if(root==NULL)
		return;
	postorder_int(root->left, head);
	postorder_int(root->right, head);
	
	struct int_tree* temp = kmalloc(sizeof(struct int_tree), GFP_KERNEL);
	temp->node = root->node;
	temp->left = NULL;
	if(*head==NULL)
		*head = temp;
	else
	{
		struct int_tree* last = *head;
		while(last->left)
			last = last->left;
		last->left = temp;
	}
}

void postorder_str(struct char_tree* root, struct char_tree** head)
{
	if(root==NULL)
		return;
	postorder_str(root->left, head);
	postorder_str(root->right, head);	
	
	struct char_tree* temp = kmalloc(sizeof(struct char_tree), GFP_KERNEL);
	strcpy(temp->node ,root->node);
	temp->left = NULL;
	if(*head==NULL)
		*head = temp;
	else
	{
		struct char_tree* last = *head;
		while(last->left)
			last = last->left;
		last->left = temp;
	}
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

void insert_str(struct char_tree** root, char temp[])
{
	if(*root == NULL)
	{
		struct char_tree* t = kmalloc(sizeof(struct char_tree), GFP_KERNEL);
		strcpy(t->node, temp);
		t->left = t->right = NULL;
		*root = t;
		return;
	}
	else if(strcmp(temp, (*root)->node) == 1 || strcmp(temp, (*root)->node) == 0)
		insert_str(&((*root)->right), temp);
	else
		insert_str(&((*root)->left), temp);
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
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	proc_data[id].pid = -1;
	proc_data[id].ct = NULL;
	proc_data[id].it = NULL;
	proc_data[id].head_ct = NULL;
	proc_data[id].head_it = NULL;
	proc_data[id].pid = -1;
	proc_data[id].mode = -1;
	proc_data[id].tm = 'z';
	proc_data[id].curr_read = 0;
	printk(KERN_INFO "pfs: closing device for process %d\n", pid);
	return 0;
}

ssize_t device_read(struct file *filp, char* bufStoreData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	char temp[100];
	if(proc_data[id].tm == 'z')
		return -EACCES;
	printk(KERN_INFO "tree_pfs: reading data for process %d\n", pid);
	if(proc_data[id].tm == 'i')
	{
		if(proc_data[id].mode == 0)
		{
			if(proc_data[id].curr_read == 0)
			{
				inorder_int(proc_data[id].it, &(proc_data[id].head_it));
				printk(KERN_INFO "tree_pfs: inorder done for process %d\n", pid);
			}
			if(proc_data[id].head_it == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read for process %d\n", pid);
				return 0;
			}
			printk(KERN_INFO "tree_ pfs: value read is %d for process %d\n", (proc_data[id].head_it)->node, pid);
			int_to_char(temp, (proc_data[id].head_it)->node);
			proc_data[id].head_it = (proc_data[id].head_it)->left;
			proc_data[id].curr_read++;
		}
		else if(proc_data[id].mode == 1)
		{
			if(proc_data[id].curr_read == 0)
			{
				inorder_str(proc_data[id].ct, &(proc_data[id].head_ct));
				printk(KERN_INFO "tree_pfs: inorder done for process %d\n", pid);
			}
			if(proc_data[id].head_ct == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read for process %d\n", pid);
				return 0;
			}
			printk(KERN_INFO "tree_ pfs: value read is %s for process %d\n", (proc_data[id].head_ct)->node, pid);
			strcpy(temp, (proc_data[id].head_ct)->node);
			proc_data[id].head_ct = (proc_data[id].head_ct)->left;
			proc_data[id].curr_read++;
		}		
	}
	
	else if(proc_data[id].tm == 'p')
	{
		if(proc_data[id].mode == 0)
		{
			if(proc_data[id].curr_read == 0)
			{
				preorder_int(proc_data[id].it, &(proc_data[id].head_it));
				printk(KERN_INFO "tree_pfs: preorder done for process %d\n", pid);
			}
			if(proc_data[id].head_it == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read for process %d\n", pid);
				return 0;
			}
			printk(KERN_INFO "tree_ pfs: value read is %d for process %d\n", (proc_data[id].head_it)->node, pid);
			int_to_char(temp, (proc_data[id].head_it)->node);
			proc_data[id].head_it = (proc_data[id].head_it)->left;
			proc_data[id].curr_read++;
		}
		else if(proc_data[id].mode == 1)
		{
			if(proc_data[id].curr_read == 0)
			{
				preorder_str(proc_data[id].ct, &(proc_data[id].head_ct));
				printk(KERN_INFO "tree_pfs: preorder done for process %d\n", pid);
			}
			if(proc_data[id].head_ct == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read for process %d\n", pid);
				return 0;
			}
			printk(KERN_INFO "tree_ pfs: value read is %s for process %d\n", (proc_data[id].head_ct)->node, pid);
			strcpy(temp, (proc_data[id].head_ct)->node);
			proc_data[id].head_ct = (proc_data[id].head_ct)->left;
			proc_data[id].curr_read++;
		}		
	}
	
	else if(proc_data[id].tm == 's')
	{
		if(proc_data[id].mode == 0)
		{
			if(proc_data[id].curr_read == 0)
			{
				postorder_int(proc_data[id].it, &(proc_data[id].head_it));
				printk(KERN_INFO "tree_pfs: postorder done for process %d\n", pid);
			}
			if(proc_data[id].head_it == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read for process %d\n", pid);
				return 0;
			}
			printk(KERN_INFO "tree_ pfs: value read is %d for process %d\n", (proc_data[id].head_it)->node, pid);
			int_to_char(temp, (proc_data[id].head_it)->node);
			proc_data[id].head_it = (proc_data[id].head_it)->left;
			proc_data[id].curr_read++;
		}
		else if(proc_data[id].mode == 1)
		{
			if(proc_data[id].curr_read == 0)
			{
				postorder_str(proc_data[id].ct, &(proc_data[id].head_ct));
				printk(KERN_INFO "tree_pfs: postorder done for process %d\n", pid);
			}
			if(proc_data[id].head_ct == NULL)
			{
				printk(KERN_ALERT "tree_pfs: No more data to read for process %d\n", pid);
				return 0;
			}
			printk(KERN_INFO "tree_ pfs: value read is %s for process %d\n", (proc_data[id].head_ct)->node, pid);
			strcpy(temp, (proc_data[id].head_ct)->node);
			proc_data[id].head_ct = (proc_data[id].head_ct)->left;
			proc_data[id].curr_read++;
		}		
	}

	ret = copy_to_user(bufStoreData, temp, bufsize);
	return 1;
}

ssize_t device_write(struct file *filp, const char* bufSourceData, size_t bufsize, loff_t* curroffset)
{
	int pid = (int)task_pid_nr(current);
	int id = find_proc(pid);
	char temp[100];
	if(proc_data[id].tm == 'z')
		return -EACCES;
	ret = copy_from_user(temp, bufSourceData, bufsize);

	if(proc_data[id].mode == 0)
		insert_int(&proc_data[id].it, char_to_int(temp));
	else
		insert_str(&proc_data[id].ct, temp);
	proc_data[id].curr_read = 0;
	proc_data[id].head_it = NULL;
	proc_data[id].head_ct = NULL;
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
	int num_nodes;
	switch(cmd)
	{
		case PB2_SET_TYPE:
						copy_from_user(&t, (unsigned char *)arg, sizeof(t));
						proc_data[id].ct = NULL;
						proc_data[id].it = NULL;
						proc_data[id].head_ct = NULL;
						proc_data[id].head_it = NULL;
						proc_data[id].tm = 'i';
						proc_data[id].curr_read = 0;
						if(t==0xFF)
							proc_data[id].mode = 0;
						else if(t==0xF0)
							proc_data[id].mode = 1;
						else
							return -EINVAL;
						printk(KERN_INFO "tree_pfs: Mode changed to mode %c for process %d \n", t, pid);
						return 0;

		case PB2_SET_ORDER:
						if(proc_data[id].tm == 'z')
							return -EACCES;
						copy_from_user(&tm, (char *)arg, sizeof(tm));
						if(tm == 'i')
							proc_data[id].tm = 'i';
						else if(tm == 'p')
							proc_data[id].tm = 'p';
						else if(tm == 's')
							proc_data[id].tm = 's';
						else
							return -EINVAL;

						if(proc_data[id].mode == 0)
							proc_data[id].head_it = NULL;
						else
							proc_data[id].head_ct = NULL;
						proc_data[id].curr_read = 0;
						printk(KERN_INFO "tree_pfs: Traversal order set to %c for process %d\n", tm, pid);

						return 0; 

		case PB2_GET_INFO:
						if(proc_data[id].tm == 'z')
							return -EACCES;
						copy_from_user(&temp, (struct obj_info *)arg, sizeof(temp));
						temp.deg1cnt = temp.deg2cnt = temp.deg3cnt = temp.maxdepth = temp.mindepth = 0;
						num_nodes = calculate_info(&temp, id);
						printk(KERN_INFO "tree_pfs: info calculated for process %d\n", pid);
						copy_to_user((struct obj_info *)arg, &temp, sizeof(temp));
						return num_nodes;

		case PB2_GET_OBJ:
						if(proc_data[id].tm == 'z')
							return -EACCES;
						copy_from_user(&st, (struct search_obj *)arg, sizeof(st));
						find_obj(&st, id);
						copy_to_user((struct search_obj *)arg, &st, sizeof(st));
						printk(KERN_INFO "tree_pfs: Object find completed for process %d\n", pid);
						return 0;

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
