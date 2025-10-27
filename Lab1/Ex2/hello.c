#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("Hello LKM");
static int __init hello_init(void) {
 printk(KERN_INFO "Hello, Kernel\n");
 return 0;
}
static void __exit hello_exit(void) {
 printk(KERN_INFO "Bye, Kernel\n");
}
module_init(hello_init);
module_exit(hello_exit);
