//hello_proc.c
#include <linux/init.h>      // Macros for init and exit
#include <linux/module.h>    // Core header for modules
#include <linux/kernel.h>    // KERN_INFO
#include <linux/sched.h>     // current->comm, current->pid

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("A simple hello_proc kernel module");
MODULE_VERSION("1.0");

// Hàm init
static int __init hello_proc_init(void)
{
    printk(KERN_INFO "hello_proc: loaded by process \"%s\" (pid=%d)\n",
           current->comm, current->pid);
    return 0;
}

// Hàm exit
static void __exit hello_proc_exit(void)
{
    printk(KERN_INFO "hello_proc: unloaded\n");
}

module_init(hello_proc_init);
module_exit(hello_proc_exit);

