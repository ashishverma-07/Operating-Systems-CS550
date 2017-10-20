#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#define BUF_SIZE 512

typedef struct buffer {
	unsigned long address;
	long time;
} buffer_t;

int main()
{
	buffer_t buf[BUF_SIZE];
	int fd, i = 0;
	int bytes_read = 0;
	fd = open("/proc/page_faults",O_RDONLY);
	bytes_read = read(fd, buf, 1);
	printf("Tracking Page Faults for a Process:\n");	
	for (i = 0; i < BUF_SIZE; i++) {
		printf("0x%lx  :  %ld\n", buf[i].address, buf[i].time);		
	}
	close(fd);
	return 0;
}	
