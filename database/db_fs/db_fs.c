// db_fs.c
#include <linux/xattr.h>

#include "db_fs.h"
#include "../database.h"
#include "../../utils/utils.h"

#define COUNT_SUBFILES_NAME "security.subfiles"

#include "../../vtagfs.h" // to pr_info

struct getdents_callback_rmap {
	struct dir_context ctx;
	long sequence;
    struct dentry* d_rmap;
    struct dentry* d_branch;
    struct datafile *datafile;
};

int db_create(struct dentry *dentry, struct dentry *parent){
    // struct dentry *parent;
    int err = 0;
    umode_t mode = DEFAULT_MODE_FILES;
    pr_info("\n");
    // parent = dget_parent(dentry);
    if(!parent)
        return -ENOENT;
    
    if(!d_is_negative(parent))
        err = vfs_create(&init_user_ns, d_inode(parent), dentry, mode, true);
    
    // dput(parent);
    return err;
}

int db_rmdir(struct dentry *dentry){
    pr_info("\n");
    return vfs_rmdir(&init_user_ns, d_inode(dentry->d_parent), dentry);
}

int db_add_xattr(struct dentry *dentry, char *name, void *value){
    pr_info("\n");
    return vfs_setxattr(&init_user_ns, dentry, name, value, strlen(value), XATTR_CREATE);
}

int db_set_xattr(struct dentry *dentry, char *name, void *value){
    pr_info("\n");
    return vfs_setxattr(&init_user_ns, dentry, name, value, strlen(value), XATTR_REPLACE);
}

int db_link(struct dentry *old_dentry, struct dentry *new_dentry){
    int err;
    pr_info("\n");
    struct dentry *dir = dget_parent(new_dentry);
    err = vfs_link(old_dentry, &init_user_ns, d_inode(dir), new_dentry, NULL);
    dput(dir);
    return err;
}

int db_unlink(struct dentry *dentry){
    int err;
    struct dentry *dir = dget_parent(dentry);
    err = vfs_unlink(&init_user_ns, d_inode(dir), dentry, NULL);
    dput(dir);
    return err;
}


char *db_get_xattr(struct dentry *dentry, char *name){
    int err;
    char *buff = kzalloc(XATTR_SIZE_MAX, GFP_KERNEL);
    pr_info("\n");
    if(!buff)
        return ERR_PTR(-ENOMEM);
    err = vfs_getxattr(&init_user_ns, dentry, name, buff, XATTR_SIZE_MAX);
    if(err < 0){
        kfree(buff);
        return ERR_PTR(err);
    }
    return buff;
}

int create_next_branch(struct dentry *dir, long nr){
    pr_info("\n");
    int err = 0;
    char *buff = long_to_string(nr);
    if(IS_ERR(buff))
        return PTR_ERR(buff);
    
    err = db_add_xattr(dir, NEXT_NAME, buff);
    kfree(buff);
    return err;
}

int set_next_branch(struct dentry *dir, long nr){
    pr_info("\n");
    int err = 0;
    char *buff = long_to_string(nr);
    if(IS_ERR(buff))
        return PTR_ERR(buff);
    
    err = db_set_xattr(dir, NEXT_NAME, buff);
    kfree(buff);
    return err;
}

struct dentry *db_lookup_dentry_share(struct dentry *parent, char *name){ // :::
    struct dentry *d_target;
    struct inode *inode;
    pr_info("\n");

    int err;
    if(!parent){
        return NULL;
    }

    inode = d_inode(parent);
    inode_lock_shared(inode);
    // err = down_write_killable_nested(&inode->i_rwsem, I_MUTEX_PARENT);
    // inode_lock_nested(inode, I_MUTEX_PARENT); //
    if(err)
        return NULL;

    d_target = lookup_one(&init_user_ns, name, parent, strlen(name));
    // inode_unlock(inode);

    inode_unlock_shared(inode);

    if(IS_ERR(d_target) || (d_target && !d_target->d_inode)){ // d_is_negative
        dput(d_target);
        return NULL;
    }
    return d_target;
}


struct dentry *db_lookup_dentry(struct dentry *parent, char *name){ // :::
    struct dentry *d_target;
    struct inode *inode;
    pr_info("\n");
    
    if(!parent){
        return NULL;
    }
    
    inode = d_inode(parent);
    inode_lock(inode);
    // down_write_killable(&inode->i_rwsem);
    d_target = lookup_one(&init_user_ns, name, parent, strlen(name));
    inode_unlock(inode);

    if(IS_ERR(d_target) || (d_target && !d_target->d_inode)){
        dput(d_target);
        return NULL;
    }
    return d_target;
}

struct dentry *db_mkdir(struct dentry *parent, char *name){
    int err;
    pr_info("\n");
    
    struct dentry *child = d_alloc_name(parent, name);
    if(!child)
        return NULL;
    
    err = vfs_mkdir(&init_user_ns, d_inode(parent), child, (umode_t) DEFAULT_MODE_FILES);
    if(IS_ERR(err))
        return NULL;
    d_add(child, NULL); // ???
    return child;
}

int db_write_datafile(struct dentry *dentry, struct datafile *df){
    int err;
    char *buff;
    pr_info("\n");

    buff = long_to_string(df->ino);
    if(IS_ERR(buff))
        return PTR_ERR(buff);

    err = db_add_xattr(dentry, INO_NAME, buff);
    if(IS_ERR(err))
        return err;

    kfree(buff);

    buff = long_to_string(df->ino_parent);
    if(IS_ERR(buff))
        return PTR_ERR(buff);

    err = db_add_xattr(dentry, INO_PARENT_NAME, buff);
    kfree(buff);
    
    return err;
}

int db_read_datafile(struct dentry *dentry, struct datafile *df){
    char *ino, *ino_parent;
    pr_info("\n");
    ino = db_get_xattr(dentry, INO_NAME);
    if(!ino)
        return -ENOENT;

    df->ino = string_to_long(ino, BASE_NAME_BRANCH);

    kfree(ino);
    ino_parent = db_get_xattr(dentry, INO_PARENT_NAME);
    if(!ino_parent)
        return -ENOENT;

    df->ino_parent = string_to_long(ino_parent, BASE_NAME_BRANCH);
    kfree(ino_parent);
    return 0;
}


struct dentry *db_create_datafile(struct datafile *df, struct dentry *parent){
    struct dentry *dentry;
    int err;
    pr_info("\n");

    dentry = d_alloc_name(parent, df->name);
    if(!dentry)
        return -ENOMEM;

    err = db_create(dentry, parent);
    if(IS_ERR(err))
        goto out_err;

    err = db_write_datafile(dentry, df);
    if(!err)
        return dentry;

out_err:
    dput(dentry);
    return ERR_PTR(err);
}

char *get_name_branch_dir(struct dentry *dir){
    pr_info("\n");
    return db_get_xattr(dir, NEXT_NAME);
}

struct dentry *db_lookup_branch_dir(struct dentry *dir){
    struct dentry *branch_dir;
    char *name;
    pr_info("\n");
    
    name = get_name_branch_dir(dir);

    if(IS_ERR(name))
        return (struct dentry *)name;

    branch_dir = db_lookup_dentry_share(dir, name);

    kfree(name);
    return branch_dir;
}

int init_count_branch(struct dentry *dir){
    char *buff;
    int err;
    long init_nr = 0;
    pr_info("\n");

    buff = long_to_string(init_nr);

    if(IS_ERR(buff))
        return PTR_ERR(buff);
    
    err = db_add_xattr(dir, COUNT_SUBFILES_NAME, buff);
    kfree(buff);
    return err;
}

struct dentry *db_create_branch(struct dentry *dir){
    struct dentry *branch_dir;
    char *name;
    pr_info("\n");

    name = get_name_branch_dir(dir);
    if(IS_ERR(name))
        return (struct dentry *)name; // ??? copy code from db_lookup_branch_dir
    branch_dir = db_mkdir(dir, name);
    if(IS_ERR(branch_dir))
        return branch_dir;
    
    init_count_branch(branch_dir);

    return branch_dir;
}
long count_files(struct dentry *dir){ // :::
    long nr;
    pr_info("\n");
    char *buff = db_get_xattr(dir, COUNT_SUBFILES_NAME);
    if(IS_ERR(buff))
        return -1; // ???
    nr = string_to_long(buff, BASE_NAME_BRANCH);
    kfree(buff);
    return nr;
}

int set_count_files(struct dentry *dir, long nr){ // :::
    char *buff;
    int err;
    pr_info("\n");

    pr_info("nr: %ld\n", nr);


    buff = long_to_string(nr);

    if(IS_ERR(buff))
        return PTR_ERR(buff);
    
    err = db_set_xattr(dir, COUNT_SUBFILES_NAME, buff);
    kfree(buff);
    return err;
}


long update_next_branch(struct dentry *d_rmap){
    long nr;
    char *name;
    pr_info("\n");

    name = get_name_branch_dir(d_rmap);
    if(IS_ERR(name))
        return PTR_ERR(name);

    nr = string_to_long(name, BASE_NAME_BRANCH);
    if(IS_ERR(nr))
        return nr;
    nr += 1;
    set_next_branch(d_rmap, nr);
    return nr;
}

struct dentry *db_create_file_dmap(struct datafile *df, struct dentry *d_dmap){  // :::
    pr_info("\n");
    return db_create_datafile(df, d_dmap);
}

struct dentry *db_create_file_rmap(struct datafile *df, struct dentry *d_rmap, struct dentry *dmap_file){  // :::
    struct dentry *branch_dir, *new_dentry;
    int err;
    long nr_files;
again:
    branch_dir = db_lookup_branch_dir(d_rmap);

    if(IS_ERR_OR_NULL(branch_dir))
        branch_dir = db_create_branch(d_rmap);
    else if(nr_files = count_files(branch_dir) >  MAX_FILE_BRANCH - 1){
        update_next_branch(d_rmap);
        goto again;
    }
    
    if(IS_ERR(branch_dir))
        return PTR_ERR(branch_dir);

    new_dentry = d_alloc_name(branch_dir, df->name);

    if(!new_dentry)
        return -ENOMEM;
    
    err = db_link(dmap_file, new_dentry);
    if(IS_ERR(err)){
        dput(new_dentry);
        return ERR_PTR(err);
    }
    set_count_files(branch_dir, nr_files + 1);
    
    return new_dentry;
}

int iter_rmap(struct dir_context *ctx, const char *name, int len,
                loff_t pos, u64 ino, unsigned int d_type){
    
    int err = 0;
    struct dentry *d_branch, *dentry, *d_rmap;
    struct datafile *datafile;
    struct getdents_callback_rmap *getdents_rmap = container_of(ctx, struct getdents_callback_rmap, ctx);

    pr_info("\n");
    
    getdents_rmap->sequence++;
    d_rmap = getdents_rmap->d_rmap;
    datafile = getdents_rmap->datafile;

    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
		return 0;

    d_branch = db_lookup_dentry_share(d_rmap, name); // share

    if(!d_branch)
        return -ENOENT;

    dentry = db_lookup_dentry_share(d_branch, datafile->name); // share

    if(dentry){
        // todo: retrun dentry
        getdents_rmap->d_branch = d_branch;
        dput(dentry);
        return 0;
    }

    
    dput(d_branch);
    return err;
}



struct dentry *lookup_branch_by_datafile(struct datafile *df, struct dentry *d_rmap, struct vfsmount *mnt){
    /*
    * @df: datafile of file to remove
    * @d_rmap: dentry of rmap tag (in db)
    * 
    * lookup the dentry of branch
    * 
    * return dentry of branch if success, else return NULL.
    */

    struct file *file;
    int err;
    struct dentry *d_branch = NULL;
    pr_info("\n");

    file = open_file_dentry(d_rmap, mnt, O_RDONLY | O_DIRECTORY);
    if(!file)
        return NULL;
    
    struct getdents_callback_rmap getdents_rmap = {
		.ctx.actor = iter_rmap,
        .datafile = df,
        .d_rmap = d_rmap,
        .d_branch = NULL,
        .sequence = 0,
	};

    while(1){ // 1

        long old_seq = getdents_rmap.sequence;
		err = iterate_dir(file, &getdents_rmap.ctx);

        if(IS_ERR(err)) // ???
            break;

        if(old_seq == getdents_rmap.sequence)
			break;

        d_branch = getdents_rmap.d_branch;
        if(d_branch)
            break;
    }

    fput(file);
    return d_branch;

//    for branch_name in d_rmap
        // d_branch = db_lookup_dentry(d_rmap, branch_name);
        // dentry = db_lookup_dentry(d_branch, df->name);
        // if(dentry)
        //     dput(dentry)
        //     return d_branch
        // dput(d_branch)

}

int db_remove_file_rmap(struct datafile *df, struct dentry *d_rmap, struct vfsmount *mnt){
    /*
    * @df: datafile of file to remove
    * @d_rmap: dentry of rmap tag (in db)
    * @mnt: vfsmount of db
    * 
    * lookup the dentry of branch, after found branch
    * lookup the dentry of the file and unlink this file
    * decrement the counter of branch direcotry
    * 
    * return 0 if success, else return non-zero
    */

   struct dentry *branch_dir, *dentry;
   struct inode *inode;
   long nr_files;
   int err = 0;
   pr_info("\n");

    branch_dir = lookup_branch_by_datafile(df, d_rmap, mnt); // <<<
    if(!branch_dir)
        return -ENOENT;
    
    dentry = db_lookup_dentry_share(branch_dir, df->name); // share?
    if(!dentry){
        err = -ENOENT;
        goto out;
    }
    err = db_unlink(dentry);
    if(err)
        goto out;
    dput(dentry);

    inode = d_inode(branch_dir);
    
    // lock branch dir
    inode_lock_shared(inode);
    
    nr_files = count_files(branch_dir);
    if(!nr_files){
        // unlock branch dir
        inode_unlock_shared(inode);
        err = db_rmdir(branch_dir);
        goto out;
    }

    set_count_files(branch_dir, nr_files - 1);

    // unlock branch dir
    inode_unlock_shared(inode);

out:
    dput(branch_dir);
    return err;
}


int db_remove_file_dmap(struct datafile *df, struct dentry *d_dmap){  // :::
    /*
    * @df: datafile of file to remove
    * @d_dmap: dentry of parent file (in db)
    * 
    * unlink the file of dmap
    * 
    * return 0 if success, else return non-zero
    */

    struct dentry *dentry;
    int err;

    pr_info("\n");

    dentry = db_lookup_dentry_share(d_dmap, df->name); // share?
    if(!dentry)
        return -ENOENT;

    err = db_unlink(dentry);
    dput(dentry);
    return err;
}

int init_rmap(struct dentry *dir){
    pr_info("\n");
    return create_next_branch(dir, NEXT_INIT_VALUE);
}