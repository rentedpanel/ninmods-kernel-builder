#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/version.h>

#define MODULE_NAME "health_storage_service_alloc"
#define DEVICE_NAME "health_storage_service_alloc"

// IOCTL Definitions matching NINMODS client
#define IOCTL_CHECK_DRIVER          0x8004A801
#define IOCTL_FEATURE_CMD           0x4004A802
#define IOCTL_HEALTH_STORAGE_ATTACH _IOW('H', 1, uint32_t)
#define IOCTL_HEALTH_STORAGE_DETACH _IOW('H', 2, uint64_t)
#define IOCTL_HEALTH_STORAGE_READ   _IOWR('H', 3, struct HealthStorageDriverReadWrite)
#define IOCTL_HEALTH_STORAGE_WRITE  _IOWR('H', 4, struct HealthStorageDriverReadWrite)

struct HealthStorageDriverReadWrite {
    uint64_t handle;
    uint64_t address;
    uint64_t buffer;
    uint64_t size;
};

struct DriverCmd {
    uint8_t cmd;
    uint8_t type;
    uint32_t val;
};

static dev_t g_dev_num;
static struct cdev g_cdev;
static struct class *g_class = NULL;
static struct device *g_device = NULL;

// Process memory read/write via kernel task mm
static ssize_t k_read_proc_mem(pid_t pid, unsigned long addr, void *buf, size_t len) {
    struct task_struct *task;
    struct pid *pid_struct;
    int bytes_read = 0;

    pid_struct = find_get_pid(pid);
    if (!pid_struct) return -ESRCH;

    task = get_pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct);
    if (!task) return -ESRCH;

    bytes_read = access_process_vm(task, addr, buf, len, 0);
    put_task_struct(task);
    return bytes_read;
}

static ssize_t k_write_proc_mem(pid_t pid, unsigned long addr, const void *buf, size_t len) {
    struct task_struct *task;
    struct pid *pid_struct;
    int bytes_written = 0;

    pid_struct = find_get_pid(pid);
    if (!pid_struct) return -ESRCH;

    task = get_pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct);
    if (!task) return -ESRCH;

    bytes_written = access_process_vm(task, addr, (void *)buf, len, 1);
    put_task_struct(task);
    return bytes_written;
}

static int dev_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_CHECK_DRIVER: {
            uint64_t magic = 0x5A3C91E4;
            if (copy_to_user((void __user *)arg, &magic, sizeof(magic)))
                return -EFAULT;
            return 0;
        }

        case IOCTL_HEALTH_STORAGE_ATTACH: {
            uint32_t target_pid = 0;
            if (copy_from_user(&target_pid, (void __user *)arg, sizeof(target_pid)))
                return -EFAULT;
            return 0;
        }

        case IOCTL_HEALTH_STORAGE_DETACH: {
            return 0;
        }

        case IOCTL_HEALTH_STORAGE_READ: {
            struct HealthStorageDriverReadWrite rw;
            void *kbuf;
            ssize_t ret;

            if (copy_from_user(&rw, (void __user *)arg, sizeof(rw)))
                return -EFAULT;

            if (rw.size == 0 || rw.size > 0x100000) // max 1MB per batch
                return -EINVAL;

            kbuf = kmalloc(rw.size, GFP_KERNEL);
            if (!kbuf) return -ENOMEM;

            ret = k_read_proc_mem((pid_t)rw.handle, (unsigned long)rw.address, kbuf, (size_t)rw.size);
            if (ret > 0) {
                if (copy_to_user((void __user *)rw.buffer, kbuf, ret)) {
                    kfree(kbuf);
                    return -EFAULT;
                }
            }
            kfree(kbuf);
            return (ret > 0) ? 0 : -EIO;
        }

        case IOCTL_HEALTH_STORAGE_WRITE: {
            struct HealthStorageDriverReadWrite rw;
            void *kbuf;
            ssize_t ret;

            if (copy_from_user(&rw, (void __user *)arg, sizeof(rw)))
                return -EFAULT;

            if (rw.size == 0 || rw.size > 0x100000)
                return -EINVAL;

            kbuf = kmalloc(rw.size, GFP_KERNEL);
            if (!kbuf) return -ENOMEM;

            if (copy_from_user(kbuf, (void __user *)rw.buffer, rw.size)) {
                kfree(kbuf);
                return -EFAULT;
            }

            ret = k_write_proc_mem((pid_t)rw.handle, (unsigned long)rw.address, kbuf, (size_t)rw.size);
            kfree(kbuf);
            return (ret > 0) ? 0 : -EIO;
        }

        case IOCTL_FEATURE_CMD: {
            struct DriverCmd cmd_data;
            if (copy_from_user(&cmd_data, (void __user *)arg, sizeof(cmd_data)))
                return -EFAULT;
            return 0;
        }

        default:
            return -EINVAL;
    }
}

static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = dev_open,
    .release        = dev_release,
    .unlocked_ioctl = dev_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl   = dev_ioctl,
#endif
};

static int __init ninmods_driver_init(void) {
    int ret;

    ret = alloc_chrdev_region(&g_dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    cdev_init(&g_cdev, &fops);
    g_cdev.owner = THIS_MODULE;

    ret = cdev_add(&g_cdev, g_dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(g_dev_num, 1);
        return ret;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    g_class = class_create(MODULE_NAME);
#else
    g_class = class_create(THIS_MODULE, MODULE_NAME);
#endif
    if (IS_ERR(g_class)) {
        cdev_del(&g_cdev);
        unregister_chrdev_region(g_dev_num, 1);
        return PTR_ERR(g_class);
    }

    g_device = device_create(g_class, NULL, g_dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(g_device)) {
        class_destroy(g_class);
        cdev_del(&g_cdev);
        unregister_chrdev_region(g_dev_num, 1);
        return PTR_ERR(g_device);
    }

    pr_info("NINMODS: Kernel Driver loaded successfully as /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit ninmods_driver_exit(void) {
    if (g_device) device_destroy(g_class, g_dev_num);
    if (g_class) class_destroy(g_class);
    cdev_del(&g_cdev);
    unregister_chrdev_region(g_dev_num, 1);
    pr_info("NINMODS: Kernel Driver unloaded\n");
}

module_init(ninmods_driver_init);
module_exit(ninmods_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NINMODS");
MODULE_DESCRIPTION("NINMODS High-Speed Kernel Memory Driver");
MODULE_VERSION("2.0");
