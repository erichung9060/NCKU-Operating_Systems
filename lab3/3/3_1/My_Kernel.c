#include <asm/current.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>

#define procfs_name "Mythread_info"
#define BUFSIZE 1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t ubuffer_len, loff_t *offset) {
    /* Do nothing */
    return 0;
}

static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t ubuffer_len, loff_t *offset) {
    if (*offset > 0) {
        return 0;  // EOF
    }

    struct task_struct *thread;
    for_each_thread(current, thread) {
        if (current->pid == thread->pid) {
            continue;
        }
        int len = sprintf(&buf[*offset], "PID: %d, TID: %d, Priority: %d, State: %d\n",
                          current->pid, thread->pid, thread->prio, thread->__state);
        *offset += len;
    }

    int fail = copy_to_user(ubuf, buf, *offset);
    if (fail) return -EFAULT;

    return *offset;
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void) {
    proc_create(procfs_name, 0644, NULL, &Myops);
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void) {
    remove_proc_entry(procfs_name, NULL);
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
