/* 
	Group - 10
	Vishal Gupta (15CS30039)
	Vishesh Agarwal (15CS30040)
	Kernel Version - 4.15.0-29-generic
*/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<string.h>

#define PB2_SET_TYPE _IOW(0x10, 0x31, int32_t*)
#define PB2_SET_ORDER _IOW(0x10, 0x32, int32_t*)
#define PB2_GET_INFO _IOR(0x10, 0x33, int32_t*)
#define PB2_GET_OBJ _IOR(0x10, 0x34, int32_t*)

#define FILE "/proc/partb_2_15CS30039"


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

int main(int argc, char const *argv[])
{
	int fd, i, N, ret, num;
	char ch, read_buf[100], order, mch;
	unsigned char mode, write_buf[100];

	fd = open(FILE, O_RDWR);		
	
	if(fd == -1) 
	{
		printf("file either doesn't exist or is locked by another process\n");
		return -1;
	}

	while(1)
	{
		printf("Want to sort integers or strings? \npress i for integers and s for strings\n");
		scanf("%c%*c", &mch);
		if(mch == 's')
		{ mode = 0xF0; break;}
		else if(mch == 'i')
		{ mode = 0xFF; break; }
		else
		{ printf("Enter a valid choice\n"); }
	}

	printf("Enter the order of traversal (i-> inorder, p->preorder, s->postorder)\n");
	scanf("%c%*c", &order);

	// write_buf[0] = mode;
	// write_buf[1] = N;
	// write_buf[2] = '\0';

	// write(fd, write_buf, sizeof(write_buf));
	ioctl(fd, PB2_SET_TYPE, (int32_t *)&mode);
	ioctl(fd, PB2_SET_ORDER, (int32_t *)&order);

	struct obj_info oi;
	struct search_obj so;
	ch = 'r';
	while(ch != 'e')
	{
		printf("\nr: read from device, w: write to device, t: set type,\ns: set order, g: get info, o: get obj, e: exit \nenter command\n");
		scanf("%c%*c", &ch);
		switch(ch)
		{
			case 'w':
					printf("enter object\n");
					scanf("%[^\n]%*c", write_buf);
					// printf("gonna write it\n");
					ret = write(fd, write_buf, sizeof(write_buf));
					break;

			case 'r':
					ret = read(fd, read_buf, sizeof(read_buf));
					if(ret <= 0)
						printf("No more data to read\n");
					else
						printf("%s\n", read_buf);
					break;

			case 't':
					printf("Enter s for strings and i for integers\n");
					scanf("%c%*c", &mch);
					if(mch == 's')
						mode = 0xF0;
					else if(mch == 'i')
						mode = 0xFF;
					else
						printf("Enter a valid choice next time");
					if(mch == 's' || mch =='i')
						ioctl(fd, PB2_SET_TYPE, (int32_t *)&mode);
					break;

			case 's':
					printf("Enter the order of traversal (i-> inorder, p->preorder, s->postorder)\n");
					scanf("%c%*c", &order);
					if(order == 'i' || order == 'p' || order == 's')
						ioctl(fd, PB2_SET_ORDER, (int32_t *)&order);
					else
						printf("Enter a valid choice next time");
					break;

			case 'g':
					num = ioctl(fd, PB2_GET_INFO, (int32_t *)&oi);
					printf("Number of nodes with degree 1 in BST : %d\n", oi.deg1cnt);
					printf("Number of nodes with degree 2 in BST : %d\n", oi.deg2cnt);
					printf("Number of nodes with degree 3 in BST : %d\n", oi.deg3cnt);
					printf("Minimum height of BST : %d\n", oi.mindepth);
					printf("MAximum height of BST : %d\n", oi.maxdepth);
					printf("Number of elements in BST : %d\n", num);
					break;

			case 'o':
					printf("Want to search for string or int objects? type s for strings and i for integers\n");
					scanf("%c%*c", &mch);
					if(mch == 's')
						mode = 0xF0;
					else if(mch == 'i')
						mode = 0xFF;
					else
						printf("Enter a valid choice next time");
					if(mch == 's' || mch =='i')
					{
						so.objtype = mode;
						printf("Enter the object to search\n");
						if(mch == 'i')
							scanf("%d%*c", &(so.int_obj));
						else
							{ scanf("%[^\n]%*c", so.str); so.len = strlen(so.str);}
						ioctl(fd, PB2_GET_OBJ, (int32_t *)&so);
					}
					printf("Object status: found = %d\n", so.found);
					break;
			case 'e':
					printf("Exiting process\n");
					break;
			default:
					printf("Command not recognized\n");
					break;
		}

	}

	close(fd);
	return 0;
}