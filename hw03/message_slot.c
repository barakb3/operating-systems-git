/* We are part of the kernel */
#undef __KERNEL__
#define __KERNEL__


#undef MODULE
#define MODULE

/* included for all kernel modules */
#include <linux/module.h>
/* included for __init and __exit macros */
#include <linux/init.h>

MODULE_LICENSE("GPL");

/* loader */
static int __init msgslot_init(void)
{
    /* when soemthing fails returns non-zero value */
    if (0)
    {
        return 1;
    }
    printk("Module loaded\n");
    return 0;
}

/* unloader */
static void __exit msgslot_exit(void)
{
    printk("Module unloaded\n");
}

/* Defining initialization and exit functions of the driver */
module_init(msgslot_init);
module_exit(msgslot_exit);