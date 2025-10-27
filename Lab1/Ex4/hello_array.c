//hello_array.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("Hello array module");

static int nums[10];
static int num_count;
module_param_array(nums, int, &num_count, 0644);
MODULE_PARM_DESC(nums, "Array of integers (max 10)");

static int __init hello_array_init(void)
{
    int i, sum = 0;

    printk(KERN_INFO "hello_array: loaded with %d numbers\n", num_count);

    for (i = 0; i < num_count; i++) {
        printk(KERN_INFO "hello_array: nums[%d] = %d\n", i, nums[i]);
        sum += nums[i];
    }

    printk(KERN_INFO "hello_array: sum = %d\n", sum);

    return 0;
}

static void __exit hello_array_exit(void)
{
    printk(KERN_INFO "hello_array: unloaded\n");
}

module_init(hello_array_init);
module_exit(hello_array_exit);

