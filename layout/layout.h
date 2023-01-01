#ifndef VTAGFS_LAYOUT_H_
#define VTAGFS_LAYOUT_H_
#include <linux/fs.h>

// extern struct dentry *add_tag(char *full_path, char *tag_name, unsigned int fd);
extern struct dentry *lookup_tag_root(char *tag_name);
extern struct dentry *add_tag(unsigned int fd, char *tag_name); // v1
extern struct dentry *delete_tag(unsigned int fd, char *tag_name);

extern struct dentry *lookup_dentry(struct dentry *dentry, char *name);
extern struct dentry *force_lookup(struct dentry *dentry, char *name, umode_t mode);


#endif /* VTAGFS_LAYOUT_H_ */
