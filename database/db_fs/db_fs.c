#include "db_fs.h"
#include "../database.h"

#include <linux/xattr.h>
#include <linux/string.h>

#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>

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


#define BASE_NAME_BRANCH 10
#define COUNT_SUBFILES_NAME "security.subfiles"

char *long_to_string(long num){
    char *buff;
    buff = kzalloc(sizeof(10), GFP_KERNEL);
    if(!buff)
        return -ENOMEM;
    sprintf(buff, "%ld", num);
    return buff;
}

long string_to_long(char *buff){
    long nr;
    int err;
    err = kstrtol(buff, BASE_NAME_BRANCH, &nr);
    if(IS_ERR(err))
        return err;
    return nr;
}


int db_create(struct dentry *dentry){
    umode_t mode = DEFAULT_MODE_FILES;
    return vfs_create(&init_user_ns, d_inode(dentry->d_parent), dentry, mode, true);
}


int db_rmdir(struct dentry *dentry){
    return vfs_rmdir(&init_user_ns, d_inode(dentry->d_parent), dentry);
}


int db_add_xattr(struct dentry *dentry, char *name, void *value){
    return vfs_setxattr(&init_user_ns, dentry, name, value, strlen(value), XATTR_CREATE);
}

int db_set_xattr(struct dentry *dentry, char *name, void *value){
    return vfs_setxattr(&init_user_ns, dentry, name, value, strlen(value), XATTR_REPLACE);
}

int db_link(struct dentry *old_dentry, struct dentry *new_dentry){
    int err;
    struct dentry *dir = dget_parent(new_dentry);
    err = vfs_link(old_dentry, &init_user_ns, d_inode(dir), new_dentry, NULL);
    dput(dir);
    return err;
}

char *db_get_xattr(struct dentry *dentry, char *name){
    int err;
    char *buff = kzalloc(XATTR_SIZE_MAX, GFP_KERNEL);
     if(!buff)
        return ERR_PTR(-ENOMEM);
    err = vfs_getxattr(&init_user_ns, dentry, name, buff, XATTR_SIZE_MAX);
    if(err < 0){
        kfree(buff);
        return ERR_PTR(err);
    }
    return buff;
}

int set_next_branch(struct dentry *dir, long nr){
    int err = 0;
    char *buff = long_to_string(nr);
    if(IS_ERR(buff))
        return PTR_ERR(buff);
    
    err = db_add_xattr(dir, NEXT_NAME, buff);
    kfree(buff);
    return err;
}


struct dentry *db_lookup_dentry(struct dentry *parent, char *name){ // :::
    struct dentry *d_target;
    struct inode *inode;
    if(!parent){
        return NULL;
    }
    
    inode = d_inode(parent);
    inode_lock(inode);
    d_target = lookup_one(&init_user_ns, name, parent, strlen(name));
    inode_unlock(inode);

    if(IS_ERR(d_target)){
        dput(d_target);
        return NULL;
    }

    if(d_target && !d_target->d_inode){
        dput(d_target);
        return NULL;
    }
    return d_target;
}

struct dentry *db_mkdir(struct dentry *parent, char *name){ // :::
    int err;
    umode_t mode = DEFAULT_MODE_FILES;
    struct dentry *child = d_alloc_name(parent, name);
    if(!child)
        return ERR_PTR(-ENOMEM);
    err = vfs_mkdir(&init_user_ns, d_inode(parent), child, mode);
    if(err < 0)
        return ERR_PTR(err);
    d_add(child, NULL); // ???
    return child;
}

int db_write_datafile(struct dentry *dentry, struct datafile *df){
    int err;
    char *buff;

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
    ino = db_get_xattr(dentry, INO_NAME);
    if(!ino)
        return -ENOENT;

    df->ino = string_to_long(ino);

    kfree(ino);
    ino_parent = db_get_xattr(dentry, INO_PARENT_NAME);
    if(!ino_parent)
        return -ENOENT;

    df->ino_parent = string_to_long(ino_parent);
    kfree(ino_parent);
    return 0;
}


struct dentry *db_create_datafile(struct datafile *df, struct dentry *parent){
    struct dentry *dentry;
    int err;

    dentry = d_alloc_name(parent, df->name);
    if(!dentry)
        return -ENOMEM;
    
    err = db_create(dentry);
    if(IS_ERR(err))
        goto out_err;

    err = db_write_datafile(dentry, df);
    if(err){
        goto out_err;
    }

    return dentry;

out_err:
    dput(dentry);
    return ERR_PTR(err);
}

char *get_name_branch_dir(struct dentry *dir){
    return db_get_xattr(dir, NEXT_NAME);
}

struct dentry *db_lookup_branch_dir(struct dentry *dir){
    struct dentry *branch_dir;
    char *name;
    
    name = get_name_branch_dir(dir);

    if(IS_ERR(name))
        return (struct dentry *)name;

    branch_dir = db_lookup_dentry(dir, name);

    kfree(name);
    return branch_dir;
}



int init_count_branch(struct dentry *dir){
    char *buff;
    int err;
    long init_nr = 0;

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
    char *buff = db_get_xattr(dir, COUNT_SUBFILES_NAME);
    if(IS_ERR(buff))
        return -1; // ???
    nr = string_to_long(buff);
    kfree(buff);
    return nr;
}

int set_count_files(struct dentry *dir, long nr){ // :::
    char *buff;
    int err;

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

    name = get_name_branch_dir(d_rmap);
    if(IS_ERR(name))
        return PTR_ERR(name);

    nr = string_to_long(name);
    if(IS_ERR(nr))
        return nr;
    nr += 1;
    set_next_branch(d_rmap, nr);
    return nr;
}

struct dentry *db_create_file_dmap(struct datafile *df, struct dentry *d_dmap){  // :::
    return db_create_datafile(df, d_dmap);
}

struct dentry *db_create_file_rmap(struct datafile *df, struct dentry *d_rmap, struct dentry *dmap_file){  // :::
    struct dentry *branch_dir, *new_dentry;
    int err;
    long nr_files;

    branch_dir = db_lookup_branch_dir(d_rmap);
    if(IS_ERR_OR_NULL(branch_dir))
        branch_dir = db_create_branch(d_rmap);
    else if(nr_files = count_files(branch_dir) >  MAX_FILE_BRANCH - 1){
        update_next_branch(d_rmap);
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

int db_remove_file_dmap(struct datafile *df, struct dentry *d_dmap){  // :::
    return 0;
}



int init_rmap(struct dentry *dir){
   return set_next_branch(dir, NEXT_INIT_VALUE);
}