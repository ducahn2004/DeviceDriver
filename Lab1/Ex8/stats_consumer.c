//stats_consumer.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("Consumer module using compute_stats");
MODULE_VERSION("1.0");

// Struct trùng với Provider
struct stats_result {
    int sum;
    int avg;
    int max;
    int min;
};

// Khai báo prototype của hàm từ Provider
extern int compute_stats(int *arr, size_t len, struct stats_result *res);

static int __init consumer_init(void)
{
    int sample[] = {10, 20, 30, 40, 50};
    size_t len = sizeof(sample)/sizeof(sample[0]);
    struct stats_result res;
    int ret;

    ret = compute_stats(sample, len, &res);
    if (ret) {
        printk(KERN_ERR "stats_consumer: compute_stats failed: %d\n", ret);
        return ret;
    }

    printk(KERN_INFO "stats_consumer: sum=%d avg=%d max=%d min=%d\n",
           res.sum, res.avg, res.max, res.min);

    return 0;
}

static void __exit consumer_exit(void)
{
    printk(KERN_INFO "stats_consumer: unloaded\n");
}

module_init(consumer_init);
module_exit(consumer_exit);

