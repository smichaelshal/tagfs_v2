// inode->i_private = vtag // inode of vdir
// dentry->d_fsdata = dentry_list // ???
// file->private_data = tag_ctx // file of vdir

// todo: check all releases functions

// alloc
//      init
//           get
//           put
//      release
// free

// alloc
//      init_base
//           init
//              put
//              get
//           release
//      release_base
// free

/*
* During an ideal release (without problems - when there is no use and no change in resources):
*
* Destruction of all dentries by shrinking and using stale with d_release, d_revalidate.
* 
* After that, all the branches are empty and you can disconnect them without a problem.
* 
* and then detaching the db_tag
* 
* And finally destruction of vtag
*/

// d_revalidate
// d_release
// delete cursor

// relsease called when the object is empty. (natural way)
// drop try empty the object. (deliberate way)

#include <linux/fs_struct.h>
#include "publisher.h"
#include "list_utils.h"
#include "../utils/utils.h"
#include "../database/database.h"
#include "../vtagfs.h"

#include "../include/ramfs/ramfs.h"

#include <linux/magic.h>

#define NR_BRANCH_READ_AHEAD 16

void put_vbranch(struct vbranch *vbranch, struct tag_context *tag_ctx);



struct getdents_callback_tag {
	struct dir_context ctx;
	long sequence;
    long mode;
    struct db_tag *db_tag;
    struct vbranch* last_vbranch;
};

struct getdents_callback_branch {
	struct dir_context ctx;
	long sequence;
    long mode;
    struct vbranch *vbranch;
    struct vtag *vtag;
    struct super_block *sb;
    struct dentry *dir;
};

struct dentry *lookup_tag(struct inode *dir, struct dentry *dentry, unsigned int flags);
struct dentry *lookup_file(struct inode *dir, struct dentry *dentry, unsigned int flags);

int tag_readdir(struct file *file, struct dir_context *ctx);
int tag_dir_open(struct inode *inode, struct file *file);
int tag_dir_close(struct inode *inode, struct file *file);

const struct dentry_operations file_dentry_operations;
// const struct inode_operations tag_dir_inode_operations;

int release_tag_context(struct tag_context *tag_ctx);
void release_db_tag(struct db_tag *db_tag);
void release_vtag(struct vtag *vtag);

const struct dentry_operations tag_dentry_operations = { // <<<
    // .d_revalidate // check if there are new db_tags ???
};

static inline unsigned char dt_type(struct inode *inode) { // from libfs
	return (inode->i_mode >> 12) & 15;
}

struct datafile *alloc_datafile(void){ // database.c
    return kzalloc(sizeof(struct datafile), GFP_KERNEL);
}

struct dentry_list *alloc_dentry_list(void){
    return kzalloc(sizeof(struct dentry_list), GFP_KERNEL);
}

struct vbranch *alloc_vbranch(void){
    return kzalloc(sizeof(struct vbranch), GFP_KERNEL);
}
struct db_tag *alloc_db_tag(void){
    return kzalloc(sizeof(struct db_tag), GFP_KERNEL);
}

struct vtag *alloc_vtag(void){
    return kzalloc(sizeof(struct vtag), GFP_KERNEL);
}

struct tag_context *alloc_tag_context(void){
    return kzalloc(sizeof(struct tag_context), GFP_KERNEL);
}

void release_vbranch_kref(struct kref *kref){ // <<<
    pr_info("start release_vbranch_kref\n");
    return;
}

bool is_vbranch_stale(struct vbranch *vbranch){
	return !!atomic_read(&vbranch->is_stale);
}

void make_stale(struct vbranch *vbranch){
	atomic_set(&vbranch->is_stale, 1);
}

void clean_stale(struct vbranch *vbranch){
	atomic_set(&vbranch->is_stale, 0);
}

struct dentry_list *init_dentry_list(struct dentry *dentry, struct vtag *vtag, struct vbranch *vbranch){
    /*
    * alloc and fill dentry_list
    */

    struct dentry_list *dentry_list = alloc_dentry_list();
    if(!dentry_list)
        return NULL;

    INIT_LIST_HEAD(&dentry_list->child);
    dentry_list->flag = 0;
    dentry_list->vtag = vtag;
    dentry_list->dentry = dentry;
    if(vbranch)
        dentry_list->vbranch = vbranch;
    else
        dentry_list->vbranch = NULL;

    return dentry_list;
}

struct db_tag *init_db_tag(struct dentry *dir, struct vtag *vtag, char *name){
    /*
    * Get the dir (on disk) of tag
    * return db_tag of this dir if success, else return NULL
    */
   pr_info("\n");
    
    struct db_tag *db_tag = alloc_db_tag();
    if(!db_tag)
        return NULL;

    db_tag->name = dup_name(name);
    if(!db_tag->name){
        kfree(db_tag);
        return NULL;
    }

    db_tag->dir = dir;
    db_tag->sb = dir->d_sb; // <<< change to virtual tag disk

    INIT_LIST_HEAD(&db_tag->child);
    INIT_LIST_HEAD(&db_tag->vbranchs);
    mutex_init(&db_tag->vbranchs_lock);
    db_tag->vtag = vtag;
    db_tag->flag = 0;

    return db_tag;
    
}


struct tag_context *init_tag_context(struct vtag *vtag){
    /*
    * Create initialized tag_context
    * return tag_context if success, else return NULL
    */
   pr_info("\n");

    struct tag_context *tag_ctx;
    tag_ctx = alloc_tag_context();
    if(!tag_ctx)
        return NULL;

    tag_ctx->vtag = vtag;

    tag_ctx->file_tag = NULL;
    tag_ctx->file_branch = NULL;

    tag_ctx->db_tag_cursor = NULL;
    tag_ctx->vbranch_cursor = NULL;
    tag_ctx->dentry_cursor = NULL;

    tag_ctx->flag = 0;

	spin_lock_init(&tag_ctx->vbranch_cursor_lock); // ??? needed
	spin_lock_init(&tag_ctx->db_tag_cursor_lock);
	spin_lock_init(&tag_ctx->dentry_cursor_lock);

	mutex_init(&tag_ctx->file_tag_lock);
	mutex_init(&tag_ctx->file_branch_lock); // ??? needed

    tag_ctx->is_locked_vbranch = false;

    return tag_ctx;
}

bool is_vbranch_lock(struct vbranch *vbranch){
    return !!kref_read(&vbranch->kref);
}

int tag_dir_open(struct inode *inode, struct file *file){
    /*
    * Create and add tag_context to file
    * return 0 if success, else retrun non-zero
    */
   pr_info("\n");
   struct tag_context *tag_ctx;
    tag_ctx = init_tag_context(inode->i_private);
    if(!tag_ctx)
        return -ENOMEM;
    file->private_data = tag_ctx;
    return 0;
}

int tag_dir_close(struct inode *inode, struct file *file){
    /*
    * release tag context and disconnet tag_context from file
    * return 0
    */
    pr_info("\n");
    release_tag_context(file->private_data);
    file->private_data = NULL;
    return 0;
}


void init_vbranch(struct vbranch *vbranch, struct db_tag *db_tag, char *name){
    pr_info("\n");
    vbranch->name = name;
    vbranch->flag = INIT_MODE;
    vbranch->db_tag = db_tag;

    INIT_LIST_HEAD(&vbranch->dentries);
    INIT_LIST_HEAD(&vbranch->child);
    clean_stale(vbranch);
    mutex_init(&vbranch->vbranch_lock);
    kref_init(&vbranch->kref);

    /*
    * add_vbranch() must to be last, becuse after this line,
    * the vbranch exposed in public.
    */
    add_vbranch(&db_tag->vbranchs, vbranch);
}


bool is_branch_loaded(struct vbranch *vbranch){ // <<<
    /*
    * return true if the branch alredy loaded to cache, else return false
    */
   pr_info("\n");

    return !(vbranch->flag & INIT_MODE) && !is_vbranch_stale(vbranch);
}

int fill_datafile(struct datafile *datafile, struct dentry *dentry, struct super_block *sb){
    // move to database.c

    /*
    * @datafile: empty datafile
    * @dentry: dentry of db file
    * @sb: sb of real file
    *
    * fill the datafile from data in disk
    * return 0 if success, else return non-zero
    */

    int err;

    datafile->name = dup_name(dentry->d_name.name);
    if(!datafile->name)
        return -ENOMEM;
    
    err = db_read_datafile(dentry, datafile);
    if(err){
        kfree(datafile->name);
        return err;
    }

    datafile->sb = sb;
    return 0;
}


struct datafile *init_datafile(struct dentry *dentry, struct super_block *sb){
    /*
    * get dentry of db file, sb of real file
    *
    * alloc and fill datafile.
    * read the ino, ino_parent of real file from db.
    * return datafile if success, else return NULL
    */

    int err;
    struct datafile *datafile = alloc_datafile();
    pr_info("\n");

    if(!datafile)
        return NULL;

    err = fill_datafile(datafile, dentry, sb);
    if(err){
        kfree(datafile);
        return NULL;
    }
    return datafile;
}

struct dentry *lookup_rmap(struct dentry *dentry){
    // move database.c ?
    /*
    * get dentry of dir tag (on disk).
    *
    * return dentry of rmap (on disk) if success,
    * else return NULL.
    */
    pr_info("\n");
    return db_lookup_dentry_share(dentry, RMAP_DIR_NAME);
}

struct file *open_file_dentry(struct dentry *dentry, umode_t mode){
    /*
    * open file of dentry.
    * return file if success, else return NULL.
    */

    struct path path;
    struct file *filp = NULL;
    char *buff_path, *full_path;
    int err;
    pr_info("\n");


    buff_path = kzalloc(PATH_MAX, GFP_KERNEL);
    if(!buff_path)
        return NULL;
    
    full_path = dentry_path_raw(dentry, buff_path, PATH_MAX);
    if(IS_ERR(full_path))
        goto out;
    
    err = kern_path(full_path, 0, &path);
    if(IS_ERR(err))
        goto out;

    filp = dentry_open(&path, mode, current_cred());
    path_put(&path);

out:
    kfree(buff_path);
    return filp;
}

struct vtag *init_vtag(char *name){
    /*
    * alloc and fill vtag.
    * return vtag is success, else return NULL
    */
    pr_info("\n");
    struct vtag *vtag = alloc_vtag();
    if(!vtag)
        return NULL;

    vtag->name = dup_name(name);
    INIT_LIST_HEAD(&vtag->db_tags);
    kref_init(&vtag->kref);

    return vtag;
}


int release_tag_context(struct tag_context *tag_ctx){
    /*
    * release tag_context
    * return 0 if success else return non-zero
    */
   pr_info("\n");

    if(tag_ctx->file_tag)
        fput(tag_ctx->file_tag);
    
    if(tag_ctx->file_branch)
        fput(tag_ctx->file_branch);

    if(tag_ctx->db_tag_cursor)
        delete_db_tag(tag_ctx->db_tag_cursor);
    
    if(tag_ctx->vbranch_cursor){
        // put_vbranch(vbranch, tag_ctx); // ???
        delete_vbranch(tag_ctx->vbranch_cursor);
    }
    
    // if(tag_ctx->dentry_cursor)
    //     delete_dentry_list(tag_ctx->dentry_cursor);
    
    kfree(tag_ctx);

    return 0;
}

void free_datafile(struct datafile *datafile){
    /*
    * free datafile and resources.
    */
   pr_info("\n");
    kfree(datafile->name);
    kfree(datafile);
}

int dentry_file_revalidate(struct dentry *dentry, unsigned int flags){
    /*
    * check if vbranch of this dentry is stale
    */
   pr_info("\n");
    struct dentry_list *dentry_list = dentry->d_fsdata;
    if(dentry_list &&  dentry_list->vbranch)
        return !is_vbranch_stale(dentry_list->vbranch);
    return 1; // always valid, change ???
}

void free_dentry_list(struct dentry_list *dentry_list){
    /*
    * delete the dentry_list from dentries list, and free dentry_list.
    */
    pr_info("\n");
    delete_dentry_list(dentry_list);
    kfree(dentry_list);
}

void dentry_file_release(struct dentry *dentry){
    /*
    * d_release of regular files in vdir
    * make stale
    * if the dentry is the last in vbranch, release vbranch
    */

    struct dentry_list *dentry_list;
    struct vbranch *vbranch;
    pr_info("\n");
    dentry_list = dentry->d_fsdata;
    if(!dentry_list)
        return;
    
    vbranch = dentry_list->vbranch;

    if(vbranch && !is_vbranch_stale(vbranch))
        make_stale(vbranch);

    free_dentry_list(dentry_list);

    if(vbranch && is_vbranch_empty(vbranch)){
        vbranch->flag = INIT_MODE;
        clean_stale(vbranch);
    }
}

void free_vbranch(struct vbranch *vbranch){
    /*
    * delete the vbranch from vbranchs list, and free vbranch.
    */
   pr_info("\n");
    delete_vbranch(vbranch);
    kfree(vbranch->name);
    kfree(vbranch);
}

void release_vbranch(struct vbranch *vbranch){  // <<<
    /*
    * Releases the branch and
    * tries to release the db_tag if it can (when db_tag empty)
    */

    struct db_tag *db_tag = vbranch->db_tag;
    pr_info("\n");
    free_vbranch(vbranch);
    if(is_db_tag_empty(db_tag))
        release_db_tag(db_tag);
}

void free_db_tag(struct db_tag *db_tag){
    /*
    * delete the db_tag from db_tags list, and free db_tag.
    */

    delete_db_tag(db_tag);
    kfree(db_tag->name);
    kfree(db_tag);
}

void release_db_tag(struct db_tag *db_tag){
    /*
    * Releases the db_tag and
    * tries to release the vtag if it can (when vtag empty)
    */
    pr_info("\n");
    struct vtag *vtag = db_tag->vtag;
    free_db_tag(db_tag);
    if(is_vtag_empty(vtag))
        release_vtag(vtag);
}

void free_vtag(struct vtag *vtag){
    kfree(vtag->name);
    kfree(vtag);
}

void release_vtag(struct vtag *vtag){ // <<<
    /*
    * Releases the vtag and all the resources of vtag (vdir)
    */
    pr_info("\n");
    if(vtag->vdir)
        dput(vtag->vdir); // ??? try delete vdir
    free_vtag(vtag);
}

void make_dead_vbranch(struct vbranch *vbranch){
    struct dentry_list *cursor, *current_dentry_list;

    if(is_vbranch_lock(vbranch)){
        release_vbranch(vbranch);
        return;
    }

    cursor = add_dentry_cursor(&vbranch->dentries);
    if(!cursor)
        return; // error ??? must success !!! (the vbranch remains alive)

    while(1){
        current_dentry_list = get_current_update_dentry(cursor, &vbranch->dentries, NULL); // ??? lock
        if(!current_dentry_list)
            break;
        current_dentry_list->flag |= DEAD_MODE;
    }
    delete_dentry_list(cursor);
    
}

void umount_db_tag(struct db_tag *db_tag){
    pr_info("\n");
    struct vbranch *cursor, *current_vbranch;

    db_tag->flag |= DEAD_MODE;

    shrink_dcache_sb(db_tag->sb);

    cursor = add_vbranch_cursor(&db_tag->vbranchs);
    if(!cursor)
        return;

    while(1){
        current_vbranch = get_current_update_vbranch(cursor, &db_tag->vbranchs, NULL); // ??? lock
        if(!current_vbranch)
            break;
        make_dead_vbranch(current_vbranch);
    }
    delete_vbranch(cursor);
}

void unload_vtagfs(void){ // <<<
    /*
    * when the module unload (uninstall)
    * must kill all structs and objects. !!!
    */
}




struct dentry *get_root_vdir(void){  // <<<
    /*
    * return the parent of vdir, if success return dentry, else return NULL.
    * todo: public variable to root dir.
    */

    struct path path;
    struct dentry *dentry;
    int err;
    pr_info("\n");

    err = kern_path(root_tag_path, 0, &path);
    if(err)
        return NULL;
    
    dentry = dget(path.dentry);
    path_put(&path);
    return dentry;
}

struct dentry *create_vdir(char *name){
    /*
    * alloc and fill vdir (dentry of vtag)
    * return dentry of vdir if success, else return NULL.
    */
    struct dentry *parent, *dentry;
    struct inode *inode_parent, *inode;
    int err;

    parent = get_root_vdir();
    if(!parent)
        return NULL;

    dentry = d_alloc_name(parent, name);
    if(!dentry)
        goto out;
    
    // d_set_d_op(dentry, &tag_dentry_operations); // todo: active this line
	d_add(dentry, NULL); // ???

    inode_parent = parent->d_inode;
    err = inode_parent->i_op->mkdir(&init_user_ns, inode_parent, dentry, DEFAULT_MODE_FILES);
    if(err){
        dput(dentry);
        dentry = NULL;
        goto out;
    }
    dget(dentry); // pin vdir // ???

    inode = dentry->d_inode;
    inode->i_fop = &tag_dir_operations; // <<<
    inode->i_op = &tag_dir_inode_operations;
out:
    dput(parent);
    return dentry;
}

struct dentry *lookup_tag_by_mount(struct vfsmount *mnt, char *name){
    /*
    * Lookup in this mount the dir of tag
    * return dir (dentry) if found, else return NULL
    */
    pr_info("\n");
    if(!mnt->mnt_root)
        return NULL;

    return db_lookup_dentry_share(mnt->mnt_root, name); // <<<
}

bool is_taged_mount(struct vfsmount *mnt){ // <<<
    /*
    * return true if the mount contain taged files, else return false
    * now this is temporery way.
    * todo: check with xattr
    */
   pr_info("\n");
    if(mnt && mnt->mnt_sb){
        switch (mnt->mnt_sb->s_magic) {
            case EXT4_SUPER_MAGIC:
                return true;
            // case RAMFS_MAGIC:
            //     return true;
            // case TMPFS_MAGIC:
            //     return true;
            // case PROC_SUPER_MAGIC:
            //     return true;
        }
    }
    return false;
}

int scan_mounts(struct vfsmount *mnt, void *arg){
    /*
    * Get vfsmount and vtag.
    * add db_tag of vfsmount if relevant to vtag.
    * return 0 ???
    */ 

    struct vtag *vtag = (struct vtag *)arg;
    struct dentry *dir;
    struct db_tag *db_tag;
    pr_info("\n");

    if(is_taged_mount(mnt))
        dir = lookup_tag_by_mount(mnt, vtag->name);
        if(!dir || d_is_negative(dir) || !d_is_dir(dir))
            return 0;
        
        if(dir && simple_positive(dir) && dir->d_inode)
            db_tag = init_db_tag(dir, vtag, vtag->name);
            if(db_tag)
                add_db_tag(&vtag->db_tags, db_tag);
    return 0;
}

int lookup_db_tags(struct vtag *vtag){
    /*
    * Get name of tag and head list of db_tags from vtag
    * Fill list of db_tag in vtag with the relevant db_tags
    * 
    * return 0 if success else return non-zero
    *
    * Todo: Check what is fail in this functuon ??? (currently always returns zero) 
    */
    pr_info("\n");
    iterate_mounts(scan_mounts, vtag, current->fs->root.mnt);        
    return 0;
}



struct vtag *create_vtag(char *name){
    /*
    * Create vtag and fill relevant data
    * return vtag, if error return NULL
    */
    pr_info("\n");
    struct vtag *vtag = init_vtag(name);
    lookup_db_tags(vtag);
    return vtag;
}

struct dentry *create_symlink_hook(struct dentry *dentry){
    /*
    * Get dentry of vdir tag
    *
    * return dentry if symlink if success, else reutrn NULL.
    */

   struct dentry *d_sym;
   char *symlink_data;
   int err;
   pr_info("\n");

    d_sym = d_alloc_name(dentry, SYMLINK_FILENAME);
    if(!d_sym)
        return NULL;

    d_add(d_sym, NULL);

    symlink_data = join_path_str(root_tag_path, dentry->d_name.name);
    if(!symlink_data)
        goto out;

    err = dentry->d_inode->i_op->symlink(&init_user_ns, dentry->d_inode, d_sym, symlink_data);
    kfree(symlink_data);
    if(IS_ERR(err))
        goto out;
    
    return d_sym;
out:
    dput(d_sym);
    return NULL;
}

struct dentry *lookup_tag(struct inode *dir, struct dentry *dentry, unsigned int flags){
    /*
    * Create vtag and vdir.
    * return vdir (dentry) if success else return NULL
    *
    * The function always returns vdir
    * if there is a problem not of the module such as a resource problem
    * (for example there is not enough memory)
    *
    * If the function is called then the tag is probably not in the cache
    */

    struct vtag *vtag;
    struct dentry *vdir, *d_parent, *d_sym;
    pr_info("%s\n", dentry->d_name.name);

    vtag = create_vtag(dentry->d_name.name);
    if(!vtag)
        return NULL;
        
    
    vdir = create_vdir(dentry->d_name.name); // <<<
    if(!vdir){
         d_parent = dget_parent(dentry);
        if(d_parent){
    
            vdir = db_lookup_dentry_share(d_parent, dentry->d_name.name);
            dput(d_parent);
        }
        goto out;
    }

    vdir->d_inode->i_private = vtag;
    vtag->vdir = vdir;

    d_sym = create_symlink_hook(vdir);
    if(!d_sym){
        dput(vdir);
        goto out;
    }

    return vdir;
out:
    release_vtag(vtag);
    return NULL;
}








struct dentry *lookup_dentry_in_db(struct dentry *root, char *name){
    // move to database.c

    /*
    * Lookup dentry in db_tag root by name
    * Search in dmap datafile by name
    *
    * if found return dentry, else return NULL
    */

    struct dentry *dmap, *dentry;
    pr_info("\n");

    
    dmap = db_lookup_dentry_share(root, DMAP_DIR_NAME);
    if(!dmap)
        return NULL;
    dentry = db_lookup_dentry_share(dmap, name);
    dput(dmap);
    return dentry;
}

struct datafile *lookup_datafile_in_db(struct db_tag *db_tag, char *name){
    // move to database.c

    /*
    * Lookup datafile in db_tag by name
    * Search in dmap datafile by name
    *
    * if found return datafile, else return NULL
    */
    struct dentry *dentry;
    pr_info("\n");

    dentry = lookup_dentry_in_db(db_tag->dir, name);
    
    if(!dentry)
        return NULL;

    return init_datafile(dentry, db_tag->sb);
}

struct dentry *load_datafile(struct vtag *vtag, struct datafile *datafile){
    /*
    * Get datafile and vtag.
    *
    * From this datafile create dentry and load to dcache
    * return dentry of file if successs, else return NULL
    */

    struct inode *inode;
    struct dentry *dentry;
    pr_info("\n");

    inode = iget_generic(datafile->sb, datafile->ino);
    if(!inode)
        return NULL;


    dentry = d_alloc_name(vtag->vdir, datafile->name);
    if(!dentry)
        goto out;
    
    dentry->d_sb = datafile->sb; // ??? shrink list change

    d_set_d_op(dentry, &file_dentry_operations);
	d_add(dentry, inode); // ???

out:
    iput(inode);
    return dentry;
}


struct dentry *lookup_file(struct inode *dir, struct dentry *dentry, unsigned int flags){
    /*
    * Lookup file in tag
    *
    * the lookup search in dmap, and load the dentry to dcache
    * if found return dentry of file, else return NULL
    */

    struct db_tag *cursor, *current_db_tag;
    struct datafile *datafile;
    struct dentry_list *dentry_list;

    struct dentry *d_file = NULL;
    struct vtag *vtag = dir->i_private;
    pr_info("%s\n", dentry->d_name.name);


    
    if(!vtag) // needed ???
        return NULL;
    
    cursor = add_db_tag_cursor(&vtag->db_tags);
    if(!cursor)
        return NULL;

    while(1){
        current_db_tag = get_current_update_db_tag(cursor, &vtag->db_tags, NULL);
        if(!current_db_tag)
            break;
       
        

        datafile = lookup_datafile_in_db(current_db_tag, dentry->d_name.name);
        if(!datafile)
            continue;
        

        d_file = load_datafile(vtag, datafile);

        
        if(!d_file)
            break; // todo: check what need in this error ???

        
        dentry_list = init_dentry_list(d_file, vtag, NULL);
        if(!dentry_list){
            dput(d_file);
            d_file = NULL;
            break;
        }
        d_file->d_fsdata = dentry_list;
    }
    delete_db_tag(cursor);
    return d_file;
}





int unlock_vbranch(struct vbranch *vbranch, struct dentry_list *last){
    /*
    * Unlocks a vbranch after the position of the past_cursor until the beginning.
    *
    * get vbranch and last of the place that could not be locked from
    * if the past_cursor is NULL try unlock all dentries in vbranch
    * return 0 if success, else return non-zero
    
    (use in error lock_vbranch and drop_vbranch)
        todo mabay: new reverse for function ???
    */

    struct dentry *dentry;
    struct dentry_list *cursor, *current_dentry_list;
    pr_info("\n");

    
    cursor = add_dentry_cursor(&vbranch->dentries);
    if(!cursor)
        return -ENOMEM;

    while(1){
        current_dentry_list = get_current_update_dentry(cursor, &vbranch->dentries, NULL); // ??? lock
        if(!current_dentry_list || current_dentry_list == last)
            break;
        dput(current_dentry_list->dentry); // ???
    }
    delete_dentry_list(cursor);
    // vbranch->flag &= ~VBRANCH_LOCK;
    return 0;
}


bool lock_dentries_vbranch(struct vbranch *vbranch, spinlock_t *lock){
    /*
    * lock all dentries in vbranch
    * return true if success, else return false
    */
    bool ret = true;
    struct dentry *dentry;
    struct dentry_list *cursor, *current_dentry_list;
    pr_info("\n");
    cursor = add_dentry_cursor(&vbranch->dentries);

    if(!cursor)
        return false;

    while(1){
        current_dentry_list = get_current_update_dentry(cursor, &vbranch->dentries, lock); // ??? lock
        if(!current_dentry_list)
            break;
        dentry = dget(current_dentry_list->dentry);
        if(!dentry){
            unlock_vbranch(vbranch, current_dentry_list); 
            ret = false;
            goto out;
        }
    }
out:
    delete_dentry_list(cursor);
    return ret;
}

bool lock_vbranch(struct vbranch *vbranch, spinlock_t *lock, bool *is_locked_vbranch){ 
    /*
    * try lock vbranch, if success return true, else return false
    *
    * Solve the problem of repeated locking by the same caller (same context) !!! v
    */
   pr_info("\n");
    if(*is_locked_vbranch)
        return true;
    
    kref_get(&vbranch->kref);
    
    if(is_vbranch_stale(vbranch))
        goto out_err;
    
    if(kref_read(&vbranch->kref) == 1){
        if(!lock_dentries_vbranch(vbranch, lock))
            goto out_err;
    }

    if(is_vbranch_stale(vbranch))
        goto out_err;

    *is_locked_vbranch = true;

    return true;

out_err:
    kref_put(&vbranch->kref, release_vbranch_kref);
    return false;
}


void test_vbranchs(struct list_head *list){
    pr_info("\n");
    
    struct vbranch *current1, *cursor1;
    cursor1 = add_vbranch_cursor(list);
    if(!cursor1)
        return;

    while(1){
        current1 = get_current_update_vbranch(cursor1, list, NULL); // ??? lock
        if(!current1)
            break;
        pr_info("current1: name: %s, addr: %x, flag: %ld\n", current1->name, current1, current1->flag);
    }
    delete_vbranch(cursor1);
}

struct vbranch *fast_lookup_vbranch(struct tag_context *tag_ctx,struct db_tag *db_tag){
    /*
    * return vbranch if success, else return NULL
    *
    * The search is only done in the cache (the lists from db_tag).
    */

    struct vbranch *current_vbranch;
    struct vbranch *cursor, *vbranch_cursor;

    pr_info("\n");

    if(!tag_ctx->vbranch_cursor)
        tag_ctx->vbranch_cursor = add_vbranch_cursor(&db_tag->vbranchs);
        
    cursor = tag_ctx->vbranch_cursor;
    current_vbranch = cursor;

    test_vbranchs(&tag_ctx->vtag->db_tags);

    return get_current_vbranch(cursor, &db_tag->vbranchs);
}


struct file *open_file_tag(struct tag_context *tag_ctx, struct db_tag *db_tag){  // <<<
    /*
    * open rmap dir from db_tag
    * return rmap file if success, else return NULL
    */

    struct dentry *rmap;
    struct file *filp = NULL;
    pr_info("\n");

    // if(!tag_ctx->file_tag)
    mutex_lock(&tag_ctx->file_tag_lock);

    if(tag_ctx->file_tag){
        filp = tag_ctx->file_tag;
        goto out;
    }
    rmap = lookup_rmap(db_tag->dir);
    if(!rmap)
        goto out;
    filp = open_file_dentry(rmap, O_RDONLY | O_DIRECTORY);
out:
    mutex_unlock(&tag_ctx->file_tag_lock);
    return filp;
}

struct vbranch *load_base_vbranch(struct db_tag *db_tag, char *name){
    /*
    * load the base of vbranch to list in db_tag.
    * return vbranch if success, else return NULL
    */
    struct vbranch *vbranch;
    pr_info("\n");

    vbranch = alloc_vbranch();
    if(!vbranch)
        return NULL;
    init_vbranch(vbranch, db_tag, name);
    return vbranch;
}

int iter_tag(struct dir_context *ctx, const char *name, int len,
                loff_t pos, u64 ino, unsigned int d_type){  // <<<
    
    int err = 0;
    struct db_tag *db_tag;
    struct vbranch *last;
    struct getdents_callback_tag *getdents_tag = container_of(ctx, struct getdents_callback_tag, ctx);

    pr_info("\n");
    
    db_tag = getdents_tag->db_tag;
    getdents_tag->sequence++;
    last = getdents_tag->last_vbranch;

    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
		return 0;

    if(!last)
        getdents_tag->mode &= ~SKIP_MODE;

    if(!(getdents_tag->mode & SKIP_MODE))
        load_base_vbranch(getdents_tag->db_tag, name); // <<<
    else if(last && last->name == name)
        getdents_tag->mode &= ~SKIP_MODE;
    
    return err;
}

int readahead_vbranchs(struct tag_context *tag_ctx, struct db_tag *db_tag, long max_count){
    /*
    * @db_tag: source to read
    * @and count - how many try read
    *
    * load base vbranchs from db_tag
    *
    * Returns the number of vbranch read
    */

    int err;
    struct vbranch *vbranch, *last_vbranch;
    struct file *file_tag;
    long count = 0;

    pr_info("\n");

    mutex_lock(&db_tag->vbranchs_lock);
    vbranch = fast_lookup_vbranch(tag_ctx, db_tag);
    if(vbranch)
        goto out;
    
    if(!tag_ctx->file_tag)
        tag_ctx->file_tag = open_file_tag(tag_ctx, db_tag); // @@@
    file_tag = tag_ctx->file_tag;

    if(!file_tag)
        goto out;

    last_vbranch = get_current_vbranch_prev(tag_ctx->vbranch_cursor, &db_tag->vbranchs);
    

    struct getdents_callback_tag getdents_tag = {
		.ctx.actor = iter_tag,
        .db_tag = db_tag,
        .sequence = 0,
        .mode = SKIP_MODE,
        .last_vbranch = last_vbranch,
	};

    while(max_count--){
        long old_seq = getdents_tag.sequence;
		err = iterate_dir(file_tag, &getdents_tag.ctx); // @@@

        if(IS_ERR(err)) // ???
            break;
        
        if(old_seq == getdents_tag.sequence)
			break;
        
        count++;
    }
out:
    mutex_unlock(&db_tag->vbranchs_lock);
    return count;
}

int drop_vbranch(struct vbranch *vbranch){ // <<<
    /*
    * emptied the vbranch, becomes the vbranch to base (dput to all dentries)
    * unlock vbranch
    *
    * return 0 if success, else return non-zero.
    *
    * todo: try replace the current while to shrink. !!!
    * (now maybe error when drop and load again,
    * becuse the vbranch is not emptry, (d_drop dont kill dentry))
    */

    struct dentry *dentry;
    struct dentry_list *cursor, *current_dentry_list;
    pr_info("\n");

    if(is_vbranch_lock(vbranch))
        unlock_vbranch(vbranch, NULL);

    // shrink_vbranch(vbranch); // <<< ???
    
    cursor = add_dentry_cursor(&vbranch->dentries);
    if(!cursor)
        return -ENOMEM;

    while(1){
        current_dentry_list = get_current_update_dentry(cursor, &vbranch->dentries, NULL); // ??? lock
        if(!current_dentry_list)
            break;
        d_drop(current_dentry_list->dentry);
    }

    delete_dentry_list(cursor);

    // vbranch->flag |= INIT_MODE; // add when the vbranch became to base_vbranch (when empty).
    // clean_stale(vbranch); // add when the vbranch became to base_vbranch (when empty).
    return 0;
}

struct db_tag *update_next_db_tag(struct tag_context *tag_ctx){
    /*
    * Moves the cursor to the next db_tag
    *
    * Return the current db_tag, if success else return NULL
    */
   pr_info("\n");
    if(tag_ctx->file_tag){
        fput(tag_ctx->file_tag);
        tag_ctx->file_tag = NULL;
    }
    get_current_update_db_tag(tag_ctx->db_tag_cursor, &tag_ctx->vtag->db_tags, &tag_ctx->db_tag_cursor_lock);
    return get_current_db_tag(tag_ctx->db_tag_cursor, &tag_ctx->vtag->db_tags);
}

struct vbranch *slow_lookup_vbranch(struct tag_context *tag_ctx, struct db_tag *db_tag, struct vbranch *vbranch){ // <<<
    /*
    * return vbranch if success, else return NULL.
    *
    * The lookup tries hard to get the vbranch
    * and does all the things necessary to get it
    *
    * The function return base vbranch, ???
    * that is, it contains basic information and needs to be loaded ???
    */
   pr_info("\n");
retry_readahead:
    if(!vbranch){
        if(!readahead_vbranchs(tag_ctx, db_tag, NR_BRANCH_READ_AHEAD)){
            db_tag = update_next_db_tag(tag_ctx);
            if(!db_tag)
                return NULL;
            move_vbranch_cursor(tag_ctx->vbranch_cursor, &db_tag->vbranchs);
        }
        vbranch = get_current_vbranch(tag_ctx->vbranch_cursor, &db_tag->vbranchs);
        goto retry_readahead;
    }

    if(is_vbranch_stale(vbranch))
        drop_vbranch(vbranch);
    return vbranch;
}

int iter_branch(struct dir_context *ctx, const char *name, int len,
                loff_t pos, u64 ino, unsigned int d_type){  // <<<
    
    struct super_block *sb;
    struct vbranch *vbranch;
    struct vtag *vtag;
    struct dentry *dir, *db_file_dentry, *dentry;
    struct datafile *datafile;
    struct dentry_list *dentry_list;
    int err = 0;
    pr_info("\n");

    struct getdents_callback_branch *getdents_branch = container_of(ctx, struct getdents_callback_branch, ctx);
    
    getdents_branch->sequence++;

    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
        return 0;

    ctx->pos++; // ???

    sb = getdents_branch->sb;
    vbranch = getdents_branch->vbranch;
    vtag = getdents_branch->vtag;
    dir = getdents_branch->dir;

    if(getdents_branch->mode & SKIP_MODE)
        return 0;
    
    db_file_dentry = db_lookup_dentry_share(dir, name);
    if(!db_file_dentry)
        return -ENOENT;
    
    datafile = init_datafile(db_file_dentry, sb);
    dput(db_file_dentry);
    if(!datafile)
        return -ENOMEM;
    dentry = load_datafile(vtag, datafile);
    if(!dentry)
        goto out;

    dentry_list = init_dentry_list(dentry, vtag, vbranch);
    if(dentry_list){
        dentry->d_fsdata = dentry_list;
        add_dentry_list(&vbranch->dentries, dentry_list);
    }else{
        err = -ENOMEM;
        dput(dentry);
    }
out:
    free_datafile(datafile);
    return err;
}

struct file *open_file_branch(struct tag_context *tag_ctx, struct vbranch *vbranch){
    /*
    * get tag_ctx, vbranch.
    * try open file of vbranch (in disk)
    *
    * lookup in rmap the dir of branch.
    * return file if success, else return NULL.
    */

    struct dentry *rmap, *dentry;
    struct file *filp = NULL;

    pr_info("\n");


    // if(!tag_ctx->file_branch)
    mutex_lock(&tag_ctx->file_branch_lock);

    if(tag_ctx->file_branch){
        filp = tag_ctx->file_branch;
        goto out;
    }
    
    rmap = lookup_rmap(vbranch->db_tag->dir);
    if(!rmap)
        goto out;
    
    dentry = db_lookup_dentry_share(rmap, vbranch->name);
    dput(rmap);
    if(!dentry)
        goto out;
    
    filp = open_file_dentry(dentry, O_RDONLY | O_DIRECTORY);
    dput(dentry);

out:
    mutex_unlock(&tag_ctx->file_branch_lock);
    return filp;
}

struct vbranch *load_vbranch(struct tag_context *tag_ctx, struct vbranch *vbranch, struct super_block *sb){ // <<<
    /*
    * get base vbranch and tag_ctx
    * try to load this vbranch
    *
    * return vbranch if success, else return NULL
    */
    
    struct file *file_branch;
    struct mutex *vbranch_lock;
    int err;
    pr_info("\n");

    vbranch_lock = &vbranch->vbranch_lock;
    mutex_lock(vbranch_lock);

    if(!tag_ctx->file_branch)
        tag_ctx->file_branch = open_file_branch(tag_ctx, vbranch);
    file_branch = tag_ctx->file_branch;

    if(!file_branch){
        vbranch = NULL;
        goto out;
    }

    struct getdents_callback_branch getdents_branch = {
		.ctx.actor = iter_branch,
        .sb = sb,
        .vbranch = vbranch,
        .vtag = tag_ctx->vtag,
        .dir = file_branch->f_path.dentry,
        .sequence = 0,
        .mode = 0,
	};

    while(1){
        long old_seq = getdents_branch.sequence;
		err = iterate_dir(file_branch, &getdents_branch.ctx); // @@@

        if(IS_ERR(err)) // ???
            break;
        
        if(old_seq == getdents_branch.sequence){
            tag_ctx->file_branch = NULL;
            fput(file_branch);
			break;
        }
    }

out:
    mutex_unlock(vbranch_lock);
    if(vbranch){
        kref_get(&vbranch->kref);
        tag_ctx->is_locked_vbranch = true;
        clean_stale(vbranch);
        vbranch->flag &= ~INIT_MODE;
    }
    return vbranch;
}

struct vbranch *get_vbranch_lock(struct tag_context *tag_ctx, struct db_tag *db_tag){
    /*
    * return the current vbranch.
    * if no more dentries return NULL.
    *
    * The vbranch searching start in fast path (in caches)
    * if not fount goes to slow path
    */
    
    struct vbranch *vbranch;
    int err;
    pr_info("\n");

    vbranch = fast_lookup_vbranch(tag_ctx, db_tag);
   
    if(vbranch && is_branch_loaded(vbranch)){
        if(lock_vbranch(vbranch, &tag_ctx->dentry_cursor_lock, &tag_ctx->is_locked_vbranch))
            return vbranch;
    }
    vbranch = slow_lookup_vbranch(tag_ctx, db_tag, vbranch);
    if(!vbranch)
        return NULL;
    
    if(is_branch_loaded(vbranch))
        if(lock_vbranch(vbranch, &tag_ctx->dentry_cursor_lock, &tag_ctx->is_locked_vbranch))
            return vbranch;
    return load_vbranch(tag_ctx, vbranch, db_tag->sb);

}


struct db_tag *get_db_tag(struct tag_context *tag_ctx){
    /*
    * return the current db_tag.
    * if no more db_tags return NULL.
    *
    * In the end of db_tag must update before call more times !!!
    */

    struct db_tag *cursor;
    pr_info("\n");

    spin_lock(&tag_ctx->vbranch_cursor_lock);
    if(!tag_ctx->db_tag_cursor)
        tag_ctx->db_tag_cursor = add_db_tag_cursor(&tag_ctx->vtag->db_tags);
    spin_unlock(&tag_ctx->vbranch_cursor_lock);

    return get_current_db_tag(tag_ctx->db_tag_cursor, &tag_ctx->vtag->db_tags);
}


void put_vbranch(struct vbranch *vbranch, struct tag_context *tag_ctx){
    /*
    * put the vbranch in the end of used.

    // kref_put
    // unlock branch
    // is_locked_vbranch = false,
    // delete cursor dentry
    // update cursor vbranchs
    */

    pr_info("\n");

    kref_put(&vbranch->kref, release_vbranch_kref);
    if(!kref_read(&vbranch->kref))
        unlock_vbranch(vbranch, NULL);

    tag_ctx->is_locked_vbranch = false;
}


bool emit_dentry(struct dir_context *ctx, struct dentry *dentry){
    /*
    * Emit files.
    * return true if the emit success, else return false
    */

    struct inode *inode;
    const char *name;
	unsigned type;
	int len;
	u64 ino;
    pr_info("\n");
    
    inode = d_inode(dentry);
    if(!inode) // needed?
        return false;
    
    name = dentry->d_name.name;
    len = dentry->d_name.len;
    type = dt_type(inode);
    ino = inode->i_ino;

    pr_info("emit: %s\n", name);

    return dir_emit(ctx, name, len, ino, type);
}


int scan_vbranch(struct dir_context *ctx, struct vbranch *vbranch, struct tag_context *tag_ctx, struct db_tag *db_tag){
    /*
    * Scan the branch and emit files.
    * Returns the number of files it emit
    *
    * If cursor is end, free vbranch
    */
    
    struct dentry_list *cursor, *current_dentry;
    long count = 0;
    pr_info("\n");

    if(!tag_ctx->dentry_cursor)
        tag_ctx->dentry_cursor = add_dentry_cursor(&vbranch->dentries);
        
    cursor = tag_ctx->dentry_cursor;
    current_dentry = cursor;

    while(1){
        // current_dentry = get_current_update_dentry(cursor, &vbranch->dentries, &tag_ctx->dentry_cursor_lock);
        current_dentry = get_current_dentry_list(cursor, &vbranch->dentries);
        if(!current_dentry)
            break;


        if(!emit_dentry(ctx, current_dentry->dentry))
            break;


        update_dentry_list_cursor(cursor, current_dentry); // check if must lock (now not lock)
        count++;

    }

    // if(is_vbranch_in_end(cursor)) // <<<
    if(!current_dentry){
        delete_dentry_list(tag_ctx->dentry_cursor);
        tag_ctx->dentry_cursor = NULL; // ???
        get_current_update_vbranch(tag_ctx->vbranch_cursor, &db_tag->vbranchs, NULL); //  &db_tag->vbranchs_lock ??? lock?
        put_vbranch(vbranch, tag_ctx);
    }
    return count;
}


int tag_readdir(struct file *file, struct dir_context *ctx){
    /*
    * Emit files from current branch.
    * 
    * That means the branch must be cached.
    * return 0
    */

    struct db_tag *db_tag;
    struct vbranch *vbranch;
    bool loop = true;
    long count = 0;
    struct tag_context *tag_ctx = (struct tag_context *)file->private_data;
    pr_info("\n");

retry:
    db_tag = get_db_tag(tag_ctx); // ^^^
    if(!db_tag)
        return 0;
    vbranch = get_vbranch_lock(tag_ctx, db_tag); // ^^^
    if(!vbranch)
        return 0;

    count = scan_vbranch(ctx, vbranch, tag_ctx, db_tag);
    if(!count && loop){
        loop = false;
        goto retry;
    }
    return 0;
}


// const struct inode_operations tag_dir_inode_operations = {
// 	.lookup		= lookup_file, // <<
// 	// .symlink	= ramfs_symlink,
    
// 	// .create		= ramfs_create,
// 	// .link		= simple_link,
// 	// .unlink		= simple_unlink,
// 	// .mkdir		= ramfs_mkdir,
// 	// .rmdir		= simple_rmdir,
// 	// .mknod		= ramfs_mknod,
// 	// .rename		= simple_rename,
// 	// .tmpfile	= ramfs_tmpfile,
// };

const struct dentry_operations file_dentry_operations = {
    .d_release = dentry_file_release,
    .d_revalidate = dentry_file_revalidate,
};