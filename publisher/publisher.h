
#ifndef VTAGFS_PUBLISHER_H_
#define VTAGFS_PUBLISHER_H_

#define FS_NAME "vtagfs"
#define VTAGFS_PUBLISHER_MAGIC 0x817358f2

#include "../database/database.h"



extern struct file_system_type vtag_fs_type;
extern struct dentry *lookup_tagfs(struct inode *dir, struct dentry *dentry, unsigned int flags);
extern int test_links(struct dentry *dir, char *file_path);

extern struct dentry *load_datafile(struct tag_context *tag_ctx, struct datafile *datafile);

extern int tag_dir_open(struct inode *inode, struct file *file);
extern int tag_dir_close(struct inode *inode, struct file *file);
extern int tag_readdir(struct file *file, struct dir_context *ctx);




// int publish_dentry(struct dentry *dentry);

#endif /* VTAGFS_PUBLISHER_H_ */
