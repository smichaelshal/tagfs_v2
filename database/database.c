#include "database.h"

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



struct tag *alloc_tag(void){ //::: use slab
    return kzalloc(sizeof(struct tag), GFP_KERNEL);
}

struct branch *alloc_branch(void){ //::: use slab
    return kzalloc(sizeof(struct branch), GFP_KERNEL);
}

struct datafile *alloc_datafile(void){ //::: use slab
    return kzalloc(sizeof(struct datafile), GFP_KERNEL);
}

void put_tag(struct tag *tag){
    dput(tag->dir);
    path_put(&tag->path);
    kfree(tag);
}
void put_datafile(struct datafile *df){
    kfree(df);
}


int fill_tag(struct tag *tag, char *name, struct dentry *dir){
    char *buff_name, *buff_path, *full_path;
    int err;

    if(!tag)
        return -EINVAL;
    
    buff_name = kzalloc(strlen(name), GFP_KERNEL);
    if(!buff_name)
        return -ENOMEM;

    buff_path = kzalloc(PATH_MAX, GFP_KERNEL);
    if(!buff_path)
        return -ENOMEM;

    tag->dir = dget(dir);

    
    
    full_path = dentry_path_raw(tag->dir, buff_path, PATH_MAX);
    if(!IS_ERR(full_path)){
        pr_info("full_path: %s\n", full_path);
        err = kern_path(full_path, 0, &tag->path);
    }else
        err = PTR_ERR(full_path);

    kfree(buff_path);
    if(err){
        return err;
        kfree(buff_name);
    }

    strcpy(buff_name, name);
    tag->name = buff_name;

    return 0;
}

int fill_datafile_from_dentry(struct datafile *df, struct dentry *d_file){
    char *buff_name, *name;
    struct dentry *dir;

    if(!df)
        return -EINVAL;

    
    name = d_file->d_name.name;

    buff_name = kzalloc(strlen(name), GFP_KERNEL);
    if(!buff_name)
        return -ENOMEM;
    
    strcpy(buff_name, name);
    dir = dget_parent(d_file); // check errors ???

    df->name = buff_name;
    df->ino = d_file->d_inode->i_ino;
    df->ino_parent = dir->d_inode->i_ino;

    dput(dir);
    return df;
}

struct tag *lookup_tag_by_sb(struct super_block *sb, char *name);
struct tag *create_tag(struct super_block *sb, char *name);
int taged_file_by_tag(struct dentry *d_file, struct tag *tag);

int remove_datafiles(struct tag *tag, char *name){ // :::
    return 0;
}

int taged_file(struct dentry *d_file, char *name){
    struct tag *tag;
    int err;

    tag = lookup_tag_by_sb(d_file->d_inode->i_sb, name);
    if(IS_ERR_OR_NULL(tag))
        tag = create_tag(d_file->d_inode->i_sb, name);
    if(!tag)
        return -ENOENT;
    
    err = taged_file_by_tag(d_file, tag);
    return err;
}

int untaged_file(struct dentry *d_file, char *name){
    // lookup tag
    // untaged file
    struct tag *tag;
    int err;

    tag = lookup_tag_by_sb(d_file->d_inode->i_sb, name);
    if(!tag)
        return -ENOENT;
    
    err = remove_datafiles(tag, name);
    put_tag(tag);
    return err;
}

struct tag *lookup_tag(char *name){

    struct path root;
    struct super_block *sb;
    struct tag *tag;
    
    root = current->fs->root;
    sb = root.dentry->d_inode->i_sb;

    tag = lookup_tag_by_sb(sb, name);
    return tag;
    

    /* todo:
        for sb in all_sb:
            lookup_tag_by_sb(sb, name);
    */
}

struct datafile *lookup_datafile(struct tag *tag, char *name){
    return NULL;
}

struct branch *lookup_branch(struct tag *tag, unsigned long nr){
    return NULL;
}

struct tag *lookup_tag_by_sb(struct super_block *sb, char *name){ // ~~~
    struct dentry *dir_tag, *dmap, *rmap;
    struct tag *tag;
    int err;

    dir_tag = db_lookup_dentry(sb->s_root, name);
    if(!dir_tag)
        return NULL;
    
    tag = alloc_tag();
    err = fill_tag(tag, name, dir_tag);
    if(IS_ERR(err))
        return ERR_PTR(err);
    return tag;
}


struct tag *create_tag(struct super_block *sb, char *name){
    struct tag *tag, *err;
    struct dentry *dir_tag, *child;

    pr_info("start create_tag1\n");
    

    dir_tag = db_mkdir(sb->s_root, name);
    if(IS_ERR(dir_tag))
        return (struct tag *)(dir_tag);

    child = db_mkdir(dir_tag, DMAP_DIR_NAME);
    if(IS_ERR(child)){
        tag = (struct tag*)child;
        goto out;
    }
    dput(child);

    child = db_mkdir(dir_tag, RMAP_DIR_NAME);
    if(IS_ERR(child)){
        tag = (struct tag*)child;
        goto out;
    }

    init_rmap(child);

    dput(child);
    tag = alloc_tag();
    fill_tag(tag, name, dir_tag);

out:
    dput(dir_tag);
    return tag;
}
int db_create_datafiles(struct datafile *df, struct tag *tag){  // :::
    // create new file in dmap and rmap
    struct dentry *dir, *dmap, *rmap, *d_file, *dmap_file, *rmap_file;
    int err = 0;
    dir = tag->dir;

    pr_info("start db_create_datafiles1\n");
    

    dmap = db_lookup_dentry(dir, DMAP_DIR_NAME);
    if(!dmap)
        return -ENOENT;
    
    rmap = db_lookup_dentry(dir, RMAP_DIR_NAME);
    if(!rmap){
        err = -ENOENT;
        goto out_err_rmap;
    }
        
    dmap_file = db_create_file_dmap(df, dmap);
    if(IS_ERR(dmap_file)){
        err = PTR_ERR(dmap_file);
        goto out;
    }
    
    rmap_file = db_create_file_rmap(df, rmap, dmap_file);
    if(IS_ERR(rmap_file)){
        err = PTR_ERR(rmap_file);
        db_remove_file_dmap(df, dmap);
    }

    dput(rmap_file);
    dput(dmap_file);

out:
    dput(rmap);
out_err_rmap:
    dput(dmap);

    return err;
}

int taged_file_by_tag(struct dentry *d_file, struct tag *tag){ // ~~~
    struct datafile *datafile;
    pr_info("start taged_file_by_tag1\n");
    

    datafile = alloc_datafile();
    if(!datafile)
        return -ENOMEM;

    fill_datafile_from_dentry(datafile, d_file);
    db_create_datafiles(datafile, tag);
    return 0;
}

int fill_branch(struct branch *branch, char *name, struct dentry *dir){
    struct dentry *dentry;
    dentry = db_lookup_dentry(dir, name);
    if(!dentry)
        return -ENOENT; 
    
    branch->name = name;
    branch->dir = dentry;
    return 0;
}

int fill_datafile(struct datafile *datafile, char *name, struct dentry *dir){
    struct dentry *dentry;
    dentry = db_lookup_dentry(dir, name);

    if(!dentry)
        return -ENOENT;

    datafile->name = name;
    db_read_datafile(dir, datafile);
   
   dput(dentry);
    return 0;
}