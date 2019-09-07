#include<stdio.h>

int main()
{
	char c[10];
	c[0] = '\0';
	printf("%s\n", c);
	// c = "jhumma";
	c[0] = 100;
	int i = c[0];
	printf("%d\n", i);
	c[1] = 'b';
	c[2] = '\0'; 
	printf("%s\n", c);
	printf("Yo yo\n");
	return 0;
}