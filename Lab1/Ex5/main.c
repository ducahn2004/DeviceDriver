//main.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("Hello multi-file module");

extern void helper_greet(void);

static int __init hello_multi_init(void)
{
    printk(KERN_INFO "hello_multi: module loaded\n");
    helper_greet();
    return 0;
}

static void __exit hello_multi_exit(void)
{
    printk(KERN_INFO "hello_multi: module unloaded\n");
}

module_init(hello_multi_init);
module_exit(hello_multi_exit);
