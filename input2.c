#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>

#define PB2_SET_TYPE _IOW(0x10, 0x31, int32_t*)
#define PB2_SET_ORDER _IOW(0x10, 0x32, int32_t*)
#define PB2_GET_INFO _IOR(0x10, 0x33, int32_t*)
#define PB2_GET_OBJ _IOR(0x10, 0x34, int32_t*)

#define FILE "/proc/partb_2_15CS30039"

int main(int argc, char const *argv[])
{
	int fd, i, N, ret;
	char ch, read_buf[100];
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
		scanf("%c%*c", &ch);
		if(ch == 's')
		{ mode = 0xF0; break;}
		else if(ch == 'i')
		{ mode = 0xFF; break; }
		else
		{ printf("Enter a valid choice\n"); }
	}

	// printf("Enter the number of objects to sort\n");
	// scanf("%d%*c", &N);

	// write_buf[0] = mode;
	// write_buf[1] = N;
	// write_buf[2] = '\0';

	// write(fd, write_buf, sizeof(write_buf));
	ioctl(fd, PB2_SET_TYPE, (int32_t *)&mode);

	ch = 'r';
	while(ch != 'e')
	{
		printf("\nr: read from device, w: write to device, e: exit \nenter command\n");
		scanf("%c%*c", &ch);
		switch(ch)
		{
			case 'w':
					printf("enter object\n");
					scanf("%[^\n]%*c", write_buf);
					// printf("gonna write it\n");
					ret = write(fd, write_buf, sizeof(write_buf));
					printf("%d more objects to write\n",ret);
					break;
			case 'r':
					read(fd, read_buf, sizeof(read_buf));
					printf("%s\n", read_buf);
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