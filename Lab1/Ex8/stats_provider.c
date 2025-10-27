//stats_provider.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171");
MODULE_DESCRIPTION("Provider module for compute_stats");
MODULE_VERSION("1.0");

struct stats_result {
    int sum;
    int avg;
    int max;
    int min;
};

int compute_stats(int *arr, size_t len, struct stats_result *res)
{
    size_t i;

    if (!arr || !res || len == 0)
        return -EINVAL;  // Kiểm tra tham số hợp lệ

    res->sum = 0;
    res->max = arr[0];
    res->min = arr[0];

    for (i = 0; i < len; i++) {
        res->sum += arr[i];
        if (arr[i] > res->max)
            res->max = arr[i];
        if (arr[i] < res->min)
            res->min = arr[i];
    }

    res->avg = res->sum / len;

    return 0;
}
EXPORT_SYMBOL(compute_stats);

static int __init provider_init(void)
{
    printk(KERN_INFO "stats_provider: loaded\n");
    return 0;
}

static void __exit provider_exit(void)
{
    printk(KERN_INFO "stats_provider: unloaded\n");
}

module_init(provider_init);
module_exit(provider_exit);

