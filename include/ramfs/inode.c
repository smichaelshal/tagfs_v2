// copy of fs/ramfs/inode.c

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

#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <asm/signal.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/file.h>

#include <linux/fs_struct.h>

// #include "internal.h" // >>
#include "../../vtagfs.h" // <<
#include "ramfs.h" // <<

#include "../../database/database.h"

int count_red = 0;


struct ramfs_mount_opts {
	umode_t mode;
};

struct ramfs_fs_info {
	struct ramfs_mount_opts mount_opts;
};

#define RAMFS_DEFAULT_MODE 0755

static const struct super_operations ramfs_ops;
static const struct inode_operations ramfs_dir_inode_operations;


// ==================================== my code ====================================

#define DOT_STR "."
#define DOTDOT_STR ".."




int tag_readdir(struct file *file, struct dir_context *ctx); // ***
struct file *get_file_tag(struct tag *tag); // ***
struct file *get_file_branch(struct branch *branch);  // ***
int tagfs_readdir(struct file *file, struct dir_context *ctx); // ***

struct getdents_callback { // ???
	struct dir_context ctx;
	void *data;
	struct dentry *dir;
};


static int iter_tag(struct dir_context *ctx, const char *name, int len, // ***
			loff_t pos, u64 ino, unsigned int d_type)
{
	struct getdents_callback *buf =
		container_of(ctx, struct getdents_callback, ctx);
	int result = 0;
    int err;
    struct branch *branch;

	// buf->sequence++;
   
    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
       goto out;
    
	branch = alloc_branch();
	// branch->nr = name; // ??? string_to_long
	fill_branch(branch, name, buf->dir);
	// branch->dir

	buf->data = branch;
out:
	return result;
}

static int iter_branch(struct dir_context *ctx, const char *name, int len, // ***
			loff_t pos, u64 ino, unsigned int d_type){

struct getdents_callback *buf =
		container_of(ctx, struct getdents_callback, ctx);
	int result = 0;
    int err;
    struct datafile *datafile;

	// buf->sequence++;
   
    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
       goto out;
    
	datafile = alloc_datafile();
	// if(!datafile) // err <<<
	fill_datafile(datafile, name, buf->dir);

out:
	return result;
}


#include <linux/blkdev.h>
#include <linux/export.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/quotaops.h>
#include <linux/mutex.h>
#include <linux/namei.h>
#include <linux/exportfs.h>
#include <linux/writeback.h>
#include <linux/buffer_head.h> /* sync_mapping_buffers */
#include <linux/fs_context.h>
#include <linux/pseudo_fs.h>
#include <linux/fsnotify.h>
#include <linux/unicode.h>
#include <linux/fscrypt.h>

#include <linux/uaccess.h>

const struct file_operations simple_dir_operations = {
	.open		= dcache_dir_open,
	.release	= dcache_dir_close,
	.llseek		= dcache_dir_lseek,
	.read		= generic_read_dir,
	// .iterate_shared	= dcache_readdir, // <<<
	.iterate_shared	= tag_readdir, // <<<
	.fsync		= noop_fsync,
};

// =================================================================================


struct inode *ramfs_get_inode(struct super_block *sb,
				const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode * inode = new_inode(sb);

	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(&init_user_ns, inode, dir, mode);
		inode->i_mapping->a_ops = &ram_aops;
		mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
		mapping_set_unevictable(inode->i_mapping);
		inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
		switch (mode & S_IFMT) {
		default:
			init_special_inode(inode, mode, dev);
			break;
		case S_IFREG:
			inode->i_op = &ramfs_file_inode_operations;
			inode->i_fop = &ramfs_file_operations;
			break;
		case S_IFDIR:
			inode->i_op = &ramfs_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;

			/* directory inodes start off with i_nlink == 2 (for "." entry) */
			inc_nlink(inode);
			break;
		case S_IFLNK:
			inode->i_op = &page_symlink_inode_operations;
			inode_nohighmem(inode);
			break;
		}
	}
	return inode;
}

/*
 * File creation. Allocate an inode, and we're done..
 */
/* SMP-safe */
static int
ramfs_mknod(struct user_namespace *mnt_userns, struct inode *dir,
	    struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct inode * inode = ramfs_get_inode(dir->i_sb, dir, mode, dev);
	int error = -ENOSPC;

	if (inode) {
		d_instantiate(dentry, inode);
		// dget(dentry);	/* Extra count - pin the dentry in core */ // <<<
		error = 0;
		dir->i_mtime = dir->i_ctime = current_time(dir);
	}
	return error;
}

static int ramfs_mkdir(struct user_namespace *mnt_userns, struct inode *dir,
		       struct dentry *dentry, umode_t mode)
{
	int retval = ramfs_mknod(&init_user_ns, dir, dentry, mode | S_IFDIR, 0);
	if (!retval)
		inc_nlink(dir);
	return retval;
}

static int ramfs_create(struct user_namespace *mnt_userns, struct inode *dir,
			struct dentry *dentry, umode_t mode, bool excl)
{
	return ramfs_mknod(&init_user_ns, dir, dentry, mode | S_IFREG, 0);
}

static int ramfs_symlink(struct user_namespace *mnt_userns, struct inode *dir,
			 struct dentry *dentry, const char *symname)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = ramfs_get_inode(dir->i_sb, dir, S_IFLNK|S_IRWXUGO, 0);
	if (inode) {
		int l = strlen(symname)+1;
		error = page_symlink(inode, symname, l);
		if (!error) {
			d_instantiate(dentry, inode);
			// dget(dentry);  // pin ???
			dir->i_mtime = dir->i_ctime = current_time(dir);
		} else
			iput(inode);
	}
	return error;
}

static int ramfs_tmpfile(struct user_namespace *mnt_userns,
			 struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct inode *inode;

	inode = ramfs_get_inode(dir->i_sb, dir, mode, 0);
	if (!inode)
		return -ENOSPC;
	d_tmpfile(dentry, inode);

	return 0;
}

static const struct inode_operations ramfs_dir_inode_operations = {
	.create		= ramfs_create,
	.lookup		= lookup_tagfs, // simple_lookup <<
	.link		= simple_link,
	.unlink		= simple_unlink, // <<<
	.symlink	= ramfs_symlink, // <<<?
	.mkdir		= ramfs_mkdir,
	.rmdir		= simple_rmdir, // <<<
	.mknod		= ramfs_mknod,
	.rename		= simple_rename,
	.tmpfile	= ramfs_tmpfile,
};


/*
 * Display the mount options in /proc/mounts.
 */
static int ramfs_show_options(struct seq_file *m, struct dentry *root)
{
	struct ramfs_fs_info *fsi = root->d_sb->s_fs_info;

	if (fsi->mount_opts.mode != RAMFS_DEFAULT_MODE)
		seq_printf(m, ",mode=%o", fsi->mount_opts.mode);
	return 0;
}

static const struct super_operations ramfs_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
	.show_options	= ramfs_show_options,
};

enum ramfs_param {
	Opt_mode,
};

const struct fs_parameter_spec ramfs_fs_parameters[] = {
	fsparam_u32oct("mode",	Opt_mode),
	{}
};

static int ramfs_parse_param(struct fs_context *fc, struct fs_parameter *param)
{
	struct fs_parse_result result;
	struct ramfs_fs_info *fsi = fc->s_fs_info;
	int opt;

	opt = fs_parse(fc, ramfs_fs_parameters, param, &result);
	if (opt < 0) {
		/*
		 * We might like to report bad mount options here;
		 * but traditionally ramfs has ignored all mount options,
		 * and as it is used as a !CONFIG_SHMEM simple substitute
		 * for tmpfs, better continue to ignore other mount options.
		 */
		if (opt == -ENOPARAM)
			opt = 0;
		return opt;
	}

	switch (opt) {
	case Opt_mode:
		fsi->mount_opts.mode = result.uint_32 & S_IALLUGO;
		break;
	}

	return 0;
}

static int ramfs_fill_super(struct super_block *sb, struct fs_context *fc)
{
	struct ramfs_fs_info *fsi = sb->s_fs_info;
	struct inode *inode;

	sb->s_maxbytes		= MAX_LFS_FILESIZE;
	sb->s_blocksize		= PAGE_SIZE;
	sb->s_blocksize_bits	= PAGE_SHIFT;
	sb->s_magic		= VTAGFS_PUBLISHER_MAGIC; // RAMFS_MAGIC <<
	sb->s_op		= &ramfs_ops;
	sb->s_time_gran		= 1;

	inode = ramfs_get_inode(sb, NULL, S_IFDIR | fsi->mount_opts.mode, 0);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

static int ramfs_get_tree(struct fs_context *fc)
{
	return get_tree_nodev(fc, ramfs_fill_super);
}

static void ramfs_free_fc(struct fs_context *fc)
{
	kfree(fc->s_fs_info);
}

static const struct fs_context_operations ramfs_context_ops = {
	.free		= ramfs_free_fc,
	.parse_param	= ramfs_parse_param,
	.get_tree	= ramfs_get_tree,
};

int ramfs_init_fs_context(struct fs_context *fc)
{
	struct ramfs_fs_info *fsi;

	fsi = kzalloc(sizeof(*fsi), GFP_KERNEL);
	if (!fsi)
		return -ENOMEM;

	fsi->mount_opts.mode = RAMFS_DEFAULT_MODE;
	fc->s_fs_info = fsi;
	fc->ops = &ramfs_context_ops;
	return 0;
}

static void ramfs_kill_sb(struct super_block *sb)
{
	kfree(sb->s_fs_info);
	shrink_dcache_sb(sb);
	kill_litter_super(sb);
}

// static struct file_system_type ramfs_fs_type = { // >>
// 	.name		= "ramfs",
// 	.init_fs_context = ramfs_init_fs_context,
// 	.parameters	= ramfs_fs_parameters,
// 	.kill_sb	= ramfs_kill_sb,
// 	.fs_flags	= FS_USERNS_MOUNT,
// };

// static int __init init_ramfs_fs(void) // >>
// {
// 	return register_filesystem(&ramfs_fs_type);
// }
// fs_initcall(init_ramfs_fs);


struct file_system_type vtag_fs_type = {
	.name		= FS_NAME, // <<
	.init_fs_context = ramfs_init_fs_context,
	.parameters	= ramfs_fs_parameters,
	.kill_sb	= ramfs_kill_sb,
	.fs_flags	= FS_USERNS_MOUNT,
};



int tag_readdir(struct file *file, struct dir_context *ctx) {
	// try to lookup branch in dcahce, else try load branch from disk
	pr_info("aa0\n");
	

	struct tag *tag;
	struct file *tag_file;
	struct file *branch_file;
	int err;

	pr_info("bb0\n");

	struct branch *branch;
	struct datafile *datafile;

	pr_info("bb1\n");

	struct getdents_callback getdents_tag = {
		.ctx.actor = iter_tag,
		.dir = tag->dir,
	};

	pr_info("bb2\n");

	struct getdents_callback getdents_branch = {
		.ctx.actor = iter_branch,
	};

	pr_info("aa1\n");

	tag = (struct tag *)file->f_inode->i_private;
	
	
	
	if(!tag->last_branch){
		pr_info("aa2\n");

		tag_file = get_file_tag(tag);

		pr_info("aa3\n");


		if(!tag_file){
			pr_info("aa4\n");
			// return -EINVAL; // ;;;
		}

		pr_info("aa5\n");
		iterate_dir(tag_file, &getdents_tag.ctx); // get branch
		pr_info("aa6\n");
		branch = getdents_tag.data;
		pr_info("aa7\n");

				
		
		if(!branch){
			pr_info("aa8\n");
			fput(tag_file);
			tag->filp = NULL;
			// return 0; // ??? end of files in tag  // ;;;
		}
		pr_info("aa9\n");

	}else{
		pr_info("aa10\n");
		branch = tag->last_branch;
	}
	pr_info("aa11\n");
	return dcache_readdir(file, ctx); // ???

	// // if(!branch->dir) // error ???
	// getdents_branch.dir = branch->dir;  // ;;;

	// branch_file = get_file_branch(branch);
	// if(!branch_file)
	// 	return -EINVAL;

	// iterate_dir(branch_file, &getdents_branch.ctx); // get branch
	// datafile = getdents_branch.data;

	// if(!datafile){
	// 	fput(branch_file);
	// 	tag->last_branch = NULL;
	// 	return -1; // ??? pass to next branch
	// }

	// load_datafile(tag, datafile); // <<<

	// return dcache_readdir(file, ctx); // ???


	
	/*
		for branch in tag:
			for datafile in branch:
				add to dcache
	*/


	// if(!branch_file) // pass to next branch
	
	// get file
	// for each in iter:


	/*
	 lookup in dcache:
		 if dcache found and stale:
		 	delete branch

		if branch not load:
			lookup and load branch
		
		if branch load:
			lock shrink branch
		else
			add current file to dcache ???

	*/
	return 0;
}


struct file *get_file_tag(struct tag *tag){ // ???
	// return the file of <tag>/rmap
	struct file *filp;

	if(tag->filp)
		return filp;

	filp = dentry_open(&tag->path, O_RDONLY, current_cred());
	tag->filp = filp;
	return filp;
}

struct file *get_file_branch(struct branch *branch){
	// return the file of <tag>/rmap
	struct file *filp;

	if(branch->filp)
		return filp;

	
	filp = dentry_open(&branch->path, O_RDONLY, current_cred());
	branch->filp = filp;
	return filp;
}