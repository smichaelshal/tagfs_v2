#include "database.h"
#include "../publisher/publisher.h"
#include "../utils/utils.h"


int taged_file(struct dentry *dentry, char *name);
int untaged_file(struct dentry *dentry, char *name);


struct dentry *lookup_tag_dir(struct super_block *sb, char *name){
   return db_lookup_dentry_share(sb->s_root, name);
}

struct dentry *create_tag(struct super_block *sb, char *name){
    /*
    * create new tag dir and ramp, dmap.
    * return dir of tag if success, else return NULL
    */

    struct dentry *dir_tag, *dmap, *rmap;

    dir_tag = db_mkdir(sb->s_root, name);
    if(!dir_tag)
        return NULL;

    dmap = db_mkdir(dir_tag, DMAP_DIR_NAME);
    if(!dmap)
        goto out_err;
    
    dput(dmap);
    rmap = db_mkdir(dir_tag, RMAP_DIR_NAME);
    if(!rmap)
        goto out_err;

    init_rmap(rmap); // <<<
    dput(rmap);
    return dir_tag;
    
out_err:
    dput(dir_tag);
    return NULL;

}

int fill_datafile_from_dentry(struct datafile *datafile, struct dentry *dentry){
    /*
    *
    */

   struct dentry *parent;

    parent = dget_parent(dentry);
    if(!parent)
        return -ENOENT;
    
    datafile->ino_parent = parent->d_inode->i_ino;
    dput(parent);

    datafile->ino = dentry->d_inode->i_ino;
    datafile->sb = dentry->d_sb;

    datafile->name = dup_name(dentry->d_name.name);
    if(!datafile->name){
        kfree(datafile);
        return -ENOMEM;
    }

    return 0;
}

int db_create_datafiles(struct datafile *datafile, struct dentry *dir){
    /*
    * @datafile: datafile of file to taged
    * @dir: dir of tag in db
    * 
    *  return 0 if success, else return non-zero.
    */
    
    struct dentry *dmap, *rmap, *dmap_file, *rmap_file;
    int err = -ENOENT;

    dmap = db_lookup_dentry(dir, DMAP_DIR_NAME);
    if(!dmap)
        return -ENOENT;

    rmap = db_lookup_dentry(dir, RMAP_DIR_NAME);
    if(!rmap)
        goto out_err_rmap;

    dmap_file = db_create_file_dmap(datafile, dmap); // <<<
    if(!dmap_file)
        goto out_err_dmap_file;

    dput(dmap_file);
    rmap_file = db_create_file_rmap(datafile, rmap, dmap_file); // <<<
    if(!rmap_file)
        db_remove_file_dmap(datafile, dmap); // <<<
    else
        dput(rmap_file);

    err = 0;

out_err_dmap_file:
    dput(rmap);

out_err_rmap:
    dput(dmap);

    return err;
}

int add_file_to_tag(struct dentry *dentry, struct dentry *dir){
    /*
    * @dentry: dentry of file to taged
    * @dir: dir of taged in db
    * 
    * return 0 if success, else return non-zero
    */

    struct datafile *datafile;
    int err;

    datafile = alloc_datafile();
    if(!datafile)
        return -ENOMEM;

    err = fill_datafile_from_dentry(datafile, dentry); // <<<
    if(err)
        return err;
    err = db_create_datafiles(datafile, dir);
    return err;
}

int taged_file(struct dentry *dentry, char *name){
    /*
    * taged file
    * @dentry: dentry of the file want taged.
    * @name: name of tag
    * 
    * return 0 if success, else return non-zero.
    * 
    * if before call the function the tag db in disk not exist,
    * the function create new tag db
    */

    struct dentry *dir;
    int err;

    dir = lookup_tag_dir(dentry->d_sb, name);
    if(!dir)
        dir = create_tag(dentry->d_sb, name);
    if(!dir)
        return -ENOENT;
    
    err = add_file_to_tag(dentry, dir);
    dput(dir);

    return err;
}

int untaged_file(struct dentry *dentry, char *name){ // <<<
    /*
    * return 0 if success, else return non-zero.
    * todo: this function !!!
    */

    struct dentry *dir;
    int err;

    dir = lookup_tag_dir(dentry->d_sb, name);
    if(!dir)
        return -ENOENT;

    return 0;
}