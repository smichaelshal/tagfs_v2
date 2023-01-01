#ifndef VTAGFS_KERNEL_API_H_
#define VTAGFS_KERNEL_API_H_

extern int __init vtag_dev_init(void);
extern void __exit vtag_dev_exit(void);

// extern struct dentry *add_tag(char *full_path, char *tag_name);
// extern struct dentry *delete_tag(char *path, char *tag_name);
// extern struct dentry *lookup_tag_root(char *tag_name);

#endif /* VTAGFS_KERNEL_API_H_ */
