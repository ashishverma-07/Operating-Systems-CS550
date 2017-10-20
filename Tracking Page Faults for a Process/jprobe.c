#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <linux/memory.h> 
#include <linux/memcontrol.h> 
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */
#include <linux/slab.h> 

#define BUF_SIZE	512
#define PROC_NAME "page_faults"

	int pos=0;
	pid_t process_id= 1;
	module_param(process_id, int, 0);

typedef struct buffer {
	unsigned long address;
	long time;
} buffer_t;

buffer_t buf[BUF_SIZE];
	
static struct jprobe my_jprobe = {
	.entry = handle_mm_fault,
	.kp = {
		.symbol_name    = "handle_mm_fault",
	},
};

static int open_callback(struct inode *inod,struct file *fil);
static int read_callback(struct file *filp,buffer_t *out,size_t count,loff_t *offp );
static int release_callback(struct inode *inod,struct file *fil);

struct file_operations f_op = {
	.owner = THIS_MODULE,
	.read= read_callback,
	.open = open_callback,
	.release = release_callback
};

static int read_callback(struct file *filp, buffer_t *out,size_t count, loff_t *offp) 
{
	int ret = 0, i = 0;
	for (i = 0; i < BUF_SIZE; i++) {
		ret = copy_to_user(&out[i], &buf[i], sizeof(buffer_t));
		if(ret < 0) {
			printk(KERN_ERR "copy_to_user failed\n");
			return -1;
		}
	}
	return ret;
}

static int open_callback(struct inode *inodp, struct file *filp)
{
	printk(KERN_INFO "Tracking Page Faults for a Process was opened.\n");		
	return 0;
}

static int release_callback(struct inode *inodp, struct file *filp)
{
	printk(KERN_INFO "Tracking Page Faults for a Process was closed.\n");
	return 0;
}

static int __init jprobe_init(void)
{
	int ret;
	struct proc_dir_entry *proc_file;
	
	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_ERR "register_jprobe failed, returned %d.\n", ret);
		return -1;
	}
	printk(KERN_INFO "jprobe was successfully registered at %p.\n",my_jprobe.kp.addr);
	proc_file = proc_create(PROC_NAME, 0, NULL, &f_op);
	if (proc_file == NULL) {
		printk(KERN_ERR "proc_create failed\n");
		return -1;
	}
	printk(KERN_INFO "Proc /proc/%s was successfully created.\n", PROC_NAME);
	return 0;
}
  
static void __exit jprobe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	printk(KERN_INFO "jprobe at %p was unregistered.\n", my_jprobe.kp.addr);
	remove_proc_entry(PROC_NAME, NULL);
	printk(KERN_INFO "/proc/%s removed.\n", PROC_NAME);
}

int handle_mm_fault(struct mm_struct *mm, struct vm_area_struct *vma,unsigned long address, unsigned int flags) {
  struct timespec c_time;//created a timespec variable to hold the current time
	printk(KERN_INFO "Current PID - %d\n",current->pid);
	if(process_id == current->pid) {
		c_time = current_kernel_time();
		buf[pos].address = address;
		buf[pos].time = c_time.tv_nsec;
		printk(KERN_INFO "Page fault for a process %d: address 0x%lx, time %ld\n",  current->pid,buf[pos].address,buf[pos].time);
		pos = (pos + 1) % BUF_SIZE;
	}	

	jprobe_return();
	return 0;
 }
 
 module_init(jprobe_init)
 module_exit(jprobe_exit)
 MODULE_LICENSE("GPL");
