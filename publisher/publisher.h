
#ifndef VTAGFS_PUBLISHER_H_
#define VTAGFS_PUBLISHER_H_

#define FS_NAME "vtagfs"
#define VTAGFS_PUBLISHER_MAGIC 0x817358f2


extern struct file_system_type vtag_fs_type;
extern struct dentry *lookup_tagfs(struct inode *dir, struct dentry *dentry, unsigned int flags);
extern int send_request_tag(char *filename);

// int publish_dentry(struct dentry *dentry);

#endif /* VTAGFS_PUBLISHER_H_ */
