#ifndef HARDLINKING_H_
#define HARDLINKING_H_

extern struct dentry *link_any(struct dentry *old_dentry, struct dentry *dir, char *name);
// extern int link_dir(struct inode *inode, struct dentry *dir, char *name);

// extern int link_dir(struct inode *inode, struct dentry *target);
// extern int link_dir(struct dentry *link_src, struct dentry *target, char *link_name);
// extern int unlink_dir(struct dentry *dir); // ???

#endif /* HARDLINKING_H_ */
