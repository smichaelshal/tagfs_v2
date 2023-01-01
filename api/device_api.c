#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include "api.h"
#include "../layout/layout.h"


static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

static int tagfs_dev_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: open()\n");
    return 0;
}
static int tagfs_dev_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: close()\n");
    return 0;
}
static ssize_t tagfs_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: read()\n");
    return 0;
}
static ssize_t tagfs_dev_write(struct file *f, const char __user *buf, size_t len,
    loff_t *off)
{
    printk(KERN_INFO "Driver: write()\n");
    return len;
}

static long tagfs_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    int ret = 0;
    struct add_single_tag *request = kzalloc(sizeof(struct add_single_tag), GFP_KERNEL);
    struct lookup_single_tag *request2 = kzalloc(sizeof(struct lookup_single_tag), GFP_KERNEL);
    if(!request){
		return -ENOMEM;
	}

    switch (cmd) {
        case IOCTL_ADD_TAG:
            ret = copy_from_user(request, (char __user *)arg, sizeof(struct add_single_tag));
            add_tag(request->fd, request->tag);
            // add_tag(request->path, request->tag, request->fd);
            break;

         case IOCTL_LOOKUP_TAG:
            ret = copy_from_user(request2, (char __user *)arg, sizeof(struct lookup_single_tag));
            lookup_tag_root(request2->tag);
            break;
    }
    kfree(request);
    return ret;
}

static struct file_operations dev_fops =
{
    .owner = THIS_MODULE,
    .open = tagfs_dev_open,
    .release = tagfs_dev_close,
    .read = tagfs_dev_read,
    .write = tagfs_dev_write,
    .unlocked_ioctl = tagfs_dev_ioctl,
};

int __init vtag_dev_init(void) /* Constructor */
{
    int ret;
    struct device *dev_ret;

    if ((ret = alloc_chrdev_region(&first, 0, 1, TAGFS_DEVICE_NAME)) < 0)
    {
        return ret;
    }
    if (IS_ERR(cl = class_create(THIS_MODULE, "chardrv")))
    {
        unregister_chrdev_region(first, 1);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, TAGFS_DEVICE_FILE_NAME)))
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return PTR_ERR(dev_ret);
    }

    cdev_init(&c_dev, &dev_fops);
    if ((ret = cdev_add(&c_dev, first, 1)) < 0)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return ret;
    }
    return 0;
}

void __exit vtag_dev_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
}

// module_init(vtag_dev_init);
// module_exit(vtag_dev_exit);

// MODULE_LICENSE("GPL");
// MODULE_AUTHOR("Anil Kumar Pugalia <email@sarika-pugs.com>");
// MODULE_DESCRIPTION("Our First Character Driver");
