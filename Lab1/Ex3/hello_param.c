#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("22022171 parameterized module");

static char *whom = "22022171";
static int howmany = 3;

module_param(whom, charp, 0644);
MODULE_PARM_DESC(whom, "Name to greet when the module is loaded");

module_param(howmany, int, 0644);
MODULE_PARM_DESC(howmany, "Number of times to print the greeting");

static int __init hello_param_init(void)
{
    int i;
    for (i = 1; i <= howmany; i++) {
        printk(KERN_INFO "hello_param: Hello, %s! [%d/%d]\n",
               whom, i, howmany);
    }
    return 0;
}

static void __exit hello_param_exit(void)
{
    printk(KERN_INFO "hello_param: Bye, %s!\n", whom);
}

module_init(hello_param_init);
module_exit(hello_param_exit);

