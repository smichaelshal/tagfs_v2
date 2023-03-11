#include <linux/module.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include "vtagfs.h"

#define MOUNT_ARG_PERM 0660

// ioctl add tag
// ioctl delete tag
// lookup tag

extern int __init vtag_dev_init(void);
extern void __exit vtag_dev_exit(void);

char *root_tag_path = "/mnt/vtagfs";
module_param(root_tag_path, charp, MOUNT_ARG_PERM);

int vtagfs_init(void)
{
	int err;
	
	err = register_filesystem(&vtag_fs_type);
	if (err) {
		return err;
	}

	vtag_dev_init();
	start_hooks();
	return 0;
}

static void vtagfs_exit(void)
{
	close_hooks();
	unregister_filesystem(&vtag_fs_type);
	vtag_dev_exit();
}

module_init(vtagfs_init);
module_exit(vtagfs_exit);