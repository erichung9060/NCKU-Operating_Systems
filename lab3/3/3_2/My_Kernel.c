#include <asm/current.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#define procfs_name "Mythread_info"
#define BUFSIZE 1024
char buf[BUFSIZE];  // kernel buffer

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t ubuffer_len, loff_t *offset) {
    int buffer_len = strlen(buf);

    int fail = copy_from_user(&buf[buffer_len], ubuf, ubuffer_len);
    if (fail) return -EFAULT;
    buffer_len += ubuffer_len;

    int len = sprintf(&buf[buffer_len], "PID: %d, TID: %d, Time: %llu\n",
                      current->tgid, current->pid, current->utime / 100 / 1000);
    buffer_len += len;

    return ubuffer_len;
}

static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t ubuffer_len, loff_t *offset) {
    int fail = copy_to_user(ubuf, buf, strlen(buf));
    if (fail) return -EFAULT;

    *offset = strlen(buf);
    memset(buf, 0, sizeof(buf));

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
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
