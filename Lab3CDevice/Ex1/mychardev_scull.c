/* mychardev_scull.c
 *
 * Minimal scull-like char device implementing:
 * 1) dynamic alloc_chrdev_region + cdev_init + cdev_add, proper cleanup
 * 2) open() and release()
 * 3) read() & write() using kernel memory and copy_to_user/copy_from_user
 * 4) dynamic memory organized with scull_qset/scull_dev model
 * 5) detailed debug logs: minor number on open, bytes copied on read/write
 * 6) open_count in scull_dev protected by mutex
 * 7) module parameter max_size to limit device size
 * 8) llseek() supporting SEEK_SET, SEEK_CUR, SEEK_END
 *
 * Build: make
 * Load: sudo insmod mychardev_scull.ko
 * Unload: sudo rmmod mychardev_scull
 * Device node: created automatically as /dev/mychardev0 via class/device.
 *
 * Based on scull patterns described in Lab3_LDD. :contentReference[oaicite:3]{index=3}
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("22022171 - DUC ANH NGUYEN");
MODULE_DESCRIPTION("Lab3 exercise");

/* ---------- Module parameters and defaults ---------- */

/* max_size module parameter (bytes). Exercise 7: limit device memory size. */
static unsigned long max_size = 65536; /* default 64KB; student can change */
module_param(max_size, ulong, 0444);
MODULE_PARM_DESC(max_size, "Maximum allowed size (bytes) for device storage");

/* quantum and qset defaults (scull model) */
static int scull_quantum = 4000;
static int scull_qset = 1000;
module_param(scull_quantum, int, 0444);
module_param(scull_qset, int, 0444);

/* number of devices to create (one for lab) */
#define MYDEV_NR_DEVS 1

/* device name */
static const char *mydev_name = "mychardev";

/* ---------- scull-like data structures ---------- */

/* Exercise 4: implement scull_qset structure for dynamic memory management */
struct scull_qset {
    void **data;                /* array of pointers to quantum blocks */
    struct scull_qset *next;    /* next qset in list */
};

/* Exercise 6: add open_count in struct scull_dev and a mutex for sync */
struct scull_dev {
    struct scull_qset *data;    /* pointer to first qset */
    int quantum;                /* quantum size */
    int qset;                   /* qset size (number of quantums per qset) */
    unsigned long size;         /* number of bytes stored */
    struct cdev cdev;           /* cdev struct */
    struct mutex lock;          /* mutex to protect device data */
    int open_count;             /* count how many times opened */
};

/* device array */
static struct scull_dev *mydevs;

/* device numbering */
static dev_t mydev_base;
static struct class *mydev_class;

/* Forward declarations for file operations */
static int mydev_open(struct inode *inode, struct file *filp);
static int mydev_release(struct inode *inode, struct file *filp);
static ssize_t mydev_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos);
static ssize_t mydev_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos);
static loff_t mydev_llseek(struct file *filp, loff_t off, int whence);

/* file operations table (Exercise 2,3,8) */
static const struct file_operations mydev_fops = {
    .owner = THIS_MODULE,
    .llseek = mydev_llseek,    /* Exercise 8 */
    .read = mydev_read,        /* Exercise 3 */
    .write = mydev_write,      /* Exercise 3 */
    .open = mydev_open,        /* Exercise 2 */
    .release = mydev_release,  /* Exercise 2 */
};

/* ---------- Helper functions for scull memory management ---------- */

/* Exercise 4: follow/get qset at index; allocate qsets as needed */
static struct scull_qset *scull_follow(struct scull_dev *dev, int item)
{
    struct scull_qset *qs = dev->data;
    int i;

    if (!qs) {
        qs = kzalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (!qs) return NULL;
        dev->data = qs;
    }

    for (i = 0; i < item; i++) {
        if (!qs->next) {
            qs->next = kzalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (!qs->next) return NULL;
        }
        qs = qs->next;
    }
    return qs;
}

/* Exercise 4: free all memory (truncate) - scull_trim */
static void scull_trim(struct scull_dev *dev)
{
    struct scull_qset *qs = dev->data;
    struct scull_qset *next;
    int i;

    while (qs) {
        if (qs->data) {
            for (i = 0; i < dev->qset; i++) {
                kfree(qs->data[i]);
                qs->data[i] = NULL;
            }
            kfree(qs->data);
            qs->data = NULL;
        }
        next = qs->next;
        kfree(qs);
        qs = next;
    }
    dev->data = NULL;
    dev->size = 0;
}

/* ---------- File operations implementations ---------- */

/* Exercise 2 + 5 + 6:
 * open(): find device struct, attach to filp->private_data,
 * increment open_count (protected by mutex), print minor number and open_count.
 * If opened with O_WRONLY, truncate device (free memory).
 */
static int mydev_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    /* increment open_count with mutex (Exercise 6: protect counter) */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    dev->open_count++;
    pr_info("mychardev: device (minor=%d) opened; open_count=%d\n",
            MINOR(inode->i_rdev), dev->open_count); /* Exercise 5: print minor */
    /* If open for write only, truncate */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        scull_trim(dev); /* Exercise 4: truncate on write-only open */
        pr_info("mychardev: truncated device on O_WRONLY open\n");
    }
    mutex_unlock(&dev->lock);

    return 0;
}

/* Exercise 6 + 2:
 * release(): decrement open_count (protected) and print status.
 */
static int mydev_release(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev = filp->private_data;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    dev->open_count--;
    pr_info("mychardev: device (minor=%d) released; open_count=%d\n",
            MINOR(inode->i_rdev), dev->open_count);
    mutex_unlock(&dev->lock);
    return 0;
}

/* Exercise 8:
 * llseek(): support SEEK_SET, SEEK_CUR, SEEK_END - adjust f_pos within [0, dev->size]
 */
static loff_t mydev_llseek(struct file *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos = 0;

    if (!dev) return -ENODEV;

    switch (whence) {
    case SEEK_SET:
        newpos = off;
        break;
    case SEEK_CUR:
        newpos = filp->f_pos + off;
        break;
    case SEEK_END:
        newpos = dev->size + off;
        break;
    default:
        return -EINVAL;
    }
    if (newpos < 0) return -EINVAL;
    if (newpos > (loff_t)dev->size) newpos = dev->size; /* clamp to dev->size */
    filp->f_pos = newpos;
    return newpos;
}

/* Exercise 3 + 4 + 5:
 * read(): map ppos -> qset/quantum/offset, copy_to_user
 * print number of bytes actually copied (debug).
 */
static ssize_t mydev_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *qs;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize;
    int item, s_pos, q_pos;
    size_t to_copy;
    ssize_t ret = 0;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (*ppos >= dev->size)
        goto out; /* EOF */

    if (*ppos + count > dev->size)
        count = dev->size - *ppos;

    itemsize = quantum * qset;
    item = *ppos / itemsize;
    s_pos = (*ppos % itemsize) / quantum;
    q_pos = (*ppos % itemsize) % quantum;

    /* follow to the right qset; do not allocate on read */
    qs = dev->data;
    while (item--) {
        if (!qs) break;
        qs = qs->next;
    }
    if (!qs || !qs->data || !qs->data[s_pos])
        goto out; /* nothing to read */

    /* calculate how much to copy */
    to_copy = min((size_t)quantum - q_pos, count);

    if (copy_to_user(buf, qs->data[s_pos] + q_pos, to_copy)) {
        ret = -EFAULT;
        goto out;
    }
    *ppos += to_copy;
    ret = to_copy;
    pr_info("mychardev: read %zu bytes (requested %zu) from pos %lld\n", to_copy, count, (long long)(*ppos - to_copy)); /* Exercise 5 */

out:
    mutex_unlock(&dev->lock);
    return ret;
}

/* Exercise 3 + 4 + 7 + 5:
 * write(): map ppos -> qset/quantum/offset, allocate qsets/quantum as needed,
 * enforce max_size (module param), and print actual bytes copied.
 */
static ssize_t mydev_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *qs;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize;
    int item, s_pos, q_pos;
    size_t to_copy;
    ssize_t ret = 0;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    /* Enforce max_size (Exercise 7) */
    if (*ppos >= max_size) {
        pr_warn("mychardev: write beyond max_size (%lu). Ignored.\n", max_size);
        ret = -ENOSPC;
        goto out;
    }
    if (*ppos + count > max_size) {
        /* clamp count to available space */
        count = max_size - *ppos;
        pr_warn("mychardev: write truncated to %zu bytes to respect max_size=%lu\n", count, max_size);
    }

    itemsize = quantum * qset;
    item = *ppos / itemsize;
    s_pos = (*ppos % itemsize) / quantum;
    q_pos = (*ppos % itemsize) % quantum;

    /* follow and allocate qset if necessary */
    qs = scull_follow(dev, item);
    if (!qs) {
        ret = -ENOMEM;
        goto out;
    }
    if (!qs->data) {
        qs->data = kzalloc(sizeof(void *) * qset, GFP_KERNEL);
        if (!qs->data) {
            ret = -ENOMEM;
            goto out;
        }
    }
    if (!qs->data[s_pos]) {
        qs->data[s_pos] = kzalloc(quantum, GFP_KERNEL);
        if (!qs->data[s_pos]) {
            ret = -ENOMEM;
            goto out;
        }
    }

    /* how much we can copy into this quantum */
    to_copy = min((size_t)quantum - q_pos, count);

    if (copy_from_user(qs->data[s_pos] + q_pos, buf, to_copy)) {
        ret = -EFAULT;
        goto out;
    }
    *ppos += to_copy;
    if (dev->size < *ppos)
        dev->size = *ppos;
    ret = to_copy;
    pr_info("mychardev: wrote %zu bytes (requested %zu) to pos %lld; device size=%lu\n",
            to_copy, count, (long long)(*ppos - to_copy), dev->size); /* Exercise 5 */

out:
    mutex_unlock(&dev->lock);
    return ret;
}

/* ---------- Initialization and cleanup (registration) ---------- */

/* Exercise 1: module init registers device region, init cdev, cdev_add, create /dev node */
static int __init mydev_init(void)
{
    int ret, i;
    struct device *device;

    /* allocate device numbers dynamically */
    ret = alloc_chrdev_region(&mydev_base, 0, MYDEV_NR_DEVS, mydev_name);
    if (ret < 0) {
        pr_err("mychardev: alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }
    pr_info("mychardev: allocated major=%d\n", MAJOR(mydev_base)); /* show major - helpful for testing */ /* :contentReference[oaicite:4]{index=4} */

    /* allocate device structures */
    mydevs = kzalloc(sizeof(struct scull_dev) * MYDEV_NR_DEVS, GFP_KERNEL);
    if (!mydevs) {
        unregister_chrdev_region(mydev_base, MYDEV_NR_DEVS);
        return -ENOMEM;
    }

    /* create device class for automatic /dev creation */
    mydev_class = class_create(THIS_MODULE, mydev_name);
    if (IS_ERR(mydev_class)) {
        pr_err("mychardev: class_create failed\n");
        ret = PTR_ERR(mydev_class);
        goto fail_class;
    }

    /* initialize each device */
    for (i = 0; i < MYDEV_NR_DEVS; i++) {
        struct scull_dev *dev = &mydevs[i];
        dev->quantum = scull_quantum;
        dev->qset = scull_qset;
        dev->size = 0;
        mutex_init(&dev->lock);
        dev->open_count = 0;

        cdev_init(&dev->cdev, &mydev_fops);
        dev->cdev.owner = THIS_MODULE;

        ret = cdev_add(&dev->cdev, MKDEV(MAJOR(mydev_base), MINOR(mydev_base) + i), 1);
        if (ret) {
            pr_err("mychardev: cdev_add failed for minor %d: %d\n", i, ret);
            goto fail_cdev;
        }

        /* create device node /dev/mychardev<i> */
        device = device_create(mydev_class, NULL, MKDEV(MAJOR(mydev_base), MINOR(mydev_base) + i),
                               NULL, "%s%d", mydev_name, i);
        if (IS_ERR(device)) {
            pr_err("mychardev: device_create failed for minor %d\n", i);
            ret = PTR_ERR(device);
            cdev_del(&dev->cdev);
            goto fail_cdev;
        }
    }

    pr_info("mychardev: successfully loaded\n");
    return 0;

fail_cdev:
    while (--i >= 0) {
        device_destroy(mydev_class, MKDEV(MAJOR(mydev_base), MINOR(mydev_base) + i));
        cdev_del(&mydevs[i].cdev);
        scull_trim(&mydevs[i]);
    }
    class_destroy(mydev_class);
fail_class:
    kfree(mydevs);
    unregister_chrdev_region(mydev_base, MYDEV_NR_DEVS);
    return ret;
}

/* Exercise 1: proper cleanup on module removal (rmmod) */
static void __exit mydev_exit(void)
{
    int i;
    for (i = 0; i < MYDEV_NR_DEVS; i++) {
        device_destroy(mydev_class, MKDEV(MAJOR(mydev_base), MINOR(mydev_base) + i));
        cdev_del(&mydevs[i].cdev);
        /* free dynamic memory */
        mutex_lock(&mydevs[i].lock);
        scull_trim(&mydevs[i]);
        mutex_unlock(&mydevs[i].lock);
    }
    class_destroy(mydev_class);
    kfree(mydevs);
    unregister_chrdev_region(mydev_base, MYDEV_NR_DEVS);
    pr_info("mychardev: unloaded and resources freed\n");
}

module_init(mydev_init);
module_exit(mydev_exit);
