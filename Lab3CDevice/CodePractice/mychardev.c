#include <linux/module.h>       /* Cần thiết cho mọi module kernel */
#include <linux/kernel.h>       /* Cung cấp macro như printk, KERN_INFO */
#include <linux/fs.h>           /* Cung cấp alloc_chrdev_region, file_operations */
#include <linux/cdev.h>         /* Cung cấp cdev, cdev_init, cdev_add */
#include <linux/slab.h>         /* Cần cho kmalloc, kfree */
#include <linux/mutex.h>        /* Cần cho mutex */
#include <linux/uaccess.h>
#include <linux/moduleparam.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sinh Vien");
MODULE_DESCRIPTION("Simple Character Driver - mychardev");


#define SCULL_QUANTUM 4000      /* Kích thước mỗi block dữ liệu */
#define SCULL_QSET    1000      /* Số lượng pointer trong một mảng */
#define DEFAULT_MAX_SIZE (1024 * 1024)

/* * Biến toàn cục:
 * dev: Lưu trữ số hiệu thiết bị (bao gồm Major và Minor number)
 * my_cdev: Cấu trúc đại diện cho thiết bị ký tự trong kernel
 */
//Exercise 1: implement module init and exit functions
// dev_t dev = 0;
// struct cdev my_cdev;

static struct class *mychardev_class = NULL;
//Excercise 4: implement scull-like data structures
/* Cấu trúc đại diện cho một nút trong danh sách liên kết */
struct scull_qset {
    void **data;                /* Mảng con trỏ tới các quantum */
    struct scull_qset *next;    /* Con trỏ tới nút tiếp theo*/
};


/* Cấu trúc đại diện cho thiết bị */
struct scull_dev {
    struct scull_qset *data;    /* Con trỏ tới qset đầu tiên */
    int quantum;                /* Kích thước quantum */
    int qset;                   /* Kích thước mảng qset */
    unsigned long size;         /* Tổng dữ liệu đã ghi */
    struct mutex lock;          /* Khóa bảo vệ */
    struct cdev cdev;           /* Cấu trúc cdev */
    //Exercise 6
    /*  Biến đếm số lần mở thiết bị */
    int open_count;
};
//Exercise 4
//Thay thế biến toàn cục cũ bằng instance của scull_dev
struct scull_dev my_dev; 
dev_t dev_num = 0;

static unsigned long max_size = DEFAULT_MAX_SIZE;
module_param(max_size, ulong, 0644);
MODULE_PARM_DESC(max_size, "Maximum device size in bytes");

/* Hàm giải phóng toàn bộ bộ nhớ của thiết bị (Dùng cho release hoặc O_WRONLY) */
int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset;   /* "dev" không null */
    int i;

    /* Duyệt qua danh sách liên kết và giải phóng từng phần tử */
    for (dptr = dev->data; dptr; dptr = next) { 
        if (dptr->data) {
            /* Giải phóng từng quantum trong mảng */
            for (i = 0; i < qset; i++)
                kfree(dptr->data[i]);
            /* Giải phóng mảng con trỏ */
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr); /* Giải phóng chính cấu trúc qset */
    }
    
    /* Reset lại trạng thái thiết bị về 0 */
    dev->size = 0;
    dev->quantum = SCULL_QUANTUM;
    dev->qset = SCULL_QSET;
    dev->data = NULL;
    return 0;
}

/* Hàm tìm kiếm qset thứ n trong danh sách, tự động cấp phát nếu chưa có (khi ghi) */
struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
    struct scull_qset *qs = dev->data;

    /* Nếu chưa có nút đầu tiên, cấp phát nó */
    if (!qs) {
        qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (qs == NULL) return NULL;
        memset(qs, 0, sizeof(struct scull_qset));
    }

    /* Di chuyển dọc theo danh sách liên kết */
    while (n--) {
        if (!qs->next) {
            qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (qs->next == NULL) return NULL;
            memset(qs->next, 0, sizeof(struct scull_qset));
        }
        qs = qs->next;
    }
    return qs;
}

//static char kernel_buffer[MEM_SIZE]; /* Buffer tĩnh trong kernel */
/*
 * Định nghĩa bảng phương thức file_operations.
 * Hiện tại chỉ khai báo owner, các hàm open/read/write sẽ được bổ sung sau.
 */
//Excercise 2: implement open() and release() with log messages
// static int mychardev_open(struct inode *inode, struct file *filp)
// {
//     /* In log khi thiết bị được mở */
//     printk(KERN_INFO "Mychardev device opened\n");
//     return 0; /* Trả về 0 báo hiệu thành công */
// }

//Exercise 4: implement scull-like data structures
static int mychardev_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev; /* Khai báo con trỏ tới cấu trúc thiết bị */

    /* * Thay vì dùng biến toàn cục &my_dev, ta dùng macro container_of.
     * inode->i_cdev trỏ tới thành phần cdev bên trong struct scull_dev.
     * container_of giúp "lần ngược" ra địa chỉ của struct scull_dev bao chứa nó.
     */
    dev = container_of(inode->i_cdev, struct scull_dev, cdev); 
    
    /* Lưu con trỏ dev vào private_data để các hàm read/write truy cập nhanh sau này */
    filp->private_data = dev; /*  */

    /*Khóa mutex trước khi truy cập open_count*/
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
        
    /* Nếu mở ở chế độ chỉ ghi (O_WRONLY), xóa sạch dữ liệu cũ */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) { 
        // if (mutex_lock_interruptible(&dev->lock))
        //     return -ERESTARTSYS;
            
        scull_trim(dev); /* Hàm giải phóng bộ nhớ  */

        // printk(KERN_INFO "Mychardev: Device opened. Minor number = %d, 
        //     MINOR(inode->i_rdev));

        // mutex_unlock(&dev->lock);

    }
    //Exercise 6: count open times
    dev->open_count++;
    printk(KERN_INFO "Mychardev: Device opened. Minor number = %d, Open count = %d\n", 
            MINOR(inode->i_rdev), dev->open_count);

    mutex_unlock(&dev->lock);
    return 0;
}
// //Excercise 2: implement open() and release() with log messages
static int mychardev_release(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev = filp->private_data;

    /* Khóa mutex trước khi giảm open_count */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    
    /* Giảm biến đếm */
    dev->open_count--;
    
    /* Mở khóa sau khi hoàn tất thao tác */
    mutex_unlock(&dev->lock);

    printk(KERN_INFO "Mychardev device released\n");
    return 0;
}
//Exercise 3: implement read() and write() with log messages
/* --- BỔ SUNG 3: Hàm đọc dữ liệu (Read) --- */
// static ssize_t mychardev_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
// {
//     /* Kiểm tra xem vị trí đọc hiện tại (*ppos) có vượt quá kích thước buffer không */
//     if (*ppos >= MEM_SIZE)
//         return 0;  /* Trả về 0 báo hiệu kết thúc file (EOF) */

//     /* Nếu số byte người dùng muốn đọc lớn hơn số byte còn lại trong buffer,
//      * chỉ đọc phần còn lại */
//     if (*ppos + len > MEM_SIZE)
//         len = MEM_SIZE - *ppos;

//     /* Gửi dữ liệu từ kernel buffer ra user space
//      * copy_to_user trả về 0 nếu thành công, > 0 nếu thất bại
//      */
//     if (copy_to_user(buf, kernel_buffer + *ppos, len))
//         return -EFAULT;

//     /* Cập nhật vị trí con trỏ file sau khi đọc xong */
//     *ppos += len;

//     printk(KERN_INFO "Mychardev: Read %zu bytes\n", len);
//     return len; /* Trả về số byte đã đọc thực tế */
// }
// //Exercise 3: implement read() and write() with log messages
// /* --- BỔ SUNG 4: Hàm ghi dữ liệu (Write) --- */
// static ssize_t mychardev_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
// {
//     /* Kiểm tra xem vị trí ghi có vượt quá bộ nhớ không */
//     if (*ppos >= MEM_SIZE)
//         return -ENOSPC; /* Error: No space left on device */

//     /* Nếu dữ liệu ghi lớn hơn khoảng trống còn lại, cắt bớt dữ liệu */
//     if (*ppos + len > MEM_SIZE)
//         len = MEM_SIZE - *ppos;

//     /* Nhận dữ liệu từ user space và lưu vào kernel buffer */
//     if (copy_from_user(kernel_buffer + *ppos, buf, len))
//         return -EFAULT;

//     /* Cập nhật vị trí con trỏ file */
//     *ppos += len;

//     printk(KERN_INFO "Mychardev: Written %zu bytes\n", len);
//     return len; /* Trả về số byte đã ghi thành công */
// }
//Exercise 4: implement scull-like data structures
static ssize_t mychardev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data; 
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset; /* Kích thước dữ liệu 1 qset quản lý [cite: 823] */
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    /* Kiểm tra EOF */
    if (*f_pos >= dev->size) goto out;
    if (*f_pos + count > dev->size) count = dev->size - *f_pos;

    /* Tìm vị trí trong list và array */
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum; 
    q_pos = rest % quantum;

    /* Tìm qset tại vị trí item */
    dptr = scull_follow(dev, item);

    /* Nếu qset rỗng hoặc quantum rỗng thì không có gì để đọc */
    if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
        goto out;

    /* Chỉ đọc tối đa đến hết quantum hiện tại */
    if (count > quantum - q_pos) count = quantum - q_pos;

    /* Copy data to user */
    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

out:
    mutex_unlock(&dev->lock);
    return retval;
}
//Exercise 4-5: implement scull-like data structures
// static ssize_t mychardev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
// {
//     struct scull_dev *dev = filp->private_data;
//     struct scull_qset *dptr;
//     int quantum = dev->quantum;
//     int qset = dev->qset;
//     int itemsize = quantum * qset;
//     int item, s_pos, q_pos, rest;
//     ssize_t retval = -ENOMEM;

//     if (mutex_lock_interruptible(&dev->lock))
//         return -ERESTARTSYS;

//     /* Tính toán vị trí cần ghi */
//     item = (long)*f_pos / itemsize;
//     rest = (long)*f_pos % itemsize;
//     s_pos = rest / quantum; 
//     q_pos = rest % quantum;

//     /* Tìm (hoặc tạo mới) qset tại vị trí item */
//     dptr = scull_follow(dev, item);
//     if (dptr == NULL) goto out;

//     /* Nếu mảng data chưa có, cấp phát nó  */
//     if (!dptr->data) {
//         dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
//         if (!dptr->data) goto out;
//         memset(dptr->data, 0, qset * sizeof(char *));
//     }

//     /* Nếu quantum cụ thể chưa có, cấp phát nó  */
//     if (!dptr->data[s_pos]) {
//         dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
//         if (!dptr->data[s_pos]) goto out;
//     }

//     /* Chỉ ghi tối đa đến hết quantum hiện tại */
//     if (count > quantum - q_pos) count = quantum - q_pos;

//     /* Copy data from user */
//     if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
//         retval = -EFAULT;
//         goto out;
//     }

//     *f_pos += count;
//     retval = count;

//     /* Cập nhật kích thước file nếu ghi vượt quá size cũ  */
//     if (dev->size < *f_pos) dev->size = *f_pos;
//     printk(KERN_INFO "Mychardev: Write finished. Received %zu bytes from user space\n", count);

// out:
//     mutex_unlock(&dev->lock);
//     return retval;
// }
//Exercise 7: Maximum size limit for scull device
static ssize_t mychardev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;
    
    // Khai báo theo mẫu chuẩn
    int item, s_pos, q_pos;
    ssize_t retval = -ENOMEM;
    size_t rest;
    size_t space_left;
    int truncated = 0; // Cờ báo hiệu bị cắt cụt

    /* --- BƯỚC 1: Khóa Đồng bộ --- */
    // Sử dụng mutex_lock (non-interruptible) để đảm bảo tính nhất quán của cấu trúc scull
    mutex_lock(&dev->lock);

    /* --- BƯỚC 2: Kiểm tra và Cắt cụt theo Giới hạn TỔNG --- */
    // Dùng biến toàn cục HARD_MAX_SIZE (tên chuẩn thường là max_size)
    if (*f_pos >= max_size) {
        printk(KERN_WARNING "Mychardev: ERROR - Write position (%lld) exceeds max size (%lu).\n",
                (long long)*f_pos, max_size);
        retval = -ENOSPC;
        goto out;
    }
    
    // Giới hạn số byte còn lại có thể ghi (space_left)
    space_left = max_size - *f_pos;
    if (count > space_left) {
        count = space_left; // Cắt bớt số byte yêu cầu
        truncated = 1;      // Bật cờ cảnh báo
    }

    // Nếu sau khi giới hạn, không còn chỗ để ghi
    if (count == 0) {
        retval = 0;
        goto out;
    }
    
    /* --- BƯỚC 3: Tính toán Vị trí trong Cấu trúc Scull --- */
    item = (int)(*f_pos / itemsize);
    rest = (size_t)(*f_pos % itemsize);
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    /* --- BƯỚC 4: Lấy (hoặc Cấp phát) qset và quantum --- */
    dptr = scull_follow(dev, item);
    if (!dptr)
        goto out;

    /* Nếu mảng data chưa có, cấp phát nó */
    if (!dptr->data) {
        dptr->data = kcalloc(qset, sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
    }
    
    /* Nếu quantum cụ thể chưa có, cấp phát nó */
    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kzalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }
    
    /* --- BƯỚC 5: Giới hạn theo Quantum hiện tại --- */
    // Đảm bảo không ghi tràn quantum (Lưu ý: count đã bị cắt cụt theo HARD_MAX_SIZE ở trên)
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    /* --- BƯỚC 6: Sao chép Dữ liệu --- */
    if (copy_from_user((char *)dptr->data[s_pos] + q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }

    /* --- BƯỚC 7: Cập nhật Trạng thái và Log --- */
    *f_pos += count;
    if (dev->size < *f_pos)
        dev->size = *f_pos;
    retval = count;

    /* In cảnh báo Truncated (nếu xảy ra) */
    if (truncated)
        printk(KERN_WARNING "Mychardev: write truncated to %zd bytes (max_size=%lu)\n",
                retval, max_size);
               
    printk(KERN_INFO "Mychardev: Write finished. Received %zd bytes from user space\n", retval);

out:
    mutex_unlock(&dev->lock);
    return retval;
}
//Exercise 8
static loff_t mychardev_llseek(struct file *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t new_pos;

    /* Khóa Mutex để đảm bảo f_pos và dev->size không thay đổi giữa chừng */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    switch (whence) {
        case SEEK_SET:
            // Vị trí tuyệt đối: new_pos = offset
            new_pos = off;
            break;

        case SEEK_CUR:
            // Vị trí tương đối: new_pos = vị trí hiện tại + offset
            new_pos = filp->f_pos + off;
            break;

        case SEEK_END:
            // Vị trí tương đối theo kích thước file: new_pos = dev->size + offset
            new_pos = dev->size + off;
            break;

        default:
            mutex_unlock(&dev->lock);
            return -EINVAL; // Tham số không hợp lệ
    }

    /* Kiểm tra tính hợp lệ của vị trí mới */
    if (new_pos < 0) {
        mutex_unlock(&dev->lock);
        return -EINVAL; // Vị trí không được âm
    }

    /* Nếu vị trí mới vượt quá giới hạn tối đa (max_size), trả về lỗi */
    if (new_pos > max_size) {
        mutex_unlock(&dev->lock);
        printk(KERN_WARNING "Mychardev: Seek beyond max size (%lu).\n", max_size);
        return -EFBIG; // File quá lớn (Error File Big)
    }

    /* Cập nhật vị trí và in log */
    filp->f_pos = new_pos;
    printk(KERN_INFO "Mychardev: llseek successful. New position: %lld\n", new_pos);

    mutex_unlock(&dev->lock);
    return new_pos;
}
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mychardev_open,       
    .release = mychardev_release, 
    .read = mychardev_read,   
    .write = mychardev_write, 
    .llseek = mychardev_llseek,
};


/* * Hàm khởi tạo module (Init Function)
 * Hàm này được gọi khi lệnh insmod được thực thi.
 */
//Exercise 1: implement module init and exit functions
// static int __init mychardev_init(void)
// {
//     int ret;

//     /* * BƯỚC 1: Cấp phát động số hiệu thiết bị 
//      * Sử dụng alloc_chrdev_region để kernel tự chọn Major number còn trống.
//      * Tham số:
//      * - &dev: Con trỏ để lưu kết quả số hiệu được cấp.
//      * - 0: Minor number bắt đầu (baseminor).
//      * - 1: Số lượng thiết bị cần đăng ký.
//      * - "mychardev": Tên driver hiển thị trong /proc/devices.
//      */
//     ret = alloc_chrdev_region(&dev, 0, 1, "mychardev");
//     if (ret < 0) {
//         printk(KERN_ERR "mychardev: Failed to allocate device number\n");
//         return ret;
//     }
    
//     /* Ghi log major và minor number đã được cấp phát thành công */
//     printk(KERN_INFO "mychardev: Registered successfully. Major = %d, Minor = %d\n", 
//             MAJOR(dev), MINOR(dev));

//     /* * BƯỚC 2: Khởi tạo cấu trúc cdev
//      * Hàm cdev_init liên kết cấu trúc my_cdev với bảng phương thức fops.
//      * Thiết lập owner để module không bị gỡ bỏ khi đang sử dụng.
//      */
//     cdev_init(&my_cdev, &fops);
//     my_cdev.owner = THIS_MODULE;

//     /* * BƯỚC 3: Thêm thiết bị vào hệ thống
//      * Hàm cdev_add làm cho thiết bị "sống" (live) để kernel nhận biết.
//      * Tham số:
//      * - &my_cdev: Cấu trúc cdev đã khởi tạo.
//      * - dev: Số hiệu thiết bị (Major + Minor).
//      * - 1: Số lượng thiết bị.
//      */
//     ret = cdev_add(&my_cdev, dev, 1);
//     if (ret < 0) {
//         printk(KERN_ERR "mychardev: Failed to add cdev (error %d)\n", ret);
//         /* Nếu thêm cdev thất bại, phải giải phóng số hiệu thiết bị trước khi thoát */
//         unregister_chrdev_region(dev, 1);
//         return ret;
//     }

//     printk(KERN_INFO "mychardev: Module loaded and cdev added to the system.\n");
//     return 0; /* Trả về 0 báo hiệu khởi tạo thành công */
// }

// /* * Hàm kết thúc module (Exit Function)
//  * Hàm này được gọi khi lệnh rmmod được thực thi.
//  */
// //Exercise 1: implement module init and exit functions
// static void __exit mychardev_exit(void)
// {
//     /* * BƯỚC 4: Dọn dẹp tài nguyên
//      * Thực hiện theo thứ tự ngược lại với quá trình khởi tạo.
//      */

//     /* Xóa cdev khỏi hệ thống kernel */
//     cdev_del(&my_cdev);

//     /* Trả lại số hiệu thiết bị cho kernel để các driver khác có thể sử dụng */
//     unregister_chrdev_region(dev, 1);

//     printk(KERN_INFO "mychardev: Module unloaded.\n");
// }
//Exercise 4: implement scull-like data structures
static int __init mychardev_init(void)
{
    int ret;
    
    /* --- Khởi tạo thiết bị scull --- */
    mutex_init(&my_dev.lock);       
    
    my_dev.quantum = max_size; // Đặt quantum = 10
    my_dev.qset = 1;                // Đặt qset = 1
    my_dev.data = NULL;
    my_dev.size = 0;

    /* Khởi tạo biến đếm số lần mở thiết bị */
    my_dev.open_count = 0;

    ret = alloc_chrdev_region(&dev_num, 0, 1, "mychardev");
    if (ret < 0) {
        printk(KERN_ERR "Mychardev: Failed to allocate device number (%d)\n", ret);
        return ret;
    }
    
    cdev_init(&my_dev.cdev, &fops); 
    my_dev.cdev.owner = THIS_MODULE;
    
    ret = cdev_add(&my_dev.cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "Mychardev: Failed to add cdev (%d)\n", ret);
        goto fail_register; // Nhãn lỗi cũ
    }
    // 3. Tạo Class thiết bị (Hiển thị trong /sys/class/)
    mychardev_class = class_create(THIS_MODULE, "mychardev");
    if (IS_ERR(mychardev_class)) {
        printk(KERN_ERR "Mychardev: Failed to create device class\n");
        ret = PTR_ERR(mychardev_class);
        goto fail_cdev_add;
    }
    // 4. Tạo Device Node (Tự động tạo file /dev/mychardev0)
    if (device_create(mychardev_class, NULL, dev_num, NULL, "mychardev0") == NULL) {
        printk(KERN_ERR "Mychardev: Failed to create device node /dev/mychardev0\n");
        ret = -ENODEV;
        goto fail_class_create;
    }
    printk(KERN_INFO "Mychardev: Registered successfully. Node /dev/mychardev0 created.\n");
    printk(KERN_INFO "mychardev: Registered successfully. Major = %d, Minor = %d\n", 
            MAJOR(dev_num), MINOR(dev_num));
            
    return 0;

    fail_class_create:
        class_destroy(mychardev_class);
    fail_cdev_add:
        cdev_del(&my_dev.cdev);
    fail_register:
        unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit mychardev_exit(void)
{
    /* Giải phóng toàn bộ bộ nhớ động trước khi gỡ module */
    scull_trim(&my_dev);

    // 2. Xóa Device Node (/dev/mychardev0)
    device_destroy(mychardev_class, dev_num);
    
    // 3. Hủy Class thiết bị
    class_destroy(mychardev_class);

    /* 4. Xóa cdev khỏi hệ thống */
    cdev_del(&my_dev.cdev);

    /* 5. Trả lại số hiệu thiết bị */
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "mychardev: Module unloaded.\n");
}
//Exercise 1: implement module init and exit functions
/* Macro đăng ký điểm vào và điểm ra của module */
module_init(mychardev_init);
module_exit(mychardev_exit);