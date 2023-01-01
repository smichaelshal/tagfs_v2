#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/fsnotify.h>

#include "hardlinking.h"
#include "../layout.h"

int my_vfs_link(struct dentry *old_dentry, struct user_namespace *mnt_userns,
	     struct inode *dir, struct dentry *new_dentry,
	     struct inode **delegated_inode)
{
	struct inode *inode = old_dentry->d_inode;
	unsigned max_links = dir->i_sb->s_max_links;
	int error;

	if (!inode)
		return -ENOENT;


	

	// error = may_create(mnt_userns, dir, new_dentry);
	// if (error)
	// 	return error;

	if (dir->i_sb != inode->i_sb)
		return -EXDEV;


	/*
	 * A link to an append-only or immutable file cannot be created.
	 */
	if (IS_APPEND(inode) || IS_IMMUTABLE(inode))
		return -EPERM;

	/*
	 * Updating the link count will likely cause i_uid and i_gid to
	 * be writen back improperly if their true value is unknown to
	 * the vfs.
	 */
	if (HAS_UNMAPPED_ID(mnt_userns, inode))
		return -EPERM;
	if (!dir->i_op->link)
		return -EPERM;
	// `

	inode_lock(inode);
	/* Make sure we don't allow creating hardlink to an unlinked file */
	if (inode->i_nlink == 0 && !(inode->i_state & I_LINKABLE))
		error =  -ENOENT;
	else if (max_links && inode->i_nlink >= max_links)
		error = -EMLINK;
	else {
		error = try_break_deleg(inode, delegated_inode);
		if (!error)
			error = dir->i_op->link(old_dentry, dir, new_dentry);
	}



	if (!error && (inode->i_state & I_LINKABLE)) {
		spin_lock(&inode->i_lock);
		inode->i_state &= ~I_LINKABLE;
		spin_unlock(&inode->i_lock);
	}

	inode_unlock(inode);
	if (!error)
		fsnotify_link(dir, inode, new_dentry);

	return error;
}

struct dentry *link_any(struct dentry *old_dentry, struct dentry *dir, char *name){
    struct dentry *new_dentry;
    int err;

    new_dentry = d_alloc_name(dir, name); // <<< check if faild
	
	err = my_vfs_link(old_dentry, &init_user_ns, d_inode(dir), new_dentry, NULL);
    if(err){
		dput(new_dentry);
		return NULL;
	}
    
  
    return new_dentry;
}

