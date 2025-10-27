#include <linux/module.h>
#include <linux/kernel.h>

void helper_greet(void)
{
    printk(KERN_INFO "hello_multi: greeting from helper_greet()\n");
}
