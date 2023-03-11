#ifndef VTAGFS_PUBLISHER_H_
#define VTAGFS_PUBLISHER_H_

#define FS_NAME "vtagfs"
#define VTAGFS_PUBLISHER_MAGIC 0x817358f2

#include "../database/database.h"

#define SKIP_MODE 1
#define LAST_LOCKING 2 // ???

#define CURSOR_MODE 1
#define INIT_MODE 2
#define DEAD_MODE 4

#define DOT_STR "."
#define DOTDOT_STR ".."

extern struct inode *iget_generic(struct super_block *sb, unsigned long ino);

extern struct file_system_type vtag_fs_type;
// extern const struct file_operations;

extern struct dentry *lookup_tag(struct inode *dir, struct dentry *dentry, unsigned int flags);
extern struct dentry *lookup_file(struct inode *dir, struct dentry *dentry, unsigned int flags);

extern int tag_readdir(struct file *file, struct dir_context *ctx);
extern int tag_dir_open(struct inode *inode, struct file *file);
extern int tag_dir_close(struct inode *inode, struct file *file);

extern struct vtag *alloc_vtag(void);
extern struct vbranch *alloc_vbranch(void);
extern struct dentry_list *alloc_dentry_list(void);
extern struct db_tag *alloc_db_tag(void);
extern struct datafile *alloc_datafile(void);

#endif /* VTAGFS_PUBLISHER_H_ */
