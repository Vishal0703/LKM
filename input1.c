#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>

// #define FILE "/dev/mytestdevice"
#define FILE "/proc/partb_1_15CS30039"


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

	printf("Enter the number of objects to sort\n");
	scanf("%d%*c", &N);

	write_buf[0] = mode;
	write_buf[1] = N;
	write_buf[2] = '\0';

	write(fd, write_buf, sizeof(write_buf));
	
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
					ret = read(fd, read_buf, sizeof(read_buf));
					if(ret <= 0)
						printf("No more objects to read\n");
					else
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