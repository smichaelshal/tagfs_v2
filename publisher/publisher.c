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


// #include "../vtagfs.h"
#include "publisher.h"
#include "../utils/utils.h"

#include "../database/database.h"



#define ROOT_TAG "/mnt/vtagfs"
#define SYMLINK_FILENAME "sym1"


#define FILENAME "/home/john/a1"
#define FILENAME2 "/home/john/a2"
#define FILENAME3 "/home/john"
#define FILENAME4 "/home/john/a3"


const struct dentry_operations tag_dentry_operations;
const struct dentry_operations ramfs_dentry_operations;
const struct dentry_operations regular_dentry_operations; // my_simple_dentry_operations

struct dentry *build_tag(char *name);
struct dentry *lookup_tag_file(struct inode *dir, struct dentry *dentry, unsigned int flags);


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
	struct dentry *d_sym = NULL;
	char *symlink_str;


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


		d_sym = d_alloc_name(dentry, SYMLINK_FILENAME);
		// d_sym->d_flags |= DCACHE_DENTRY_CURSOR;

		if(!d_sym)
			goto out;
		d_add(d_sym, NULL);

		symlink_str = join_path_str(ROOT_TAG, name);
		if(IS_ERR(symlink_str))
			goto out;

		err = dir->i_op->symlink(&init_user_ns, inode, d_sym, symlink_str);
		kfree(symlink_str);
		if(err)
			goto out;

		test_links(dentry, FILENAME);
		test_links(dentry, FILENAME2);
		test_links(dentry, FILENAME3);
		test_links(dentry, FILENAME4);
	
	}
out:
	path_put(&path_root_tag);
	return dentry;
}



struct dentry *create_vtag_dir(struct tag *tag){
	struct dentry *vdir;
	vdir = mkdir_tag(tag->name); // <<<
	vdir->d_inode->i_private = (void *)tag;
	tag->vdir = vdir;
	return vdir;
}

// return dentry success
// return NULL FAILD
struct dentry *lookup_query(struct inode *dir, struct dentry *dentry, unsigned int flags){
	struct dentry *vdir, *new_dentry;
	struct tag *tag;
	char *p;

	new_dentry = NULL;

	if (dentry->d_name.len > NAME_MAX){
		return NULL;
	}

	tag = lookup_tag(dentry->d_name.name); // ***
	if(!tag)
		return NULL;
	
	// tag->dir;
	vdir = create_vtag_dir(tag);
	return vdir; // ???
	
	// if(!strcmp(dentry->d_name.name, "red")){
	// 	pr_info("its red file\n");
	// 	new_dentry = mkdir_tag((char*)(dentry->d_name.name));
	// }

	// if(new_dentry)
	// 	return new_dentry;
	// return NULL;
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
	
	new_dentry = lookup_tag_file(dir, dentry, flags); // ***
	if(new_dentry)
		return new_dentry;
	
	return lookup_specific_fs(dir, dentry, flags);
}

struct dentry *lookup_tagfs(struct inode *dir, struct dentry *dentry, unsigned int flags){
	return lookup_wrapper(dir, dentry, flags, tmp_simple_lookup); // tmp_simple_lookup
}

struct dentry *build_tag(char *name){ // ???
	// build all structs

	//mkdir for tag in dcache

	return NULL;
}

struct dentry *lookup_tag_file(struct inode *dir, struct dentry *dentry, unsigned int flags){
	// lookup taged file if the file not found in dcache

	// struct tag *tag = (struct tag *)dir->i_private;
	// lookup_branch and load to dcache
	// lookup dentry of the file and return if found

	// pin only the required file

	return NULL;
}


struct dentry *load_datafile(struct tag *tag, struct datafile *datafile){
	struct dentry *child;
	struct inode *inode;

	inode = iget_locked(tag->dir->d_inode->i_sb, datafile->ino); // sb ???
	if(!inode)
		return ERR_PTR(-ENOENT);

	child = d_alloc_name(tag->vdir, datafile->name);
	if(!child){
		iput(inode);
		return ERR_PTR(-ENOMEM);
	}

	d_set_d_op(child, &regular_dentry_operations);
	d_add(child, inode);
	// dput(child); // unpin ???
	return child;
}