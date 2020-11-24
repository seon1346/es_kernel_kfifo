#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char * argv[]) {

	char *arg = argv[1];
	int i = atoi(arg);
	int f = open("/dev/BufferedMem", O_RDWR);
	if (f == -1)
		printf("file open error\n");
	ioctl(f, _IOR('Q', 1, int), &i);



	return 0;

}