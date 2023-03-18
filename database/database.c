#include "database.h"
#include "../publisher/publisher.h"
#include "../utils/utils.h"

#include "../vtagfs.h" // to pr_info




int taged_file(struct dentry *dentry, char *name, struct vfsmount *mnt);
int untaged_file(struct dentry *dentry, char *name, struct vfsmount *mnt);

struct dentry *lookup_tag_dir(struct super_block *sb, char *name){
    pr_info("\n");
    return db_lookup_dentry_share(sb->s_root, name);
}

struct dentry *create_tag(struct super_block *sb, char *name){
    /*
    * create new tag dir and ramp, dmap.
    * return dir of tag if success, else return NULL
    */

    struct dentry *dir_tag, *dmap, *rmap;
    pr_info("\n");

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
   pr_info("\n");

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
    * return non-negative value if success, else return negative.
    * the return value if success is name of the branch
    */
    
    struct dentry *dmap, *rmap, *dmap_file, *rmap_file, *d_branch;
    int err = -ENOENT;
    pr_info("\n");

    dmap = db_lookup_dentry(dir, DMAP_DIR_NAME);
    if(!dmap)
        return -ENOENT;

    rmap = db_lookup_dentry(dir, RMAP_DIR_NAME);
    if(!rmap)
        goto out_err_rmap;


    dmap_file = db_create_file_dmap(datafile, dmap); // <<<
    if(!dmap_file)
        goto out_err_dmap_file;

    rmap_file = db_create_file_rmap(datafile, rmap, dmap_file); // <<<
    dput(dmap_file);
    if(!rmap_file){
        db_remove_file_dmap(datafile, dmap); // <<<
        goto out_err_dmap_file;
    }
    
    d_branch = dget_parent(rmap_file);
    if(!d_branch)
        goto err_d_branch;
    
    pr_info("rmap_file: %s\n", d_branch->d_name.name);
    err = (int)(string_to_long(d_branch->d_name.name, BASE_NAME_BRANCH));
    dput(d_branch);

err_d_branch:
    dput(rmap_file);


out_err_dmap_file:
    dput(rmap);

out_err_rmap:
    dput(dmap);

    return err;
}

struct db_file *add_file_to_tag(struct dentry *dentry, struct dentry *dir){
    /*
    * @dentry: dentry of file to taged
    * @dir: dir of taged in db
    * 
    * return db_file if success, else return NULL
    * the return value if success is name of the branch
    */

    struct datafile *datafile;
    struct db_file *db_file;
    int err;
    long nr_branch;
    pr_info("\n");

    datafile = alloc_datafile();
    if(!datafile)
        return NULL;

    err = fill_datafile_from_dentry(datafile, dentry); // <<<
    if(err)
        return NULL;

    nr_branch = db_create_datafiles(datafile, dir);
    if(nr_branch < 0)
        return NULL;

    db_file = alloc_db_file();

    if(!db_file)
        return NULL;

    db_file->datafile = datafile;
    db_file->branch_name = long_to_string(nr_branch);

    if(!db_file->datafile)
        free_db_file(db_file);

    return db_file;
}

int taged_file(struct dentry *dentry, char *name, struct vfsmount *mnt){
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
    struct db_file *db_file;

    char *name_branch;

    int err = 0;
    pr_info("\n");

    dir = lookup_tag_dir(dentry->d_sb, name);
    if(!dir)
        dir = create_tag(dentry->d_sb, name);
    
    if(!dir)
        return -ENOENT;
    
    db_file = add_file_to_tag(dentry, dir);
    dput(dir);

    if(!db_file)
        return -ENOENT; // ???
   
    update_add_tag_cache(name, mnt, db_file);
    free_db_file(db_file);
    return 0;
}


int untaged_file(struct dentry *dentry, char *name, struct vfsmount *mnt){ // <<<
    /*
    * return 0 if success, else return non-zero.
    * todo: this function !!!
    */
   pr_info("\n");

    struct dentry *dir, *d_dmap, *d_rmap;
    struct datafile *datafile;
    int err = -ENOENT;

    dir = lookup_tag_dir(dentry->d_sb, name);
    if(!dir)
        return -ENOENT;
    
    datafile = alloc_datafile();
    if(!datafile)
        goto out_err_datafile;

    err = fill_datafile_from_dentry(datafile, dentry);
    if(err)
        goto out_err_dmap;

    d_dmap = db_lookup_dentry_share(dir, DMAP_DIR_NAME);
    if(!d_dmap)
        goto out_err_dmap;

    d_rmap = db_lookup_dentry_share(dir, RMAP_DIR_NAME);
    if(!d_rmap)
        goto out_err_rmap;

    err = db_remove_file_dmap(datafile, d_dmap);
    if(err)
        goto out;
    err = db_remove_file_rmap(datafile, d_rmap, mnt);

out:
    dput(d_rmap);
out_err_rmap:
    dput(d_dmap);
out_err_dmap:
    free_datafile(datafile);
out_err_datafile:
    dput(dir);
    return err;
}