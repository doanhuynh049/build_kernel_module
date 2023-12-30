#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/fs.h>
#include <linux/uaccess.h>
#define DEF_MAJOR_NUMER     56
#define DEF_DEVICE   "rwmod"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EMB");
MODULE_DESCRIPTION("A read write module");

volatile static int isOpen = 0;
static char msg[1024];
static int num_bytes = 0;

static ssize_t rwmod_read(struct file* filep, char __user * output, size_t nbytes, loff_t *offset)
{
    if (offset == NULL) return -EINVAL;
    if (*offset >= num_bytes) return 0;
    int bytes_read = 0;
    while ((*offset < num_bytes) && (bytes_read < nbytes))
    {
        put_user(msg[*offset], &output[bytes_read]);
        (*offset)++;
        bytes_read++;
    }
    return bytes_read;
}

static ssize_t rwmod_write(struct file* filep, const char __user * input, size_t nbytes, loff_t *offset)
{
    printk(KERN_ALERT "ERROR - cannot handle write\n");
    return -EINVAL;
}

static int rwmod_open(struct inode* inodep, struct file* filep)
{
    if (isOpen == 1)
    {
        printk(KERN_INFO "ERROR - rwmod device already open\n");
        return -EBUSY;
    }
    isOpen = 1;
    try_module_get(THIS_MODULE);
    return 0;
}

static int rwmod_release(struct inode* inodep, struct file* filep)
{
    if (isOpen == 0)
    {
        printk(KERN_INFO "ERROR - rwmod device was not opened\n");
        return -EBUSY;
    }
    isOpen = 0;
    module_put(THIS_MODULE);
    return 0;
}

long rwmod_unlocked_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case 0x01:
            printk("command 1: arg = %lu\n", arg);
            break;
        default:
            printk("command = 0x%x\n, arg = %lu", cmd, arg);
    }
    return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    // .read = rwmod_read,
    // .write = rwmod_write,
    // .open = rwmod_open,
    // .release = rwmod_release,
    .unlocked_ioctl = rwmod_unlocked_ioctl
};

static int __init rwmod_init(void)
{
    printk(KERN_INFO "RWmod: Hello EMB!\n");
    int devnum = register_chrdev(DEF_MAJOR_NUMER, DEF_DEVICE, &fops);
    printk(KERN_INFO "RWmod: major: %d!\n", devnum);
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit rwmod_cleanup(void)
{
    printk(KERN_INFO "RWmod: Cleaning up module.\n");
    unregister_chrdev(DEF_MAJOR_NUMER, DEF_DEVICE);
}

module_init(rwmod_init);
module_exit(rwmod_cleanup);
