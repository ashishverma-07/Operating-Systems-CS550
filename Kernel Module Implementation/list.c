#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#define  DEVICE_NAME "process_list"
#define  CLASS_NAME  "process_class"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("List all processes");  
MODULE_VERSION("0.1");            

struct proc_info {
	pid_t pid;
	pid_t ppid;
	unsigned int cpu;
	char state[40];
};

static int    majorNumber;
static struct proc_info procs[1024];
static int size_of_proces = 0; 
static struct class*  proclistClass  = NULL;
static struct device* proclistDevice = NULL;

static int     dev_open(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static const char *get_task_state(struct task_struct *);


static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
};


static int __init process_list_init(void){

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "process_list failed to register a major number\n");
      return majorNumber;
   }

   proclistClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(proclistClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(proclistClass);          // Correct way to return an error on a pointer
   }

   proclistDevice = device_create(proclistClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(proclistDevice)){               // Clean up if there is an error
      class_destroy(proclistClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(proclistDevice);
   }
   return 0;
}


static void __exit process_list_exit(void){
   device_destroy(proclistClass, MKDEV(majorNumber, 0));     
   class_unregister(proclistClass);                          
   class_destroy(proclistClass);                             
   unregister_chrdev(majorNumber, DEVICE_NAME);             
}


static const char *task_state_array[] = {
	"TASK_RUNNING",
	"TASK_INTERRUPTIBLE",
	"TASK_UNINTERRUPTIBLE",
	"__TASK_STOPPED",
	"__TASK_TRACED",
	"EXIT_DEAD",
	"EXIT_ZOMBIE",
	"TASK_DEAD",
	"TASK_WAKEKILL",
	"TASK_WAKING",
};

static const char *get_task_state(struct task_struct *tsk)
{
	unsigned int state = tsk->state;
	const char **p = &task_state_array[0];
	if (state == (TASK_WAKEKILL | TASK_UNINTERRUPTIBLE))
		return "TASK_WAKEKILL, TASK_UNINTERRUPTIBLE";
	while (state) {
		p++;
		state >>= 1;
	}
	return *p;
}


static int dev_open(struct inode *inodep, struct file *filep){
	struct task_struct *g;
	size_of_proces = 0;

	rcu_read_lock();
    for_each_process(g) {
		procs[size_of_proces].pid = g->pid;
		procs[size_of_proces].ppid = g->real_parent->pid;
		procs[size_of_proces].cpu = task_cpu(g);
		strcpy(procs[size_of_proces].state, get_task_state(g));
		size_of_proces++;
    }
    rcu_read_unlock();
	return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
	int error_count = 0;

   error_count = copy_to_user(buffer, procs, size_of_proces * sizeof(struct proc_info));

   if (error_count==0){            
      return (size_of_proces); 
   }
   else {
      printk(KERN_INFO "Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              
   }
}

module_init(process_list_init);
module_exit(process_list_exit);