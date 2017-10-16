#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 1024               

struct proc_info {
	pid_t pid;
	pid_t ppid;
	unsigned int cpu;
	char state[40];
};


static struct proc_info receive[BUFFER_LENGTH]; 


int main(){
   int ret, fd, i;
   char stringToSend[BUFFER_LENGTH];
   fd = open("/dev/process_list", O_RDWR);            
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }


   printf("Reading from the device...\n");
   
	ret = read(fd, receive, BUFFER_LENGTH * sizeof(struct proc_info));
           
		if (ret < 0){
			perror("Failed to read the message from the device.");
			return errno;
		}
		for(i = 0; i < ret; i++)
		{
			printf("PID=%d PPID=%d CPU=%d STATE=%s\n", receive[i].pid, receive[i].ppid, receive[i].cpu, receive[i].state);
		}
   return 0;
}