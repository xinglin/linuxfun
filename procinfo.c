#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/current.h>
#include <linux/sched.h>
#include <linux/cred.h>

#include <linux/mm.h>
#include <linux/highmem.h>

#include <linux/err.h>

#define DRIVER_AUTHOR "Xing Lin@utah"
#define DRIVER_DESC   "Have fun with Linux kernels"
#define proc_fn       "havefun"

extern struct page *mem_map;
extern unsigned long num_physpages;
extern unsigned long max_mapnr;
static unsigned long offset = 0;
struct task_struct *curproc = NULL;
char debug = 0;
/* This function is called at the beginning of a sequence.
 * ie, when:
 *	- the /proc file is read (first time)
 *	- after the function stop (end of sequence)
 *  pos is the position in /proc/filename. 
 */
static void *
de_seq_start(struct seq_file *s, loff_t * pos)
{
	curproc = current;
	seq_printf(s, "current pid: %d\n", curproc->pid);
	seq_printf(s, "ID\tUID\tPID\tcommand\n");
	offset = *(unsigned long *) pos;
	if (offset == 0) {
		/* begin the sequence. */
		return curproc;
	} else {
		printk(KERN_INFO "end of sequence %8lu\n", offset);
		*pos = 0;
		offset = 0;
		return NULL;
	}
};

/*
 * This function is called after the beginning of a sequence.
 * It's called until the return is NULL (this ends the sequence).
 */
static void *
de_seq_next(struct seq_file *s, void *v, loff_t * pos)
{
	*pos += 1;
	offset = *(unsigned long *) pos;
	curproc = next_task(curproc);
	if ( curproc == current ) {
		printk(KERN_INFO "done process iteration\n");
		return NULL;
	}
	return curproc;
}

/*
 * This function is called for each "step" of a sequence
 */
static int
de_seq_show(struct seq_file *s, void *v)
{
	uid_t uid = 10000;
	struct task_struct *task = (struct task_struct *) v;
	if(task == current){
		uid = current_uid();
	}else{
		uid = task_uid(task);
	}	
	seq_printf(s, "%lu:\t%u\t%d\t%s\n", offset, uid,task->pid, 
			task->comm);

	return 0;
}

/*
 * This function is called at the end of a sequence
 */
static void
de_seq_stop(struct seq_file *s, void *v)
{
}

static struct seq_operations de_seq_ops = {
	.start = de_seq_start,
	.next = de_seq_next,
	.stop = de_seq_stop,
	.show = de_seq_show
};

static int
proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &de_seq_ops);
};

static struct file_operations file_ops = {
	.owner = THIS_MODULE,
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

static int __init
lkp_init(void)
{
	struct proc_dir_entry *proc_file = NULL;
	printk(KERN_INFO "Hello from have fun module\n");
	printk("num_physpages: %lu, max_mapnr: %lu\n", num_physpages,
	       max_mapnr);
	proc_file = create_proc_entry(proc_fn, 0644, NULL);
	if (proc_file == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
		       proc_fn);
		return -ENOMEM;
	}
	proc_file->proc_fops = &file_ops;
	printk(KERN_INFO "/proc/%s created\n", proc_fn);
	return 0;
}

static void __exit
lkp_cleanup(void)
{
	remove_proc_entry(proc_fn, NULL);
	printk(KERN_INFO "Exit from have fun module\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

module_init(lkp_init);
module_exit(lkp_cleanup);
