/* We are part of the kernel */
#undef __KERNEL__
#define __KERNEL__

#undef MODULE
#define MODULE

#include <linux/kernel.h>  /* We're doing kernel work */
#include <linux/module.h>  /* Specifically, a module */
#include <linux/fs.h>      /* for register_chrdev */
#include <linux/uaccess.h> /* for get_user and put_user */
#include <linux/slab.h>    /* included for kmalloc flag GFP_KERNEL */

#include "message_slot.h"

MODULE_LICENSE("GPL");

/* Linked list structure for each device for different channels */
/* each node represents a channel (id, channel itself, length of the message and a pointer to the next channel in the device) */
typedef struct NODE
{
    unsigned int channel_id;
    char channel[BUF_LEN];
    int msg_len;
    struct NODE *next;
} NODE;

/* a static array (initialized with zeros) for the different devices (minor numbers between 0 to 255) */
/*
typedef struct device
{
    int initialized;
    NODE *head;
} device;
*/
static NODE *devices[256];

/* device functions */
static int device_open(struct inode *inode, struct file *file)
{
    /*
    unsigned int minor = iminor(inode);
    if (devices[minor].initialized == 0)
    {
        
        devices[minor].initialized = 1;
        devices[minor].head = NULL;
    }
    */
   /* new device */
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    return SUCCESS;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    NODE *node = (NODE *)file->private_data;
    int i;

    if (node == NULL)
    {
        /* no channel has been set on the file descriptor */
        return -EINVAL;
    }

    if (buffer == NULL)
    {
        /* arguments validation */
        return -EINVAL;
    }
    
    if (node->msg_len == 0)
    {
        /* no message exists on the channel */
        return -EWOULDBLOCK;
    }

    if (length < node->msg_len)
    {
        /* provided buffer is too small */
        return -ENOSPC;
    }

    for (i = 0; i < node->msg_len && i < BUF_LEN; i++)
    {
        put_user(node->channel[i], &buffer[i]);
    }
    return i;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
    NODE *node = (NODE *)file->private_data;
    int i;

    if (node == NULL)
    {
        /* no channel has been set on the file descriptor */
        return -EINVAL;
    }

    if (buffer == NULL)
    {
        /* arguments validation */
        return -EINVAL;
    }

    if (length == 0 || length > 128)
    {
        /* message size error */
        return -EMSGSIZE;
    }

    for (i = 0; i < length && i < BUF_LEN; i++)
    {
        get_user(node->channel[i], &buffer[i]);
    }

    node->msg_len = i;
    return i;
}

static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param)
{
    const struct inode *inode = file->f_inode;
    unsigned int minor = iminor(inode);
    NODE *curr, *next;

    if (ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0)
    {
        /* invalid arguments when calling ioctl() */
        return -EINVAL;
    }

    if (devices[minor] == NULL)
    {
        devices[minor] = (NODE *)kmalloc(sizeof(NODE), GFP_KERNEL);
        if (devices[minor] == NULL)
        {
            /* failed allocating memory */
            return -ENOMEM;
        }
        devices[minor]->channel_id = ioctl_param;
        devices[minor]->next = NULL;
        devices[minor]->msg_len = 0;
        curr = devices[minor];
    }
    else
    {
        curr = devices[minor];
        next = curr->next;
        while (ioctl_param != curr->channel_id)
        {
            if (next == NULL)
            {
                curr->next = (NODE *)kmalloc(sizeof(NODE), GFP_KERNEL);
                if (curr->next == NULL)
                {
                    /* failed allocating memory */
                    return -ENOMEM;
                }
                curr->next->channel_id = ioctl_param;
                curr->next->next = NULL;
                curr->next->msg_len = 0;
                curr = curr->next;
                break;
            }
            curr = curr->next;
            next = curr->next;
        }
    }
    file->private_data = (void *)curr;
    return 0;
}

/* device setup */
struct file_operations fops =
    {
        .owner = THIS_MODULE, // Required for correct count of module usage. This prevents the module from being removed while used.
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .unlocked_ioctl = device_ioctl,
        .release = device_release,
};

/* loader */
static int __init msgslot_init(void)
{
    /* when soemthing fails returns non-zero value */
    int ret_register_chrdev = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &fops);
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
    int i;
    NODE *curr, *last;
    for (i = 0; i < 256; i++)
    {
        curr = NULL;
        last = devices[i];
        while (last != NULL)
        {
            curr = last;
            last = curr->next;
            kfree(curr);
        }
    }
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

/* defining initialization and exit functions of the driver */
module_init(msgslot_init);
module_exit(msgslot_exit);