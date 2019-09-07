#include<stdio.h>
#include<string.h>

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

int main()
{
	// char c[10], d[10];
	// c[0] = 'a';
	// c[1] = 'b';
	// c[2] = '\0'; 
	// printf("%s\n", c);
	// strcpy(d,c);
	// printf("%s\n",d);


	// char temp[100];
	// temp[0] = '1';
	// temp[1] = '3';
	// temp[2] = '\0';

	// int i = char_to_int(temp);
	// printf("i = %d\n", i);
	// char temp2[100];
	// int_to_char(temp2, i);

	// printf("%s\n", temp2);

	unsigned char c = 0xFF;
	printf("%d\n", c);
	return 0;
}