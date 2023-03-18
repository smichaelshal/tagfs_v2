#include <linux/cdev.h>
#include <linux/path.h>


#include "api.h"
#include "../database/database.h"
#include "../database/db_fs/db_fs.h"


static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

// struct database *db = NULL;

int tag_act_file_by_fd(unsigned int fd, char *name, unsigned int cmd);


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
    // struct tag *tag;
    struct add_single_tag *request = kzalloc(sizeof(struct add_single_tag), GFP_KERNEL);
    // struct lookup_single_tag *request2 = kzalloc(sizeof(struct lookup_single_tag), GFP_KERNEL);
    if(!request){
		return -ENOMEM;
	}

    // tag = kzalloc(sizeof(struct tag), GFP_KERNEL);
    switch (cmd) {
        case IOCTL_ADD_TAG:
            ret = copy_from_user(request, (char __user *)arg, sizeof(struct add_single_tag));
            ret = tag_act_file_by_fd(request->fd, request->tag, cmd);
            break;

        case IOCTL_DELETE_TAG:
            ret = copy_from_user(request, (char __user *)arg, sizeof(struct add_single_tag));
            ret = tag_act_file_by_fd(request->fd, request->tag, cmd);
            break;
    

        //  case IOCTL_LOOKUP_TAG: // testing, in the end this is run in the hook and not with ioctl
        //     ret = copy_from_user(request2, (char __user *)arg, sizeof(struct lookup_single_tag));
        //     // lookup_by_name(db, request2->tag, );
        //     break;
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

    // db = init_db_fs();
    // if(IS_ERR(db))
    //     return PTR_ERR(db);
    

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

struct path *get_path_by_fd(unsigned int fd){
    struct file *file;
    struct path *path;
    // struct vfsmount *mnt;

    if(!fd)
        return NULL;

    file = fget(fd);
    if(!file)
        return NULL;
    
    path = &file->f_path;
    path_get(path);
    
    fput(file);
    return path;
}

int tag_act_file_by_fd(unsigned int fd, char *name, unsigned int cmd){
    struct dentry *dentry;
    struct path *path;
    int err;
    
    path = get_path_by_fd(fd);
    if(!path)
        return -ENOENT;

    switch (cmd) {
        case IOCTL_ADD_TAG:
            err = taged_file(path->dentry, name, path->mnt);
            break;
        case IOCTL_DELETE_TAG:
            err = untaged_file(path->dentry, name, path->mnt);
            break;
    }
    
    path_put(path);
    return err;
}