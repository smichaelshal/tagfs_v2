// publisher.c
#include <linux/fs_struct.h>
#include "publisher.h"
#include "../utils/utils.h"
#include "../database/database.h"

#define ROOT_TAG "/mnt/vtagfs"
#define SYMLINK_FILENAME "sym1"

#define DOT_STR "."
#define DOTDOT_STR ".."
#define NR_BRANCH_READ_AHEAD 16

struct file *get_file_by_dentry(struct dentry *dir);
struct file *get_file_by_dentry_dir(struct dentry *dir);

struct getdents_callback { // ???
	struct dir_context ctx;
	struct dentry *dir;
	struct tag_context *tag_ctx;
	void *data;
	struct vtag *tag;
	long sequence;
};

const struct dentry_operations ramfs_dentry_operations;
const struct dentry_operations tag_dentry_operations;
const struct dentry_operations regular_dentry_operations; // my_simple_dentry_operations

int tag_dir_open(struct inode *inode, struct file *file);
int tag_dir_close(struct inode *inode, struct file *file);
int tag_readdir(struct file *file, struct dir_context *ctx);

static inline unsigned char dt_type(struct inode *inode) { // from libfs
	return (inode->i_mode >> 12) & 15;
}

struct dentry *lookup_tag_file(struct inode *dir, struct dentry *dentry, unsigned int flags, struct vtag *tag);

int init_tag_context(struct tag_context *tag_ctx, struct file *filp);

struct file *__get_file_by_dentry(struct dentry *dir, bool is_dir);

int my_d_revalidate(struct dentry *dentry, unsigned int flags){
	return 1;
}

static void my_dentry_release(struct dentry *dentry){
    return;
}

int save_dentry(const struct dentry *dentry){
	return 0; // always save the dentry in cache
}

int reg_revalidate(struct dentry *dentry, unsigned int flags){
	return !is_branch_stale((struct branch*)dentry->d_fsdata);
}

int tag_revalidate(struct dentry *dentry, unsigned int flags){
	return !list_empty(&dentry->d_subdirs); // todo: check if the tag is empty always, 0 files taged
}

static void reg_release(struct dentry *dentry){
	struct branch *branch = (struct branch *)dentry->d_fsdata;
	if(!is_branch_stale(branch))
		make_stale(branch);

	// if last in branch free branch. // <<<<
}

static void d_release_vtag(struct dentry *dentry){
	pr_info("start d_release_vtag\n");
	put_vtag((struct vtag *)dentry->d_fsdata);
}


const struct dentry_operations tag_dentry_operations = {
	.d_delete = save_dentry, // in defult save, dont need this function
	.d_revalidate = my_d_revalidate, // only print
	
	.d_release = d_release_vtag, // only print
	// .d_revalidate = tag_revalidate
};

const struct dentry_operations regular_dentry_operations = {
	.d_revalidate = my_d_revalidate, // only print
	.d_release = my_dentry_release, // only print

	// .d_release = reg_release,
    // .d_revalidate = reg_revalidate,
};

const struct dentry_operations ramfs_dentry_operations = {
	// .d_delete = save_dentry, // in defult save, dont need this function
	.d_revalidate = my_d_revalidate,
	.d_release = my_dentry_release, // only print
};

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

struct dentry *mkdir_tag(char *name){
	pr_info("start mkdir_tag\n");
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
	// tagfs_wrapper_mkdir

	if(err){
		dentry = NULL;
	}
	else{
		// spin_lock(&dentry->d_lock);
		// dentry->d_fsdata = (void *)(unsigned long)(dentry->d_inode->i_ino);
		// spin_unlock(&dentry->d_lock);
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

	}
out:
	path_put(&path_root_tag);
	return dentry;
}

struct dentry *create_vtag_dir(struct vtag *tag){
	pr_info("start create_vtag_dir\n");
	struct dentry *vdir;
	vdir = mkdir_tag(tag->name); // <<<
	// vdir->d_inode->i_private = (void *)tag;
	return vdir;
}

struct vtag *init_tag(char *name){
	pr_info("start init_tag\n");
	struct vtag *vtag;
	char *buff_name;
	struct path root;
	struct super_block *sb;
	struct db_tag *db_tag;

	buff_name = kzalloc(strlen(name), GFP_KERNEL);
	if(!buff_name)
		return -ENOMEM;
	
	strcpy(buff_name, name);
	vtag = alloc_vtag();
	vtag->name = buff_name;
	vtag->magic = TAG_MAGIC;

// 	if(!tag)
// 		return ERR_PTR(-ENOMEM);
	
	INIT_LIST_HEAD(&vtag->dbs);
    INIT_LIST_HEAD(&vtag->sub_branchs);

	root = current->fs->root; // <<<< <=
	sb = root.dentry->d_inode->i_sb;
	// db_tag = lookup_tag_by_sb(sb, name); // <<<< <= ????
	list_add_sb(&vtag->dbs, sb);


	struct database *tmp_db = list_entry(vtag->dbs.next, struct database, t_child);
	pr_info("create_vtag_dir: cursor_tag: name2: %s\n", tmp_db->sb->s_root->d_name.name);

	return vtag;
}

// return dentry success
// return NULL FAILD

struct dentry *lookup_query(struct inode *dir, struct dentry *dentry, unsigned int flags){
	pr_info("start lookup_query\n");
	struct dentry *vdir, *new_dentry;
	struct vtag *tag;
	// char *p, *tmp;

	new_dentry = NULL;

	if (dentry->d_name.len > NAME_MAX){ // is needed ???
		return NULL;
	}

	// tmp = dentry->d_name.name;
	tag = init_tag(dentry->d_name.name);
	if(IS_ERR(tag))
		return NULL;
	
	// tag->dir;
	vdir = create_vtag_dir(tag);
	tag->vdir = vdir;
	vdir->d_fsdata = (void *)tag;
	vdir->d_inode->i_private = (void *)tag->magic;

	struct database *tmp_db = list_entry(tag->dbs.next, struct database, t_child);
	pr_info("lookup_query: cursor_tag: name4: %s\n", tmp_db->sb->s_root->d_name.name);

	return vdir; // ???
	
	// if(!strcmp(dentry->d_name.name, "red")){
	// 	pr_info("its red file\n");
	// 	new_dentry = mkdir_tag((char*)(dentry->d_name.name));
	// }

	// if(new_dentry)
	// 	return new_dentry;
	// return NULL;
}

struct dentry *lookup_wrapper(struct inode *dir, struct dentry *dentry, unsigned int flags, struct dentry *(lookup_specific_fs)(struct inode *, struct dentry *, unsigned int)){
	struct dentry *new_dentry = NULL;
	struct dentry *alias;

	pr_info("start lookup_wrapper\n");

	alias = d_find_any_alias(dir);

	// if(dir->i_private && sizeof(dir->i_private) == sizeof(struct tag) && ((struct vtag *)dir->i_private)->magic == TAG_MAGIC){ // <<<<
	if(alias && (alias->d_fsdata) && (long)dir->i_private == TAG_MAGIC){ // <<<< <=
		new_dentry = lookup_tag_file(dir, dentry, flags, (struct vtag *)alias->d_fsdata); // ***
		if(IS_ERR_OR_NULL(new_dentry))
			goto out;
		else
			return new_dentry;
		
	}
	new_dentry = lookup_query(dir, dentry, flags);
	if(new_dentry){
		return new_dentry;
	}
out:
	return lookup_specific_fs(dir, dentry, flags);
}

struct dentry *lookup_tagfs(struct inode *dir, struct dentry *dentry, unsigned int flags){
	pr_info("start lookup_tagfs\n");
	return lookup_wrapper(dir, dentry, flags, tmp_simple_lookup); // tmp_simple_lookup
}

struct dentry *load_datafile_file(struct vtag *vtag, struct datafile *datafile){
	pr_info("start load_datafile_file\n");
	struct dentry *child;
	struct inode *inode;

	if(!vtag)
		pr_info("vtag is null\n");

	if(!datafile)
		pr_info("datafile is null\n");

	pr_info("vtag name: %s\n", vtag->name);
	pr_info("datafile ino: %ld\n", datafile->ino);

	if(!datafile->sb)
		pr_info("datafile->sb is null\n");
	

	pr_info("sb name: %s\n", datafile->sb->s_root->d_name.name);
	// return ERR_PTR(-ENOMEM); // delete

	inode = iget_locked(datafile->sb, datafile->ino); // sb ???
	if(!inode)
		return ERR_PTR(-ENOENT);

	child = d_alloc_name(vtag->vdir, datafile->name);
	if(!child){
		iput(inode);
		return ERR_PTR(-ENOMEM);
		// if(!(child = db_lookup_dentry_share(vtag->vdir, datafile->name)))  // double name file
		// 	return ERR_PTR(-ENOMEM);
		// else if(!child->d_lockref.count)
		// 	dget(child); // pin dentry
	}
	d_set_d_op(child, &regular_dentry_operations);
	d_add(child, inode);

    // child->d_fsdata = (void *)branch;

	return child;

}

struct dentry *lookup_file_dmap(struct super_block *sb, char *name, struct vtag *tag){
	struct dentry *dentry, *dmap, *d_tag;
	struct datafile *datafile;
	int err;

	pr_info("start lookup_file_dmap\n");

	// db_tag = lookup_tag_by_sb(sb, name); // <= ???
	
	d_tag = db_lookup_dentry_share(sb->s_root, tag->name);
	if(!d_tag)
		return ERR_PTR(-ENOENT);

	dmap = db_lookup_dentry_share(d_tag, DMAP_DIR_NAME);
	dput(d_tag);
	if(!dmap)
        return ERR_PTR(-ENOENT);

	// dentry = db_lookup_dentry(dmap, name);
	// dput(dmap);

	datafile = alloc_datafile();
	if(!datafile)
		return ERR_PTR(-ENOENT);
	
	err = fill_datafile(datafile, name, dmap);
	if(IS_ERR(err))
		return ERR_PTR(err);
	datafile->sb = sb;

	// dentry = load_datafile_file(tag, datafile); // <<<< <=
	dentry = load_datafile_file(tag, datafile); // <<<< <=
	put_datafile(datafile);

	return dentry;
}

struct dentry *lookup_tag_file(struct inode *dir, struct dentry *dentry, unsigned int flags, struct vtag *tag){
	
	// lookup taged file if the file not found in dcache

	// struct vtag *tag = (struct vtag *)dir->i_private;
	// lookup_branch and load to dcache
	// lookup dentry of the file and return if found

	// pin only the required file

	// struct tag_context *tag_ctx;
	// tag_ctx = alloc_tag_context();

	struct database *db;
	struct dentry *dentry_file = NULL;
	pr_info("start lookup_tag_file\n");
	list_for_each_entry(db, &tag->dbs, t_child) {
		// dentry_file = lookup_file_dmap(db->sb, dentry_file->d_name.name);
		dentry_file = lookup_file_dmap(db->sb, dentry->d_name.name, tag);
		if(!IS_ERR_OR_NULL(dentry_file))
			return dentry_file;
	}
	return NULL;
}

int subdirs_add_dentry(struct list_head *list, struct dentry *dentry){
    struct dentry_list *dentry_list1;
	pr_info("start subdirs_add_dentry\n");
	// if(list_empty(list)){
	// 	dentry_list = list_entry(list, struct dentry_list, d_child);
	// 	dentry_list->dentry = dentry;
	// 	return 0;
	// }
	dentry_list1 = kzalloc(sizeof(struct dentry_list), GFP_KERNEL);
    if(!dentry_list1)
        return -ENOMEM;

	if(!dentry_list1)
		pr_info("dentry_list1 is null\n");

	pr_info("p0\n");

    INIT_LIST_HEAD(&dentry_list1->d_child);

	pr_info("p1\n");

	if(!dentry_list1)
		pr_info("dentry_list2 is null\n");

    dentry_list1->dentry = dentry;
	pr_info("p2\n");

	if(!list)
		pr_info("list is null\n");

	if(!dentry_list1)
		pr_info("dentry_list3 is null\n");

	if(list_empty(list))
		pr_info("list is empty\n");

	pr_info("p3\n");
	

	if(!dentry_list1)
		pr_info("dentry_list4 is null\n");

	pr_info("p4\n");
	if(list_empty(&dentry_list1->d_child))
		pr_info("dentry_list is empty\n");

	pr_info("p5\n");

	
    list_add_tail(&dentry_list1->d_child, list);
    // list_add(&dentry_list1->d_child, list);

	

	pr_info("end subdirs_add_dentry\n");
    return 0;
}

struct dentry *load_datafile(struct tag_context *tag_ctx, struct datafile *datafile){
	struct dentry *child;
	struct inode *inode;
	struct vtag *tag;
	struct super_block *sb;
	struct branch *branch;

	pr_info("start load_datafile\n");
	
	branch = list_entry(tag_ctx->cursor_branchs->next, struct branch, child);
	sb = tag_ctx->file_tag->f_path.dentry->d_inode->i_sb;


	// if(!vtag)
	// 	pr_info("vtag is null\n");

	if(!datafile)
		pr_info("datafile is null_\n");

	// pr_info("vtag name: %s\n", vtag->name);
	pr_info("datafile ino_: %ld\n", datafile->ino);

	if(!datafile->sb)
		pr_info("datafile->sb is null_\n");
	
	pr_info("sb name_: %s\n", datafile->sb->s_root->d_name.name);
	pr_info("sb magic_: %ld\n", datafile->sb->s_magic);
	// return ERR_PTR(-ENOMEM); // delete


	pr_info("i0\n");
	inode = iget_locked(sb, datafile->ino); // sb ???
	pr_info("i0.1\n");

	if(!inode)
		return ERR_PTR(-ENOENT);

	pr_info("i0.2\n");
	tag = tag_ctx->vtag;
	pr_info("i0.3\n");

	child = d_alloc_name(tag->vdir, datafile->name);
	pr_info("i1\n");
	if(!child){
		iput(inode);
		return ERR_PTR(-ENOMEM);
		// if(!(child = db_lookup_dentry(sb->s_root, datafile->name)))
		// if(!(child = db_lookup_dentry_share(tag->vdir, datafile->name))) // double name file
		// 	return ERR_PTR(-ENOMEM);
		// else if(!child->d_lockref.count)
		// 	dget(child); // pin dentry
		
		
	}
	pr_info("i2\n");
	child->d_fsdata = (void *)branch;

	pr_info("i3\n");
	d_set_d_op(child, &regular_dentry_operations);
	pr_info("i4\n");
	d_add(child, inode);
	pr_info("i5\n");
	pr_info("inode ino: %ld\n", inode->i_ino);
	pr_info("child name: %s\n", child->d_name.name);

	if(!&branch->subdirs)
		pr_info("branch->subdirs_1 is null\n");

	pr_info("i5.1\n");

	if(list_empty(&branch->subdirs))
		pr_info("branch->subdirs_1 is empty\n");

	pr_info("i5.2\n");
	
	subdirs_add_dentry(&branch->subdirs ,child);
	pr_info("i6\n");

	// dput(child); // unpin ???
	return child;
}

// -------------------- section 1 --------------------


int tag_dir_open(struct inode *inode, struct file *file) // <<<
{
	// alloc and fill tag_context
	struct tag_context *tag_ctx;
	pr_info("start tag_dir_open\n");

	tag_ctx = alloc_tag_context();
	if(!tag_ctx)
		return -ENOMEM;
	
	init_tag_context(tag_ctx, file);
	
	file->private_data = tag_ctx;
	return 0;
}

int tag_dir_close(struct inode *inode, struct file *file)  // <<<
{
	pr_info("start tag_dir_close\n");
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

	pr_info("start scan_branch\n");
	// p = &branch->subdirs->next;
	if(!branch)
		pr_info("branch is null\n");

	pr_info("scan_branch 0.1\n");
	

	if(!&branch->subdirs)
		pr_info("branch->subdirs is null\n");

	pr_info("scan_branch 0.2\n");
	

	// p = branch->subdirs.prev;
	p = branch->subdirs.next;
	// p = tag_ctx->cursor_branchs->next;

	if(!p)
		pr_info("p is null\n");
	
	pr_info("scan_branch 0.3\n");

	if(!&branch->subdirs)
		pr_info("subdirs is null\n");

	pr_info("scan_branch 0.1.1\n");

	if(list_empty(&branch->subdirs))
		pr_info("subdirs is empty\n");
		
	pr_info("scan_branch 0.2.1\n");
	// p = p->next;

	pr_info("scan_branch 0.3\n");

	struct list_head *tmp1 = &branch->subdirs;

	pr_info("scan_branch 0.4\n");


	while (1) { // skip on the first becuse is empty
		pr_info("scan_branch 0.5\n");

		if((p) == tmp1){
			pr_info("while0.1\n");
			break;
		}else{
			pr_info("while0.2\n");
		}
		
		pr_info("while0.3\n");
		dentry = list_entry(p, struct dentry_list, d_child)->dentry;

		name = dentry->d_name.name;
		len = dentry->d_name.len;
		ino = d_inode(dentry)->i_ino;
		type = dt_type(d_inode(dentry));

		pr_info("while1: %s\n", name);

		if(!dir_emit(ctx, name, len, ino, type))
			break;
        else{
            pr_info("emit: name: %s, ino: %ld\n", name, ino);
        }
		ctx->pos++;
		p = p->next;

	};
	pr_info("end while\n");
	if(p == &branch->subdirs){
		pr_info("end while0.1\n");
		return -EAGAIN;
	}
	pr_info("end scan_branch\n");
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
	struct dentry_list *ptr, *next;
	pr_info("start drop_branch\n");
	
	make_stale(branch);
	list_for_each_entry_safe(ptr, next, &branch->subdirs, d_child){
		delete_dentry_item(ptr);
	}
}

int lock_branch(struct branch *branch){ // <<<
	pr_info("start lock_branch\n");
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

static int iter_branch(struct dir_context *ctx, const char *name, int len, // ***
			loff_t pos, u64 ino, unsigned int d_type){

struct getdents_callback *buf =
		container_of(ctx, struct getdents_callback, ctx);
	int result = 0;
    int err;
    struct datafile *datafile;

	pr_info("start iter_branch\n");
	buf->sequence++;

   
    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR)){
		pr_info("out iter_branch\n");
		goto out;
	}
    
	datafile = alloc_datafile();
	if(!datafile)
		return -ENOMEM;
	err = fill_datafile(datafile, name, buf->dir);
	if(IS_ERR(err))
		return -ENOMEM; // ????
	datafile->sb = list_entry(buf->tag_ctx->cursor_tag, struct database, t_child)->sb;
	pr_info("iter_branch0\n");

	load_datafile(buf->tag_ctx, datafile);

	pr_info("iter_branch1\n");
	
	// put_datafile(datafile); // ????

	pr_info("end iter_branch\n");

out:
	return result;
}

int load_branch(struct branch *branch, struct tag_context *tag_ctx){ // <<<
	struct file *filp = NULL;
	int err;

	pr_info("start load_branch\n");
	pr_info("e0\n");

	if(!tag_ctx->file_branch){
		pr_info("e1\n");
		filp = get_file_by_dentry_dir(branch->dir);
		if(IS_ERR(filp)){
			pr_info("e2\n");
			return PTR_ERR(filp);
		}
		pr_info("e3\n");
		tag_ctx->file_branch = filp;
		pr_info("e4\n");
	
	}else{
		pr_info("e5\n");
		filp = tag_ctx->file_branch;
	}
	pr_info("e6\n");

	struct getdents_callback getdents_branch = {
		.ctx.actor = iter_branch,
		.tag_ctx = tag_ctx,
		.tag = tag_ctx->vtag,
		.dir = branch->dir,
	};
	pr_info("e7\n");

	getdents_branch.sequence = 0;


	do {
		long old_seq = getdents_branch.sequence;
		pr_info("iterate_dir_branch_1\n");
		pr_info("sequence_v1: %ld\n", getdents_branch.sequence);
		err = iterate_dir(filp, &getdents_branch.ctx);
		pr_info("iterate_dir_branch_2\n");
		if(IS_ERR(err))
			break;
		
		if(old_seq == getdents_branch.sequence){
			err = 0;
			break;
		}
	pr_info("e8\n");
	} while(1);
	pr_info("e9\n");

	clean_stale(branch);
	pr_info("e10\n");

	return 0;
}

struct branch *fast_lookup_branch(struct tag_context *tag_ctx){
	struct branch *branch = NULL;
	int err;
	pr_info("start fast_lookup_branch\n");

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
		err = -ESTALE;
		goto out_err;
	}
	
	err = lock_branch(branch);
	if(IS_ERR(err)){
		goto out_err;
	}
	return branch;
	pr_info("end fast_lookup_branch\n");
out_err:
	drop_branch(branch);
	pr_info("err fast_lookup_branch\n");
	return ERR_PTR(err);
}
struct file *get_file_by_dentry(struct dentry *dir){
	return __get_file_by_dentry(dir, false);
}

struct file *get_file_by_dentry_dir(struct dentry *dir){
	return __get_file_by_dentry(dir, true);
}

struct file *__get_file_by_dentry(struct dentry *dir, bool is_dir){
    struct file *filp;
    struct path path;
    char *buff_path, *full_path;
    int err;

	

	if(is_dir)
		pr_info("start __get_file_by_dentry dir\n");
	else
		pr_info("start __get_file_by_dentry not dir\n");

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

	if(is_dir)
    	filp = dentry_open(&path, O_RDONLY | O_DIRECTORY, current_cred());
	else
    	filp = dentry_open(&path, O_RDONLY, current_cred());

    path_put(&path);

out:
    kfree(buff_path);
    return filp;
}

// struct file *get_file_tag();

struct dentry *lookup_rmap_dentry(struct dentry *dir_tag, char *name){
	struct dentry *rmap, *tmp;
	struct file *filp;
    int err;
	pr_info("start lookup_rmap_dentry\n");

	rmap = db_lookup_dentry_share(dir_tag, RMAP_DIR_NAME);
	dput(dir_tag);
    if(!rmap)
        return ERR_PTR(-ENOENT);
	return rmap;
}

struct dentry *lookup_rmap_dentry_by_sb(struct super_block *sb, char *name){
	struct dentry *dir_tag, *rmap, *tmp;
	struct file *filp;
    int err;

	pr_info("start lookup_rmap_dentry_by_sb\n");

	tmp = dget(sb->s_root);

	dir_tag = db_lookup_dentry_share(tmp, name);
	dput(tmp);
    if(!dir_tag)
        return NULL;

	rmap = lookup_rmap_dentry(dir_tag, name);
	dput(dir_tag);
	if(IS_ERR(rmap))
        return ERR_PTR(-ENOENT);
	
	return rmap;
}

struct file *open_tag_file_by_sb(struct super_block *sb, char *name){
	struct dentry *dir_tag, *dmap, *rmap, *tmp;
	struct file *filp;
    int err;

	pr_info("start open_tag_file_by_sb\n");

	tmp = dget(sb->s_root);

	dir_tag = db_lookup_dentry_share(tmp, name);
	dput(tmp);
    if(!dir_tag)
        return NULL;

	rmap = lookup_rmap_dentry(dir_tag, name);
	dput(dir_tag);
    if(IS_ERR(rmap))
        return ERR_PTR(-ENOENT);

	filp = get_file_by_dentry_dir(rmap);

	pr_info("rmap count: %d\n", rmap->d_lockref.count);
	dput(rmap);
	return filp;
}

static int iter_tag(struct dir_context *ctx, const char *name, int len, // ***
			loff_t pos, u64 ino, unsigned int d_type)
{
	struct getdents_callback *buf =
		container_of(ctx, struct getdents_callback, ctx);
    int err;
    struct branch *branch;
	int result = 0;
	pr_info("start iter_tag\n");
	
	buf->sequence++;



   
    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR)){
		pr_info("out iter_tag\n");
		goto out;
	}
    
	branch = alloc_branch();
	if(!branch)
		return -ENOMEM;
	// branch->nr = name; // ??? string_to_long
	fill_branch(branch, name, buf->dir);
	pr_info("before add branch1\n");
	// list_add_tail(&branch->child, &buf->tag->sub_branchs);
	list_add(&branch->child, &buf->tag->sub_branchs);
	pr_info("after add branch1\n");

	buf->data = branch;
	
out:
	return result;
}

int readahead_branchs(struct tag_context *tag_ctx, unsigned long count){
	// append x init_branch to the list of branch and return the count entries we read.
	
	struct database *db;
	struct super_block *sb;
	struct branch *branch;
	struct file *filp;
	int err;

	pr_info("start readahead_branchs\n");
	
	if(!tag_ctx->file_tag){
		db = list_entry(tag_ctx->cursor_tag, struct database, t_child);
		sb = db->sb;
		
		filp = open_tag_file_by_sb(sb, tag_ctx->vtag->name);
		if(IS_ERR_OR_NULL(filp))
			return PTR_ERR(filp);
	}
	tag_ctx->file_tag = filp;

	pr_info("f_path dentry count: %d\n", tag_ctx->file_tag->f_path.dentry->d_lockref.count);
	
	struct getdents_callback getdents_tag = {
		.ctx.actor = iter_tag,
		.tag = tag_ctx->vtag,
		.dir = dget(tag_ctx->file_tag->f_path.dentry),
		// .dir = tag_ctx->vtag->vdir,
	};
	// fput(tag_ctx->file_tag);
	pr_info("iterate_dir_tag_0: %ld\n", count);

	getdents_tag.sequence = 0;
	
	while(count--){
		long old_seq = getdents_tag.sequence;
		pr_info("seq_v1: %ld\n", getdents_tag.sequence);
		err = iterate_dir(tag_ctx->file_tag, &getdents_tag.ctx);

		if(IS_ERR(err))
			break;

		if(old_seq == getdents_tag.sequence){
			err = 0;
			break;
		}
		err = count;
	}
	return err;
}

struct branch *slow_lookup_branch(struct tag_context *tag_ctx, int status){
	struct branch *branch;
	int count;

	pr_info("start slow_lookup_branch\n");

	if(!(status == -EAGAIN || status == -ESTALE)){
		pr_info("a0\n");
		count = readahead_branchs(tag_ctx, NR_BRANCH_READ_AHEAD);
		pr_info("a0.1: count %d\n", count);
		pr_info("a1\n");
		if(IS_ERR(count)){
			return ERR_PTR(count);
		}else if(!count){
			pr_info("a1.1\n");
			tag_ctx->cursor_tag = tag_ctx->cursor_tag->next;
			// if(tag_ctx->cursor_tag == &tag_ctx->vtag->dbs){
			if(tag_ctx->cursor_tag == &tag_ctx->vtag->dbs || tag_ctx->cursor_tag == tag_ctx->cursor_tag->next){
				pr_info("a2\n");
                tag_ctx->cursor_tag = tag_ctx->cursor_tag->prev; // ????
				return ERR_PTR(-ENOENT); // no more branch at all
			}
			pr_info("a3\n");
			return ERR_PTR(-EAGAIN); // no more branch in this db
		}
	}else{ // else if(status == -ENOENT){}// ???
		// readahead branchs data
		pr_info("a4\n");
		branch = list_entry(tag_ctx->cursor_branchs, struct branch, child);
		pr_info("a5\n");
		load_branch(branch, tag_ctx);
		pr_info("a6\n");
	}

	pr_info("end slow_lookup_branch\n");
	return branch;
}

struct branch *get_current_branch_lock(struct tag_context *tag_ctx){
	struct database *db;
	struct branch *branch;
	int err;
	struct dentry *d_tmp;

	pr_info("start get_current_branch_lock\n");
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
	if(IS_ERR_OR_NULL(branch)){
		pr_info("gg0\n");
		branch = slow_lookup_branch(tag_ctx, (int)branch);
	}

	pr_info("gg1\n");

	if(IS_ERR_OR_NULL(branch)){
		pr_info("gg2\n");
		if((int)branch == -ENOENT || (int)branch == -EAGAIN){
			pr_info("gg5.1\n");
			branch = list_entry(tag_ctx->cursor_branchs->next, struct branch, child);
			pr_info("gg5.3\n");
			d_tmp = branch->dir;
			if(!d_tmp){
				pr_info("d_tmp5.3 in null\n");
				return branch;
			}else{
				pr_info("d_tmp5.3 name: %s\n", d_tmp->d_name.name);
				if(d_is_negative(d_tmp)){
					pr_info("d_is_negative5.3\n");
					return branch;
				}
			}
			// ----------------------
			
			pr_info("gg4.0.1\n");
			load_branch(branch, tag_ctx);
			pr_info("gg5\n");
			
		}
		// branch = slow_lookup_branch(tag_ctx, (int)branch);
	}

	if(!branch)
		pr_info("get_current_branch_lock1: branch is null\n");

	if(IS_ERR(branch))
		pr_info("IS_ERR_OR_NULL1: branch\n");

	// if(list_empty(tag_ctx->cursor_branchs))
	// 	pr_info("get_current_branch_lock: cursor_branchs is empty\n");

	// branch = list_entry(tag_ctx->cursor_branchs->next, struct branch, child);
	if(!branch)
		pr_info("get_current_branch_lock2: branch is null\n");

	if(list_empty(&branch->subdirs))
		pr_info("nn1\n");
	else
		pr_info("nn2\n");

	if(list_empty(branch->subdirs.next))
		pr_info("nn3\n");
	else
		pr_info("nn4\n");

	struct database *tmp_db = list_entry(tag_ctx->cursor_tag, struct database, t_child);
	pr_info("get_current_branch_lock: cursor_tag: name3: %s\n", tmp_db->sb->s_root->d_name.name);

	// branch = lookup_current_branch(tag_ctx);
	// if(IS_ERR(branch)) // ???
	// 	return branch; // this is ptr err
	
	// return NULL;
	return branch;
}

int tag_readdir(struct file *file, struct dir_context *ctx) {
	struct tag_context *tag_ctx;
	int err;

	struct branch *branch;

	struct file *tag_file;
	struct file *branch_file;
	struct vtag *tag;
	struct datafile *datafile;

	pr_info("start tag_readdir\n");

	tag_ctx = file->private_data;

	branch = get_current_branch_lock(tag_ctx);
	pr_info("tag_readdir1\n");
	err = scan_branch(ctx, branch, &tag_ctx->cursor_subdirs);
	if(IS_ERR(err)){
		if(err == -EAGAIN){
			if(tag_ctx->cursor_branchs != tag_ctx->cursor_branchs->next)
				put_branch(branch);
			tag_ctx->cursor_branchs = tag_ctx->cursor_branchs->next; // update to next branch
		}
	}

	return 0;
}


int init_tag_context(struct tag_context *tag_ctx, struct file *filp){
	struct dentry *dir;
	struct vtag *tag;

	pr_info("start init_tag_context\n");

	// kref_init(&tag_ctx->refcount); // set refcount to 1

	// INIT_LIST_HEAD(&tag_ctx->cursor_tag);
	// INIT_LIST_HEAD(&tag_ctx->cursor_branchs);
	INIT_LIST_HEAD(&tag_ctx->cursor_subdirs);
	

	dir = dget(filp->f_path.dentry);
	pr_info("h2.1: %s\n", dir->d_name.name);
	pr_info("h2.2: %s\n", filp->f_path.dentry->d_name.name);
	if(!dir){
		return -ENOMEM; // ???
	}
	

	long magic;
	// tag = (struct vtag *)dir->d_inode->i_private;
	tag =  (struct vtag *)dir->d_fsdata;
	if(tag){
		magic = tag->magic;
		pr_info("magic1: %ld\n", magic);
	}else{
		pr_info("no magic1\n");
	}
	
	// if(tag){ // <<<< <=
	// 	return -ENOMEM; // ???
	// }

	tag_ctx->vtag = tag;
	tag_ctx->cursor_tag = tag->dbs.next;
	tag_ctx->cursor_branchs = &tag->sub_branchs;

	struct database *tmp_db = list_entry(tag_ctx->cursor_tag, struct database, t_child);

	pr_info("init_tag_context: cursor_tag: name1: %s\n", tmp_db->sb->s_root->d_name.name);

	tag_ctx->file_tag = NULL;
	tag_ctx->file_branch = NULL;
	
	
	dput(dir);
	return 0;
}

