#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/backing-dev.h>
#include <linux/ramfs.h>
#include <linux/sched.h>
#include <linux/parser.h>
#include <linux/magic.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs_context.h>
#include <linux/fs_parser.h>
#include <linux/namei.h>
#include <linux/fs_struct.h>


#include "publisher.h"

#define ROOT_TAG "/mnt/vtagfs"
#define FILENAME "/home/john/a1"
#define FILENAME2 "/home/john/a2"
#define FILENAME3 "/home/john"
#define FILENAME4 "/home/john/a3"

const struct dentry_operations tag_dentry_operations;
const struct dentry_operations ramfs_dentry_operations;
const struct dentry_operations regular_dentry_operations; // my_simple_dentry_operations


// ==================================== debug code ====================================

int save_dentry(const struct dentry *dentry)
{
	return 0; // always save the dentry in cache
}

int my_d_revalidate(struct dentry *dentry, unsigned int flags){
// 	struct inode *inode;
// 	if(!dentry)
// 		pr_info("dentry is null\n");
// 	inode = dentry->d_inode;
// 	if(inode)
// 		pr_info("i_ino: %ld, i_nlink: %d, i_count: %d, d_lockref_count: %d\n", inode->i_ino, inode->i_nlink, inode->i_count, dentry->d_lockref.count);
// 	else
// 		pr_info("d_lockref_count: %d\n", dentry->d_lockref.count);
// out:
	return 1;
}

static void my_dentry_release(struct dentry *dentry){
	// pr_info("d_fsdata: %d\n", dentry->d_fsdata);
}

struct dentry *tmp_simple_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
	if (dentry->d_name.len > NAME_MAX)
		return ERR_PTR(-ENAMETOOLONG);
	if (!dentry->d_sb->s_d_op)
		d_set_d_op(dentry, &ramfs_dentry_operations); // <<<
	d_add(dentry, NULL);
	return NULL;
}
// =================================================================================



struct inode *get_inode_filename(char *filename, char *buffer){
	int error;
	struct path path;
	struct super_block *sb;
	struct inode *inode;
	unsigned long ino;
	const char *name;

	error = kern_path(filename, 0, &path); // LOOKUP_REVAL
	if (error)
		return NULL;

	name = path.dentry->d_name.name;
	ino = path.dentry->d_inode->i_ino;
	sb = path.dentry->d_inode->i_sb;

	inode = iget_locked(sb, ino);
	if(!inode)
		goto out;
	if(buffer)
		memcpy(buffer, name, strlen(name));
out:
	path_put(&path);
	return inode;
}

int test_links(struct dentry *dir, char *file_path){
	int err;
	struct inode *inode;
	struct dentry *child;
	char *filename = kzalloc(NAME_MAX, GFP_KERNEL);

	if(!filename){
		return -ENOMEM;
	}
	inode = get_inode_filename(file_path, filename);
	if(!inode){
		err = -1;
		goto out;
	}
	child = d_alloc_name(dir, filename);
	if(!child){
		err = -2;
		goto out;
	}
	d_set_d_op(child, &regular_dentry_operations);
	d_add(child, inode);

	spin_lock(&child->d_lock);
	child->d_fsdata = (void *)(unsigned long)(inode->i_ino);
	spin_unlock(&child->d_lock);
	dput(child);

out:
	kfree(filename);
	return err;
}

struct dentry *mkdir_tag(char *name){
	struct dentry *dentry;
	struct dentry *d_root_tag;
	struct path path_root_tag;
	struct inode *inode, *dir;
	int err;

	err = kern_path(ROOT_TAG, 0, &path_root_tag); // LOOKUP_REVAL
	if (err)
		return NULL;

	d_root_tag = path_root_tag.dentry;
	
	dentry = d_alloc_name(d_root_tag, name);

	if(!dentry)
		goto out;
	d_set_d_op(dentry, &tag_dentry_operations);

	d_add(dentry, NULL); // <<<

	dir = d_root_tag->d_inode;
	
	err = dir->i_op->mkdir(&init_user_ns, dir, dentry, 0);


	if(err){
		dentry = NULL;
	}
	else{
		spin_lock(&dentry->d_lock);
		dentry->d_fsdata = (void *)(unsigned long)(dentry->d_inode->i_ino);
		spin_unlock(&dentry->d_lock);
		inode = dentry->d_inode;

		test_links(dentry, FILENAME);
		test_links(dentry, FILENAME2);
		test_links(dentry, FILENAME3);
		test_links(dentry, FILENAME4);
	
	}
out:
	path_put(&path_root_tag);
	return dentry;
}

// return dentry success
// return NULL FAILD
struct dentry *lookup_query(struct inode *dir, struct dentry *dentry, unsigned int flags){
	struct dentry *new_dentry = NULL;

	if (dentry->d_name.len > NAME_MAX){
		return NULL;
	}
	
	if(!strcmp(dentry->d_name.name, "red")){
		pr_info("its red file\n");
		new_dentry = mkdir_tag((char*)(dentry->d_name.name));
	}

	if(new_dentry)
		return new_dentry;
	return NULL;
}

const struct dentry_operations tag_dentry_operations = {
	.d_delete = save_dentry, // in defult save, dont need this function
	.d_revalidate = my_d_revalidate, // only print
	.d_release = my_dentry_release, // only print
};

const struct dentry_operations regular_dentry_operations = {
	// .d_delete = save_dentry, // in defult save, dont need this function
	.d_revalidate = my_d_revalidate, // only print
	.d_release = my_dentry_release, // only print
	// .d_prune = my_d_prune,
};

const struct dentry_operations ramfs_dentry_operations = {
	// .d_delete = save_dentry, // in defult save, dont need this function
	.d_revalidate = my_d_revalidate, // only print
	.d_release = my_dentry_release, // only print
};


struct dentry *lookup_wrapper(struct inode *dir, struct dentry *dentry, unsigned int flags, struct dentry *(lookup_specific_fs)(struct inode *, struct dentry *, unsigned int)){
	struct dentry *new_dentry = NULL;
	new_dentry = lookup_query(dir, dentry, flags);
	if(new_dentry)
		return new_dentry;
	return lookup_specific_fs(dir, dentry, flags);
}

struct dentry *lookup_tagfs(struct inode *dir, struct dentry *dentry, unsigned int flags){
	return lookup_wrapper(dir, dentry, flags, tmp_simple_lookup); // tmp_simple_lookup
}

int send_request_tag(char *filename){
	return NULL;
}
// EXPORT_SYMBOL(send_request_tag);


// struct dentry *send_request_tag(char *filename){
// 	struct path path;
// 	get_fs_pwd(current->fs, &path);
// 	if(path.dentry){
// 		pr_info("cwd2: %s\n", path.dentry->d_name.name);
// 	}

// 	test_links(path.dentry, "/mnt/vtagfs/red/a1");
// 	path_put(&path);

// 	return NULL;

// 	// cwd -> red
// 	// cwd -> tagfs -> red
	
// 	// create new dentry of /mnt/tagfs/<filename>

// }

// EXPORT_SYMBOL(send_request_tag);
