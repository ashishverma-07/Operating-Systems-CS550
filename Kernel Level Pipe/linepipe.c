#include<linux/miscdevice.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<linux/errno.h>
#include<asm/uaccess.h>
#include<linux/slab.h>
#include<linux/sched.h>
#include<linux/types.h>

MODULE_DESCRIPTION("Misc Character Driver for IPC");
MODULE_AUTHOR("Ashish Verma");
MODULE_LICENSE("GPL");


#define BUF_SIZE 100

typedef struct CQueue{
	unsigned char** buf;
	int size;
	int front;
	int rear;
}CQueue;

void initCQ(CQueue* q, unsigned char** buf, int size){
	q->buf = buf;
	q->size = size;
	q->front = -1;
	q->rear = -1;
}

int enQ(CQueue* q, unsigned char* value){
	if ((q->rear == q->size-1) || (q->front == q->rear+1))
		return 0;
	if ((q->rear == q->size-1) && (q->front != 0))
		q->rear = -1;
	q->rear++;
	memcpy(q->buf[q->rear], value, BUF_SIZE);
	if (q->front == -1)
		q->front = 0;
	return 1;
}

int deQ(CQueue* q, unsigned char* value){
	if (value == NULL || (q->front == -1 && q->rear == -1))
		return 0;
	memcpy(value, q->buf[q->front++], BUF_SIZE);
	if (q->front == q->size)
		q->front = 0;
	if (q->front-1 == q->rear){
		q->front = -1;
		q->rear = -1;
	}
	return 1;
}

static CQueue queue;
static unsigned char** qbuf;
static int qsize;

//Get the module parameter from user
module_param(qsize,int,S_IRUGO);
static struct semaphore empty;
static struct semaphore full;
static struct semaphore mutex;
int status = 0;
int __init mydevice_init(void);
void __exit mydevice_exit(void);
//open file operation
static int mydevice_processlist_open(struct inode *inode,struct file *filep)
{	
	pr_info("File open \n");
        return 0;
}
//close file operation
static int mydevice_processlist_close(struct inode *inodep,struct file *filep)
{
	pr_info("File close \n");
        return 0;
}
//read file operation
static ssize_t mydevice_processlist_read(struct file *filep, unsigned char *buffer, size_t len, loff_t *offset) 
{
	int status;
	//down operation on full	
	if((down_interruptible(&full))==0)
	{
		//down operation on mutex
		if((down_interruptible(&mutex))==0)
		{
			unsigned char buf[BUF_SIZE];
			if (deQ(&queue, buf))
			{
				status = copy_to_user(buffer, buf, len<BUF_SIZE? len : BUF_SIZE);
				if(status !=0)
				{
					pr_err("Failed to copy to user \n");
					return -EINVAL;
				}
			}
			//up operation on mutex
			up(&mutex);
		}
		//up operation on empty
		up(&empty);
	}
	return len;
}
//write file operation
static ssize_t mydevice_processlist_write(struct file *filep,const unsigned char *buffer, size_t len, loff_t *offset) 
{
	int ret;
	//down operation on empty	
	if((down_interruptible(&empty))==0)
	{
		//down operation on mutex
		if((down_interruptible(&mutex))==0)
		{
			unsigned char buf[BUF_SIZE];
			ret = copy_from_user(buf, buffer, len<BUF_SIZE? len : BUF_SIZE);
			if(ret !=0 || !enQ(&queue, buf))
			{
				pr_err("Failed to copy from user \n");
				return -EINVAL;
			}
			
			//up operation on mutex	
			up(&mutex);
		}
		//up operation on full
		up(&full);
	}	
	return len;
}
//file operation initialization for module
static const struct file_operations mydevice_processlist_fops = {
        .owner = THIS_MODULE,
        .open = mydevice_processlist_open,
        .release = mydevice_processlist_close,
	.read = mydevice_processlist_read,
	.write = mydevice_processlist_write,
};
//device driver registration
static struct miscdevice mydevice_processlist_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "linepipe",
        .fops = &mydevice_processlist_fops,
};
//init module for device driver
int __init mydevice_init(void)
{
	int value;
	int i=0;
	value = misc_register(&mydevice_processlist_device);
        if(value)
        {
                pr_err("Device can't be register: \n");
                return value;
        }
	//initialization of semaphore
	sema_init(&full, 0);
	sema_init(&empty, qsize);
	sema_init(&mutex, 1);
	//2-D char array initialization
	qbuf = (unsigned char**)kmalloc(sizeof(unsigned char*)*qsize, GFP_KERNEL);
	
	if(!qbuf){
		value = -ENOMEM;
		goto fail;
	}
	
	for(i=0;i<qsize;i++)
	{
	  qbuf[i] = (char *)kmalloc(sizeof(char) * BUF_SIZE, GFP_KERNEL);
	   memset(qbuf[i], 0, sizeof(unsigned char)*BUF_SIZE);
	}
	
	initCQ(&queue, qbuf, qsize);
	
	pr_info("Device register \n");
        
	return 0;
	fail:
	     mydevice_exit();
	     return value;

}
//exit module for device driver
void __exit mydevice_exit(void)
{
	misc_deregister(&mydevice_processlist_device);
	if(qbuf){
		int i=0;
		for(; i<qsize; i++)
		{
		   kfree(qbuf[i]);
		}
		kfree(qbuf);
	}
	pr_info("Device unregister");
}
module_init(mydevice_init);
module_exit(mydevice_exit);
