/* We are part of the kernel */
#undef __KERNEL__
#define __KERNEL__


#undef MODULE
#define MODULE

/* included for all kernel modules */
#include <linux/module.h>
/* included for __init and __exit macros */
#include <linux/init.h>
/* included for register_chrdev */
#include <linux/fs.h>

#define SUCCESS 0

MODULE_LICENSE("GPL");

/* device functions */
static int device_open(struct inode* inode, struct file* file)
{
    return SUCCESS;
}

static int device_release(struct inode* inode, struct file* file) 
{
    return SUCCESS;
}

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loft_t offset)
{

}

static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loft_t offset)
{
    return 
}

static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned int ioctl_param)
{
    if (ioctl_command_id == MSG_SLOT_CHANNEL)
    {

    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}
/* device setup */
struct file_operations Fops =
{
  .owner    = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
  .read     = device_read,
  .write    = device_write,
  .open     = device_open,
  .release  = device_release,
};

/* loader */
static int __init msgslot_init(void)
{
    /* when soemthing fails returns non-zero value */
    int ret_register_chrdev = register_chrdev(240, "msgslot", &Fops);
    if (ret_register_chrdev < 0)
    {
        printk(KERN_ERR "msgslot device registration failed for %d", ret_register_chrdev);
        return ret_register_chrdev;
    }
    return 0;
}

/* unloader */
static void __exit msgslot_exit(void)
{
    unregister_chrdev(240, "msgslot");
}

/* Defining initialization and exit functions of the driver */
module_init(msgslot_init);
module_exit(msgslot_exit);