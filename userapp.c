#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

// #define FILE "/dev/mytestdevice"
#define FILE "/proc/partb_1_15CS30039"

int main(int argc, char const *argv[])
{
	int fd;
	char ch, write_buf[100], read_buf[100];

	fd = open(FILE, O_RDWR);	
	// printf("fd is %d\n", fd);	
	if(fd == -1)
	{
		printf("file either doesn't exist or is locked by another process\n");
		return -1;
	}
	
	char more = 'y'; 
	while(more == 'y')
	{
		printf("r: read from device\n w: write to device\n enter command\n");
		scanf("%c%*c", &ch);
		switch(ch)
		{
			case 'w':
					printf("enter data\n");
					scanf("%[^\n]%*c", write_buf);
					// printf("gonna write it\n");
					write(fd, write_buf, sizeof(write_buf));
					break;
			case 'r':
					read(fd, read_buf, sizeof(read_buf));
					printf("device : %s\n", read_buf);
					break;
			default:
					printf("Command not recognized\n");
					break;
		}
		printf("Wanna do more ops? -> (y/n)\n");
		fflush(stdin);

		scanf("%c%*c", &more);
	}
	close(fd);
	return 0;
}