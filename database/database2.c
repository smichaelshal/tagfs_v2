// // database.c

// #include "database.h"
// // #define DBG_VERSION "1.6"
// // #undef pr_fmt
// // #define pr_fmt(fmt) "%s: %s: " fmt, __func__, DBG_VERSION





// struct db_tag *alloc_db_tag(void){ //::: use slab
//     pr_info("\n");
//     return kzalloc(sizeof(struct db_tag), GFP_KERNEL);
// }

// // struct vtag *alloc_vtag(void){ //::: use slab
// //     pr_info("\n");
// //     return kzalloc(sizeof(struct vtag), GFP_KERNEL);
// // }

// // void put_vtag(struct vtag *vtag){
// //     pr_info("\n");
// //     // if(vtag->vdir->d_lockref.count != 1) // <<<< <=
// //     //     return;
// //     // dput(vtag->vdir);
// //     kfree(vtag->name);
// //     kfree(vtag);
// // }


// // struct tag_context *alloc_tag_context(void){ //::: use slab
// //     pr_info("\n");
// //     return kzalloc(sizeof(struct tag_context), GFP_KERNEL);
// // }



// // struct datafile *alloc_datafile(void){ //::: use slab
// //     pr_info("\n");
// //     return kzalloc(sizeof(struct datafile), GFP_KERNEL);
// // }

// void put_db_tag(struct db_tag *tag){
//     pr_info("\n");
//     kfree(tag->name);
//     dput(tag->dir);
//     // fput(tag->filp);
//     // put_branch(tag->last_branch);
//     path_put(&tag->path); // <<<< <=
//     kfree(tag);
// }

// // void put_datafile(struct datafile *df){
// //     pr_info("\n");
// //     kfree(df);
// // }

// // void put_tag_context(struct tag_context *tag_ctx){
// //     pr_info("\n");
// //     // put_vtag(tag_ctx->vtag);
// //     // put_db_tag(tag_ctx->db_tag);

// //     if(!IS_ERR_OR_NULL(tag_ctx->file_tag))
// //         fput(tag_ctx->file_tag);

// //     if(!IS_ERR_OR_NULL(tag_ctx->file_branch))
// //         fput(tag_ctx->file_branch);
// //     kfree(tag_ctx);
// // }

// void release_branch(struct branch *branch){
//     pr_info("\n");
//     kfree(branch->name);
//     kfree(branch);
// }

// void put_branch(struct branch *branch){
//     pr_info("\n");
//     // fput(branch->filp);
//     dput(branch->dir);
// }



// void put_database(struct database *db){
//     pr_info("\n");
//     kfree(db);
// }

// // void put_dentry_list(struct dentry_list *dentry_list){
// //     pr_info("\n");
// //     kfree(dentry_list);
// // }

// // bool is_branch_stale(struct branch *branch){ // <<<
// //     pr_info("\n");
// // 	return !!atomic_read(&branch->is_stale);
// // }




// int fill_tag(struct db_tag *tag, char *name, struct dentry *dir){
//     char *buff_name, *buff_path, *full_path;
//     int err;
//     pr_info("\n");

//     if(!tag)
//         return -EINVAL;

//     // kref_init(&tag->refcount);

//     // INIT_LIST_HEAD(&tag->dbs);
//     // INIT_LIST_HEAD(&tag->sub_branchs);
    
//     buff_name = kzalloc(strlen(name), GFP_KERNEL);
//     if(!buff_name){
//         return -ENOMEM;
//     }

    
//     buff_path = kzalloc(PATH_MAX, GFP_KERNEL);
   
//     if(!buff_path){
//         return -ENOMEM;
//     }

//     tag->dir = dget(dir);
    
//     full_path = dentry_path_raw(tag->dir, buff_path, PATH_MAX);


//     if(!IS_ERR(full_path)){
//         pr_info("full_path: %s\n", full_path);
//         err = kern_path(full_path, 0, &tag->path);
//     }else
//         err = PTR_ERR(full_path);

//     kfree(buff_path);
//     if(err){
//         kfree(buff_name);
//         return err;
//     }
//     strcpy(buff_name, name);
//     tag->name = buff_name;
//     // tag->magic = TAG_MAGIC;
//     return 0;
// }

// int fill_datafile_from_dentry(struct datafile *df, struct dentry *d_file){
//     char *buff_name, *name;
//     struct dentry *dir;
//     pr_info("\n");

//     if(!df)
//         return -EINVAL;

    
//     name = d_file->d_name.name;

//     buff_name = kzalloc(strlen(name), GFP_KERNEL);
//     if(!buff_name)
//         return -ENOMEM;
    
//     strcpy(buff_name, name);
//     dir = dget_parent(d_file); // check errors ???

//     df->name = buff_name;
//     df->ino = d_file->d_inode->i_ino;
//     df->ino_parent = dir->d_inode->i_ino;

//     dput(dir);
//     return df;
// }

// struct db_tag *lookup_tag_by_sb(struct super_block *sb, char *name);
// struct db_tag *create_tag(struct super_block *sb, char *name);
// int taged_file_by_tag(struct dentry *d_file, struct db_tag *tag);

// int remove_datafiles(struct db_tag *tag, char *name){ // :::
//     pr_info("\n");
//     return 0;
// }



// int untaged_file(struct dentry *d_file, char *name){
//     // lookup tag
//     // untaged file
//     struct db_tag *tag;
//     int err;
//     pr_info("\n");

//     tag = lookup_tag_by_sb(d_file->d_inode->i_sb, name);
//     if(!tag)
//         return -ENOENT;
    
//     err = remove_datafiles(tag, name);
//     put_db_tag(tag);
//     return err;
// }

// int list_add_sb(struct list_head *list, struct super_block *sb){
//     struct database *database;
//     // struct database *db;
//     // struct super_block *sb_tmp;
//     pr_info("\n");

// 	database = kzalloc(sizeof(struct database), GFP_KERNEL);
//     if(!database)
//         return -ENOMEM;

//     INIT_LIST_HEAD(&database->t_child);
//     database->sb = sb;
//     list_add(&database->t_child, list);

//     // db = list_entry(list->next, struct database, t_child);
//     // sb_tmp = db->sb;
//     return 0;
// }

// struct datafile *lookup_datafile(struct db_tag *tag, char *name){
//     pr_info("\n");
//     return NULL;
// }

// struct branch *lookup_branch(struct db_tag *tag, unsigned long nr){
//     pr_info("\n");
//     return NULL;
// }

// struct db_tag *lookup_tag_by_sb(struct super_block *sb, char *name){ // ~~~
//     struct dentry *dir_tag, *dmap, *rmap;
//     struct db_tag *tag;
//     int err;
//     pr_info("\n");


//     dir_tag = db_lookup_dentry_share(sb->s_root, name);
//     if(!dir_tag)
//         return NULL;
    
//     tag = alloc_db_tag();
//     err = fill_tag(tag, name, dir_tag);
//     if(IS_ERR(err))
//         return ERR_PTR(err);
//     return tag;
// }


// struct db_tag *create_tag(struct super_block *sb, char *name){
//     struct db_tag *tag, *err;
//     struct dentry *dir_tag, *child;

//    pr_info("\n");
    

//     dir_tag = db_mkdir(sb->s_root, name);
//     if(IS_ERR(dir_tag))
//         return (struct db_tag *)(dir_tag);

//     child = db_mkdir(dir_tag, DMAP_DIR_NAME);
//     if(IS_ERR(child)){
//         tag = (struct db_tag*)child;
//         goto out;
//     }
//     dput(child);

//     child = db_mkdir(dir_tag, RMAP_DIR_NAME);
//     if(IS_ERR(child)){
//         tag = (struct db_tag*)child;
//         goto out;
//     }

//     init_rmap(child);

//     dput(child);
//     tag = alloc_db_tag();
//     fill_tag(tag, name, dir_tag);

// out:
//     dput(dir_tag);
//     return tag;
// }
// int db_create_datafiles(struct datafile *df, struct db_tag *tag){  // :::
//     // create new file in dmap and rmap
//     struct dentry *dir, *dmap, *rmap, *d_file, *dmap_file, *rmap_file;
//     int err = 0;
//     dir = tag->dir;

//     pr_info("\n");
    

//     dmap = db_lookup_dentry(dir, DMAP_DIR_NAME);
//     if(!dmap)
//         return -ENOENT;
    
//     rmap = db_lookup_dentry(dir, RMAP_DIR_NAME);
//     if(!rmap){
//         err = -ENOENT;
//         goto out_err_rmap;
//     }
        
//     dmap_file = db_create_file_dmap(df, dmap);
//     if(IS_ERR(dmap_file)){
//         err = PTR_ERR(dmap_file);
//         goto out;
//     }
    
//     rmap_file = db_create_file_rmap(df, rmap, dmap_file);
//     if(IS_ERR(rmap_file)){
//         err = PTR_ERR(rmap_file);
//         db_remove_file_dmap(df, dmap);
//     }

//     dput(rmap_file);
//     dput(dmap_file);

// out:
//     dput(rmap);
// out_err_rmap:
//     dput(dmap);

//     return err;
// }

// int taged_file_by_tag(struct dentry *d_file, struct db_tag *tag){ // ~~~
//     struct datafile *datafile;
//     pr_info("\n");
    

//     datafile = alloc_datafile();
//     if(!datafile)
//         return -ENOMEM;

//     fill_datafile_from_dentry(datafile, d_file);
//     db_create_datafiles(datafile, tag);
//     return 0;
// }

// int fill_branch(struct branch *branch, char *name, struct dentry *dir){
//     struct dentry *dentry;

//     dentry = db_lookup_dentry_share(dir, name);
//     if(!dentry)
//         return -ENOENT; 
//     pr_info("fill_branch: dentry: %s\n", dentry->d_name.name);
    
//     branch->name = name;
//     branch->dir = dentry;
//     INIT_LIST_HEAD(&branch->subdirs);
//     return 0;
// }

// // int fill_datafile(struct datafile *datafile, char *name, struct dentry *dir){
// //     struct dentry *dentry;
// //     pr_info("start fill_datafile\n");
// //     pr_info("fill_datafile dir: %s\n", dir->d_name.name);
// //     pr_info("fill_datafile name: %s\n", name);
// //     dentry = db_lookup_dentry_share(dir, name);

// //     if(!dentry)
// //         return -ENOENT;

// //     datafile->name = name;
// //     db_read_datafile(dentry, datafile);
   
// //     pr_info("end fill_datafile\n");
// //    dput(dentry);
// //     return 0;
// // }