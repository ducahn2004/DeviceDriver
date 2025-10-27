//hello_sysfs
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("A simple hello_sysfs kernel module");
MODULE_VERSION("1.0");

static int val = 0;
module_param(val, int, 0644);
MODULE_PARM_DESC(val, "An integer parameter");

static int __init hello_sysfs_init(void)
{
    printk(KERN_INFO "hello_sysfs: loaded with val=%d\n", val);
    return 0;
}

static void __exit hello_sysfs_exit(void)
{
    printk(KERN_INFO "hello_sysfs: unloaded\n");
}

module_init(hello_sysfs_init);
module_exit(hello_sysfs_exit);

