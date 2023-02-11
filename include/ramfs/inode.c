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

#include <linux/fs_struct.h>

// #include "internal.h" // >>
#include "../../vtagfs.h" // <<
#include "ramfs.h" // <<
#include "../../database/database.h"


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

static inline unsigned char dt_type(struct inode *inode) // from libfs
{
	return (inode->i_mode >> 12) & 15;
}

// ==================================== my code ====================================

int init_tag_context(struct tag_context *tag_ctx, struct file *filp);
struct file *get_file_by_dentry(struct dentry *dir);

#define DOT_STR "."
#define DOTDOT_STR ".."
#define NR_BRANCH_READ_AHEAD 16

int tag_dir_open(struct inode *inode, struct file *file);
int tag_dir_close(struct inode *inode, struct file *file);


int tag_readdir(struct file *file, struct dir_context *ctx); // ***
struct file *get_file_tag(struct tag *tag); // ***
struct file *get_file_branch(struct branch *branch);  // ***
int tagfs_readdir(struct file *file, struct dir_context *ctx); // ***

struct getdents_callback { // ???
	struct dir_context ctx;
	struct dentry *dir;
	struct tag_context *tag_ctx;
	void *data;

	unsigned long max;
	struct tag *tag;
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
	if(!branch)
		return -ENOMEM;
	// branch->nr = name; // ??? string_to_long
	fill_branch(branch, name, buf->dir);
	list_add_tail(&branch->child, &buf->tag->sub_branchs);

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
	if(!datafile)
		return -ENOMEM;
	fill_datafile(datafile, name, buf->dir);
	load_datafile(buf->tag_ctx, datafile);
	put_datafile(datafile);

out:
	return result;
}



const struct file_operations simple_dir_operations = {
	.open		= tag_dir_open,
	.release	= tag_dir_close,
	// .llseek		= dcache_dir_lseek, // <<< todo
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



// int tag_readdir(struct file *file, struct dir_context *ctx) {
// 	// try to lookup branch in dcahce, else try load branch from disk
// 	pr_info("aa0\n");
	

// 	struct tag *tag;
// 	struct file *tag_file;
// 	struct file *branch_file;
// 	int err;

// 	pr_info("bb0\n");

// 	struct branch *branch;
// 	struct datafile *datafile;

// 	pr_info("bb1\n");


// 	pr_info("bb2\n");

// 	struct getdents_callback getdents_branch = {
// 		.ctx.actor = iter_branch,
// 	};

// 	pr_info("aa1\n");

// 	tag = (struct tag *)file->f_inode->i_private;
	
// 	struct getdents_callback getdents_tag = {
// 		.ctx.actor = iter_tag,
// 		.dir = tag->dir,
// 	};
	
	
// 	if(!tag->last_branch){
// 		pr_info("aa2\n");

// 		tag_file = get_file_tag(tag);

// 		pr_info("aa3\n");


// 		if(!tag_file){
// 			pr_info("aa4\n");
// 			// return -EINVAL; // ;;;
// 		}

// 		pr_info("aa5\n");
// 		iterate_dir(tag_file, &getdents_tag.ctx); // get branch
// 		pr_info("aa6\n");
// 		branch = getdents_tag.data;
// 		pr_info("aa7\n");

				
		
// 		if(!branch){
// 			pr_info("aa8\n");
// 			fput(tag_file);
// 			tag->filp = NULL;
// 			// return 0; // ??? end of files in tag  // ;;;
// 		}
// 		pr_info("aa9\n");

// 	}else{
// 		pr_info("aa10\n");
// 		branch = tag->last_branch;
// 	}
// 	pr_info("aa11\n");
// 	return dcache_readdir(file, ctx); // ???

// 	// // if(!branch->dir) // error ???
// 	// getdents_branch.dir = branch->dir;  // ;;;

// 	// branch_file = get_file_branch(branch);
// 	// if(!branch_file)
// 	// 	return -EINVAL;

// 	// iterate_dir(branch_file, &getdents_branch.ctx); // get branch
// 	// datafile = getdents_branch.data;

// 	// if(!datafile){
// 	// 	fput(branch_file);
// 	// 	tag->last_branch = NULL;
// 	// 	return -1; // ??? pass to next branch
// 	// }

// 	// load_datafile(tag, datafile); // <<<

// 	// return dcache_readdir(file, ctx); // ???


	
// 	/*
// 		for branch in tag:
// 			for datafile in branch:
// 				add to dcache
// 	*/


// 	// if(!branch_file) // pass to next branch
	
// 	// get file
// 	// for each in iter:


// 	/*
// 	 lookup in dcache:
// 		 if dcache found and stale:
// 		 	delete branch

// 		if branch not load:
// 			lookup and load branch
		
// 		if branch load:
// 			lock shrink branch
// 		else
// 			add current file to dcache ???

// 	*/
// 	return 0;
// }


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


int tag_dir_open(struct inode *inode, struct file *file) // <<<
{
	// alloc and fill tag_context
	struct tag_context *tag_ctx;

	tag_ctx = alloc_tag_context();
	if(!tag_ctx)
		return -ENOMEM;
	
	init_tag_context(tag_ctx, file);
	
	file->private_data = tag_ctx;
	return 0;
}


int tag_dir_close(struct inode *inode, struct file *file)  // <<<
{
	put_tag_context(file->private_data);
	return 0;
}

int scan_branch(struct dir_context *ctx, struct branch *branch, struct list_head *cursor_subdirs){  // <<<
	struct list_head *p;
	struct dentry *dentry;

	const char *name;
	int len;
	u64 ino;
	unsigned type;

	p = &branch->subdirs;

	while ((p = p->next) != &branch->subdirs) { // skip on the first becuse is empty
		dentry = list_entry(p, struct dentry_list, d_child)->dentry;

		name = dentry->d_name.name;
		len = dentry->d_name.len;
		ino = d_inode(dentry)->i_ino;
		type = dt_type(d_inode(dentry));

		if(!dir_emit(ctx, name, len, ino, type))
			break;
		ctx->pos++;

	};
	if(p == &branch->subdirs)
		return -EAGAIN;
	return 0;
}



bool is_branch_empty(struct branch *branch){ // <<<
	return &branch->subdirs == branch->subdirs.next;
}
void delete_dentry_item(struct dentry_list *dentry_item){
	dput(dentry_item->dentry);
	list_del(&dentry_item->d_child);
	kfree(dentry_item);
}



void drop_branch(struct branch *branch){ // <<<
	return;
	struct dentry_list *ptr, *next;
	make_stale(branch);
	list_for_each_entry_safe(ptr, next, &branch->subdirs, d_child){
		delete_dentry_item(ptr);
	}
}

int lock_branch(struct branch *branch){ // <<<
	struct list_head *p;
	struct dentry *dentry;
	
	if(is_branch_stale(branch))
		return -ESTALE;

	p = &branch->subdirs;

	while ((p = p->next) != &branch->subdirs) { // skip on the first becuse is empty
		dentry = list_entry(p, struct dentry_list, d_child)->dentry;
		dget(dentry);
	};

	if(is_branch_stale(branch))
		return -ESTALE;

	// struct dentry *tmp;
	// list_for_each_entry_safe(?, tmp, branch->subdirs, list)
	return 0;
}


int load_branch(struct branch *branch, struct tag_context *tag_ctx){ // <<<
	struct file *filp = NULL;
	int err;

	if(!tag_ctx->file_branch){
		filp = get_file_by_dentry(branch->dir);
		if(IS_ERR(filp))
			return PTR_ERR(filp);
		tag_ctx->file_branch = filp;
	}else{
		filp = tag_ctx->file_branch;
	}

	struct getdents_callback getdents_branch = {
		.ctx.actor = iter_branch,
		.tag_ctx = tag_ctx,
		
		.tag = tag_ctx->tag,
		.dir = tag_ctx->tag->vdir,
	};

	do {
		err = iterate_dir(filp, &getdents_branch.ctx);
		if(IS_ERR(err))
			break;
	} while(1);

	clean_stale(branch);

	return 0;
}

// struct tag *lookup_current_tag(struct tag_context *tag_ctx){
// 	// lookup in list
// 	// lookup in disk

// 	struct tag *tag = NULL;
// 	struct super_block *sb = NULL;
// 	struct database *db;

// 	if(!list_empty(tag_ctx->cursor_tag)){ // ??? to check
// 		db = list_entry(tag_ctx->cursor_tag, struct database, t_child);
// 		sb = db->sb;
// 	}

// 	return NULL;
// }


// struct branch *get_next_branch_lock(struct tag_context *tag_ctx){// <<<
// 	// todo: lookup db (disk)

	
	
// 	// lookup branch in db
// 	// lookup datafile in db
// 	return NULL;
// }




// struct database *fast_lookup_db(struct tag_context *tag_ctx){ // ??? <<<
// 	return NULL;
// }

// struct database *slow_lookup_db(struct tag_context *tag_ctx){
// 	return NULL;
// }



struct branch *fast_lookup_branch(struct tag_context *tag_ctx){
	struct branch *branch = NULL;
	int err;


	if(!tag_ctx){
		return NULL;
	}

	if(!tag_ctx->cursor_branchs){
		return NULL;
	}
	
	if(!list_empty(tag_ctx->cursor_branchs)){
		branch = list_entry(tag_ctx->cursor_branchs, struct branch, child);
	} // ??? to check
		

	if(!branch)
		return ERR_PTR(-ENOENT);


	if(is_branch_empty(branch))
		return ERR_PTR(-EAGAIN); // The branch has not been loaded yet

	if(is_branch_stale(branch)){
		goto out_err;
		err = -ESTALE;
	}
	
	err = lock_branch(branch);
	if(IS_ERR(err)){
		goto out_err;
	}
	return branch;
out_err:
	drop_branch(branch);
	return ERR_PTR(err);
}

struct file *get_file_by_dentry(struct dentry *dir){
    struct file *filp;
    struct path path;
    char *buff_path, *full_path;
    int err;

    if(!dir)
        return ERR_PTR(-EINVAL);
    
    buff_path = kzalloc(PATH_MAX, GFP_KERNEL);
    if(!buff_path)
        return ERR_PTR(-ENOMEM);
    
    full_path = dentry_path_raw(dir, buff_path, PATH_MAX);
    if(!IS_ERR(full_path)){
        err = kern_path(full_path, 0, &path);
        if(IS_ERR(err)){
            filp = ERR_PTR(err);
            goto out;
        }
    }else{
        filp = (struct file *)full_path;
        goto out;
    }

    filp = dentry_open(&path, O_RDONLY, current_cred());
    path_put(&path);

out:
    kfree(buff_path);
    return filp;
}

// struct file *get_file_tag();

struct file *open_tag_file_by_sb(struct super_block *sb, char *name){
	struct dentry *dir_tag, *dmap, *rmap;
	struct file *filp;
    int err;

    dir_tag = db_lookup_dentry(sb->s_root, name);
    if(!dir_tag)
        return NULL;

	rmap = db_lookup_dentry(dir_tag, RMAP_DIR_NAME);
	dput(dir_tag);
    if(!rmap)
        return ERR_PTR(-ENOENT);
	
	filp = get_file_by_dentry(rmap);
	dput(rmap);
	return filp;
}

int readahead_branchs(struct tag_context *tag_ctx, unsigned long count){
	// append x init_branch to the list of branch and return the count entries we read.
	
	struct database *db;
	struct super_block *sb;
	struct branch *branch;
	struct file *filp;
	int err;
	unsigned long start_count = count;

	
	if(!tag_ctx->file_tag){
		db = list_entry(tag_ctx->cursor_tag, struct database, t_child);
		sb = db->sb;
		
		filp = open_tag_file_by_sb(sb, tag_ctx->tag->name);
		if(IS_ERR(filp))
			return PTR_ERR(filp);
	}
	
	tag_ctx->file_tag = filp;

	struct getdents_callback getdents_tag = {
		.ctx.actor = iter_tag,
		.tag = tag_ctx->tag,
		// .max = count,
		.dir = tag_ctx->tag->vdir,
	};
	while(count--){
		err = iterate_dir(tag_ctx->file_tag, &getdents_tag.ctx);
		if(IS_ERR(err)){
			break;
		}
	}

	return start_count - count;
}

struct branch *slow_lookup_branch(struct tag_context *tag_ctx, int status){
	struct branch *branch;
	int count;

	if(!(status == -EAGAIN || status == -ESTALE)){
		count = readahead_branchs(tag_ctx, NR_BRANCH_READ_AHEAD);
		if(IS_ERR(count)){
			return ERR_PTR(count);
		}else if(count != NR_BRANCH_READ_AHEAD){
			tag_ctx->cursor_tag = tag_ctx->cursor_tag->next;
			if(tag_ctx->cursor_tag == &tag_ctx->tag->dbs){
				return ERR_PTR(-ENOENT); // no more branch at all
			}
			return ERR_PTR(-EAGAIN); // no more branch in this db
		}
	}else{ // else if(status == -ENOENT){}// ???
		// readahead branchs data
		branch = list_entry(tag_ctx->cursor_branchs, struct branch, child);
		load_branch(branch, tag_ctx);
	}
	return branch;
}

struct branch *get_current_branch_lock(struct tag_context *tag_ctx){
	struct database *db;
	struct branch *branch;
	int err;
	// Prepare all dentry of branch to read
	
	// lookup branch: 
		//  if the end of branch current unlock and found the next
	// lock branch
	// load all dentry of branch

	// db = fast_lookup_db(tag_ctx);
	// if(!db)
	// 	slow_lookup_db(tag_ctx);

	branch = fast_lookup_branch(tag_ctx);
	// branch = NULL;
	if(IS_ERR_OR_NULL(branch))
		branch = slow_lookup_branch(tag_ctx, (int)branch);

	// branch = lookup_current_branch(tag_ctx);
	if(IS_ERR(branch))
		return branch; // this is ptr err

	
	return NULL;
}



int tag_readdir(struct file *file, struct dir_context *ctx) {
	struct tag_context *tag_ctx;
	int err;

	struct branch *branch;

	struct file *tag_file;
	struct file *branch_file;
	struct tag *tag;
	struct datafile *datafile;



	tag_ctx = file->private_data;

	branch = get_current_branch_lock(tag_ctx);
	err = scan_branch(ctx, branch, &tag_ctx->cursor_subdirs);
	if(IS_ERR(err)){
		if(err == -EAGAIN){
			tag_ctx->cursor_branchs = tag_ctx->cursor_branchs->next; // update to next branch
		}
	}

	return 0;
}


int init_tag_context(struct tag_context *tag_ctx, struct file *filp){
	struct dentry *dir;
	struct tag *tag;

	// INIT_LIST_HEAD(&tag_ctx->cursor_tag);
	// INIT_LIST_HEAD(&tag_ctx->cursor_branchs);
	INIT_LIST_HEAD(&tag_ctx->cursor_subdirs);
	

	dir = dget_parent(filp->f_path.dentry);
	pr_info("h2.1: %s\n", dir->d_name.name);
	pr_info("h2.2: %s\n", filp->f_path.dentry->d_name.name);
	if(!dir){
		return -ENOMEM; // ???
	}
	

	long magic;
	tag = (struct tag *)dir->d_inode->i_private;
	if(tag){
		magic = tag->magic;
		pr_info("magic1: %ld\n", magic);
	}else{
		pr_info("no magic1\n");
	}
	
	if(tag){
		return -ENOMEM; // ???
	}


	tag_ctx->tag = tag;
	tag_ctx->cursor_tag = &tag->dbs;
	tag_ctx->cursor_branchs = &tag->sub_branchs;

	tag_ctx->file_tag = NULL;
	tag_ctx->file_branch = NULL;
	
	
	dput(dir);
	return 0;
}