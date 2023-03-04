// inode->i_private = vtag // inode of vdir
// dentry->d_fsdata = vtag // ???
// file->private_data = tag_ctx // file of vdir

#define NR_BRANCH_READ_AHEAD 16

#define SKIP_MODE 1
#define LAST_LOCKING 2
#define CURSOR_MODE 4

struct getdents_callback_tag {
	struct dir_context ctx;
	long sequence;
    long mode;
    struct db_tag *db_tag;
};

struct getdents_callback_branch {
	struct dir_context ctx;
	long sequence;
    long mode;
    struct vbranch *vbranch;
    struct vtag *vtag;
    struct super_block *sb;
};

// ------------- list utils -----------------

// ----- dentry_list -----

struct dentry_list *add_dentry_list(struct list_head *list, struct dentry_list *dentry_list){
    list_add(&new_dentry_list->child, list);
    return new_dentry_list;
}

struct dentry_list *add_dentry_cursor(struct list_head *list){
    struct dentry_list *cursor;
    cursor = kzalloc(sizeof(struct dentry_list), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add(&cursor->child, list);
    return cursor;
}

struct dentry_list *get_next_dentry_list(struct dentry_list *cursor, struct list_head *head){
    struct list_head *next;
    struct dentry_list *current_dentry_list;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_dentry_list = list_entry(next, struct dentry_list, child);
    return current_dentry_list;
}

struct dentry_list *get_current_dentry_list(struct dentry_list *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_dentry_list(cursor, head);
    return cursor;
}

void update_dentry_list_cursor(struct dentry_list *cursor, struct dentry_list *next){
    list_move(&cursor->child, &next->child);
}

struct dentry_list *get_current_update_dentry(struct dentry_list *cursor, struct list_head *head, spinlock_t *lock){
    struct dentry_list *current_dentry_list;
    if(lock)
        spin_lock(lock);
    current_dentry_list = get_current_dentry_list(cursor, head);
    if(!current_dentry_list)
        goto out;
    
    update_dentry_list_cursor(cursor, current_dentry_list);
out:
    if(lock)
        spin_unlock(lock);
    return current_dentry_list;
}

// ----- vbranch -----

struct vbranch *add_vbranch(struct list_head *list, struct vbranch *vbranch){
    list_add(&new_vbranch->child, list);
    return new_vbranch;
}

struct vbranch *add_vbranch_cursor(struct list_head *list){
    struct vbranch *cursor;
    cursor = kzalloc(sizeof(struct vbranch), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add_tail(&cursor->child, list);
    return cursor;
}

struct vbranch *get_next_vbranch(struct vbranch *cursor, struct list_head *head){
    struct list_head *next;
    struct vbranch *current_vbranch;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_vbranch = list_entry(next, struct vbranch, child);
    return current_vbranch;
}

struct vbranch *get_current_vbranch(struct vbranch *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_vbranch(cursor, head);
    return cursor;
}

void update_vbranch_cursor(struct vbranch *cursor, struct vbranch *next){
    list_move(&cursor->child, &next->child);
}

struct vbranch *get_current_update_vbranch(struct vbranch *cursor, struct list_head *head, spinlock_t *lock){
    struct vbranch *current_vbranch;
    if(lock)
        spin_lock(lock);
    current_vbranch = get_current_vbranch(cursor, head);
    if(!current_vbranch)
        goto out;
    
    update_vbranch_cursor(cursor, current_vbranch);
out:
    if(lock)
        spin_unlock(lock);
    return current_vbranch;
}


// ----- db_tag -----

struct db_tag *add_db_tag(struct list_head *list, struct db_tag *db_tag){
    list_add(&new_db_tag->child, list);
    return new_db_tag;
}

struct db_tag *add_db_tag_cursor(struct list_head *list){
    struct db_tag *cursor;
    cursor = kzalloc(sizeof(struct db_tag), GFP_KERNEL);
    if(!cursor)
        return NULL;
    INIT_LIST_HEAD(&cursor->child);
    cursor->flag |= CURSOR_MODE;
    list_add_tail(&cursor->child, list);
    return cursor;
}

struct db_tag *get_next_db_tag(struct db_tag *cursor, struct list_head *head){
    struct list_head *next;
    struct db_tag *current_db_tag;

    next = cursor->child.next;
    if(next == head)
        return NULL;

	current_db_tag = list_entry(next, struct db_tag, child);
    return current_db_tag;
}

struct db_tag *get_current_db_tag(struct db_tag *cursor, struct list_head *head){
    while(cursor && cursor->flag & CURSOR_MODE)
        cursor = get_next_db_tag(cursor, head);
    return cursor;
}

void update_db_tag_cursor(struct db_tag *cursor, struct db_tag *next){
    list_move(&cursor->child, &next->child);
}

struct db_tag *get_current_update_db_tag(struct db_tag *cursor, struct list_head *head, spinlock_t *lock){
    struct db_tag *current_db_tag;
    if(lock)
        spin_lock(lock);
    current_db_tag = get_current_db_tag(cursor, head);
    if(!current_db_tag)
        goto out;
    
    update_db_tag_cursor(cursor, current_db_tag);
out:
    if(lock)
        spin_unlock(lock);
    return current_db_tag;
}

// -----------------------------------------------

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
    struct dentry *vdir;

    vtag = create_vtag(dentry->dname.name);
    vdir = create_vdir(dentry->dname.name); // <<<
    return vdir;
}

struct vtag create_vtag(char *name){
    /*
    * Create vtag and fill relevant data
    * return vtag, if error return NULL
    */

    vtag = init_vtag(name); // <<<
    lookup_db_tags(vtag);
    return vtag;
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

    if(is_taged_mount(mnt))
        dir = lookup_tag_by_mount(mnt, vtag->name);
        if(dir)
            db_tag = create_db_tag(dir);
            if(db_tag)
                add_db_tag(&vtag->db_tags, db_tag); // <<<
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

    iterate_mounts(scan_mounts, vtag, current->fs->root.mnt);        
    return 0;
}

bool is_taged_mount(struct vfsmount *mnt){ // <<<
    /*
    * return true if the mount contain taged files, else return false
    */
}

struct dentry *lookup_tag_by_mount(struct vfsmount *mnt, char *name){ // <<<
    /*
    * Lookup in this mount the dir of tag
    * return dir (dentry) if found, else return NULL
    */
}

struct db_tag *create_db_tag(struct dentry *dir){ // <<<
    /*
    * Get the dir (on disk) of tag
    * return db_tag of this dir if success, else return NULL
    */
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
    struct dentry *d_file = NULL;

    cursor = add_db_tag_cursor(&vtag->db_tags); // <<<
    if(!cursor)
        return NULL;
    
    current_db_tag = cursor;

    while(1){
        current_db_tag = get_current_update_db_tag(cursor, &vtag->db_tags, NULL); // <<<
        if(!current_db_tag)
            break;

        datafile = lookup_datafile_in_db(db_tag, dentry->d_name.name);
        if(datafile){
            d_file = load_datafile(vtag, datafile);
            if(d_file)
                goto out;
        }
    }
out:
    delete_db_tag_cursor(cursor); // <<<
    return d_file;
}

struct datafile *lookup_datafile_in_db(struct db_tag *db_tag, char *name){ // <<<
    /*
    * Lookup datafile in db_tag by name
    * Search in dmap datafile by name
    *
    * if found return datafile, else return NULL
    */
}

struct dentry *load_datafile(struct vtag *vtag, struct datafile *datafile){ // <<<
    /*
    * Get datafile and vtag.
    *
    * From this datafile create dentry and load to dcache
    * return dentry of file if successs, else return NULL
    */
}

int tag_dir_open(struct inode *inode, struct file *file){
    /*
    * Create and add tag_context to file
    * return 0 if success, else retrun non-zero
    */

    tag_ctx = init_tag_context(inode->i_private);
    if(!tag_ctx)
        return -1;
    file->private_data = tag_ctx;
    return 0;
}

int tag_dir_close(struct inode *inode, struct file *file){
    /*
    * put tag context and disconnet tag_context from file
    * return 0
    */
    
    put_tag_context(file->private_data);
    file->private_data = NULL;
    return 0;
}

struct tag_context *init_tag_context(struct vtag *vtag){ // <<<
    /*
    * Create initialized tag_context
    * return tag_context if success, else return NULL
    */

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

	spin_lock_init(tag_ctx->vbranch_cursor_lock);
	spin_lock_init(tag_ctx->db_tag_cursor_lock);
	spin_lock_init(tag_ctx->dentry_cursor_lock);

	mutex_init(tag_ctx->file_tag_lock);
	mutex_init(tag_ctx->file_branch_lock);

    return tag_ctx;
}

int put_tag_context(struct tag_context *tag_ctx){ // <<<
    /*
    * put tag_context
    * return 0 if success else return non-zero
    */
}

int tag_readdir(struct file *file, struct dir_context *ctx){
    /*
    * Emit files from current branch.
    * 
    * That means the branch must be cached.
    * return 0
    */

    bool loop = true;
    long count = 0;
    struct tag_context *tag_ctx = (struct tag_context *)file->private_data;

retry:
    db_tag = get_db_tag(tag_ctx); // <<< ^^^
    if(!db_tag)
        return 0;
    vbranch = get_vbranch_lock(tag_ctx, db_tag); // <<< ^^^
    if(!vbranch)
        return 0;

    count = scan_vbranch(ctx, vbranch, tag_ctx);
    if(!count && loop){
        loop = false;
        goto retry;
    }
    return 0;
}

int scan_vbranch(struct dir_context *ctx, struct vbranch *vbranch, struct tag_context *tag_ctx){
    /*
    * Scan the branch and emit files.
    * Returns the number of files it emit
    *
    * If cursor is end, free vbranch
    */
    struct dentry_list *cursor, *current_dentry;
    long count = 0;

    if(!tag_ctx->dentry_cursor)
        tag_ctx->dentry_cursor = add_dentry_cursor(&vbranch->dentries); // <<<
        
    cursor = tag_ctx->dentry_cursor;
    current_dentry = cursor;

    while(1){
        current_dentry = get_current_update_dentry(cursor, &vbranch->dentries, &tag_ctx->dentry_cursor_lock); // <<<
        if(!current_dentry)
            break;

        if(!emit_dentry(ctx, current_dentry->dentry)){
            // Check if the last file is missing !!!
            // Check if you need to update one to the previous one
            break;
        }
        count++;
    }

    if(is_vbranch_in_end(cursor)) // <<<
        free_vbranch() // <<<
            // delete cursor dentry
            // updtae cursor vbranchs
            // unlock branch
    return count;
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
    
    inode = d_inode(dentry);
    if(!inode) // needed?
        return false;
    
    name = dentry->d_name.name;
    len = dentry->d_name.len;
    type = dt_type(inode);
    ino = inode->i_ino;

    return dir_emit(ctx, name, len, ino, type);
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

    vbranch = fast_lookup_vbranch(tag_ctx, db_tag);
    if(vbranch && is_branch_loaded(vbranch)){
        if(lock_vbranch(vbranch))
            return vbranch;
    }
    vbranch = slow_lookup_vbranch(tag_ctx, db_tag, vbranch);
    if(!vbranch)
        return NULL;
    
    if(is_branch_loaded(vbranch))
        return lock_vbranch(vbranch);
    else
        return load_vbranch(tag_ctx, vbranch);
}

struct vbranch *fast_lookup_vbranch(struct tag_context *tag_ctx,struct db_tag *db_tag){
    /*
    * return vbranch if success, else return NULL
    *
    * The search is only done in the cache (the lists from db_tag).
    */

    struct vbranch *current_vbranch;
    struct vbranch *cursor, *vbranch_cursor;

    if(!tag_ctx->vbranch_cursor)
        tag_ctx->vbranch_cursor = add_vbranch_cursor(&db_tag->vbranchs); // <<<
        
    cursor = tag_ctx->vbranch_cursor;
    current_vbranch = cursor;

    return get_current_vbranch(cursor, &db_tag->vbranchs); // <<<
}

bool lock_vbranch(struct vbranch *vbranch){ // <<< 
    /*
    * try lock branch, if success return true, else return false
    *
    * Solve the problem of repeated locking by the same caller (same context) !!!
    */
}

bool is_branch_loaded(struct vbranch *vbranch){ // <<<
    /*
    * return true if the branch alredy loaded to cache, else return false
    */   
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

retry_readahead:
    if(!vbranch){
        if(!readahead_vbranchs(tag_ctx, db_tag, NR_BRANCH_READ_AHEAD)){
            db_tag = update_next_db_tag(tag_ctx);
            if(!db_tag)
                return NULL;
            move_vbranch_cursor(tag_ctx->vbranch_cursor, &db_tag->vbranchs); // <<<
        }
        vbranch = get_current_vbranch(tag_ctx->vbranch_cursor, &db_tag->vbranchs);
        goto retry_readahead;
    }

    if(is_vbranch_stale(vbranch)) // <<<
        drop_vbranch(vbranch); // <<<
    return vbranch;
}

struct vbranch *load_vbranch(struct tag_context *tag_ctx, struct vbranch *vbranch){ // <<<
    /*
    * get branch and makes sure the vbranch loaded,
    * if not alredy loaded load the branch.
    *
    * return vbranch if success, else return NULL
    *
    * The function can get base vbranch
    * (vbranch not loaded) or vbranch (loaded).
    */
    
    struct file *file_branch;

    mutex_lock(&vbranch->vbranch_lock);

    if(!tag_ctx->file_branch)
        tag_ctx->file_branch = open_file_branch(tag_ctx, vbranch); // <<<
    file_branch = tag_ctx->file_branch;

    if(!file_branch){
        vbranch = NULL;
        goto out;
    }

    struct getdents_callback_branch getdents_branch = {
		.ctx.actor = iter_branch,
        .sb = db_tag->sb,
        .vbranch = vbranch,
        .vtag = tag_ctx->vtag,
        .sequence = 0,
        .mode = 0,
	};

    while(1){
        long old_seq = getdents_branch.sequence;
		err = iterate_dir(file_branch, &getdents_branch.ctx);

        if(IS_ERR(err)) // ???
            break;
        
        if(old_seq == getdents_branch.sequence){
            tag_ctx->file_branch = NULL;
            fput(file_branch);
			break;
        }
    }

out:
    mutex_unlock(&vbranch->vbranch_lock);
    if(vbranch)
        clean_stale(vbranch);
    return vbranch;
}


struct db_tag *get_db_tag(struct tag_context *tag_ctx){
    /*
    * return the current db_tag.
    * if no more db_tags return NULL.
    *
    * In the end of db_tag must update before call more times !!!
    */

    struct db_tag *cursor, *current_db_tag;

    if(!tag_ctx->dentry_cursor)
        tag_ctx->dentry_cursor = add_db_tag_cursor(&tag_ctx->vtag->db_tags);
    
    cursor = tag_ctx->db_tag_cursor;
    current_db_tag = cursor;

    return get_current_db_tag(cursor, &tag_ctx->vtag->db_tags); // <<<
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

    long count = 0;
    int err;
    struct file *file_tag;

    mutex_lock(&db_tag->vbranchs_lock);
    vbranch = fast_lookup_vbranch(tag_ctx, db_tag);
    if(vbranch)
        goto out;
    
    if(!tag_ctx->file_tag)
        tag_ctx->file_tag = open_file_tag(tag_ctx, db_tag);
    file_tag = tag_ctx->file_tag;

    if(!file_tag)
        return 0;

    struct getdents_callback_tag getdents_tag = {
		.ctx.actor = iter_tag,
        .db_tag = db_tag,
        .sequence = 0,
        .mode = 0,
	};

    while(max_count--){
        long old_seq = getdents_tag.sequence;
		err = iterate_dir(file_tag, &getdents_tag.ctx);

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

struct db_tag *update_next_db_tag(struct tag_context *tag_ctx){
    /*
    * Moves the cursor to the next db_tag
    *
    * Return the current db_tag, if success else return NULL
    */
    if(tag_ctx->file_tag){
        fput(tag_ctx->file_tag);
        tag_ctx->file_tag = NULL;
    }
    get_current_update_db_tag(tag_ctx->db_tag_cursor, &tag_ctx->vtag->db_tags, &tag_ctx->db_tag_cursor_lock); // <<<
    return get_current_db_tag(tag_ctx->db_tag_cursor, &tag_ctx->vtag->db_tags);
}

struct file *open_file_tag(struct tag_context *tag_ctx, struct db_tag *db_tag){  // <<<
    /*
    * open rmap dir from db_tag
    * return rmap file if success, else return NULL
    */

    struct dentry *rmap;
    struct file *filp = NULL;

    if(!tag_ctx->file_tag){
        mutex_lock(&tag_ctx->file_tag_lock);

        if(tag_ctx->file_tag){
            filp = tag_ctx->file_tag;
            goto out;
        }
        rmap = lookup_rmap(db_tag->dir); // <<<
        if(!rmap)
            goto out;
        filp = open_file_dentry(rmap, O_RDONLY | O_DIRECTORY); // <<<
out:
        mutex_unlock(&tag_ctx->file_tag_lock);
        return filp;
    }
}



int iter_tag(struct dir_context *ctx, const char *name, int len,
                loff_t pos, u64 ino, unsigned int d_type){  // <<<
    
    int err = 0;
    struct db_tag *db_tag;
    struct getdents_callback_tag *getdents_tag = container_of(ctx, struct getdents_callback_tag, ctx);
    
    db_tag = getdents_tag->db_tag;
    getdents_tag->sequence++;

    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
		return 0;

    if(!(getdents_tag->mode & SKIP_MODE))
        load_base_vbranch(getdents_tag->db_tag, name); // <<<
    return err;
}

struct vbranch *load_base_vbranch(struct db_tag *db_tag, char *name){
    /*
    * load the base of vbranch to list in db_tag.
    * return vbranch if success, else return NULL
    */
    struct vbranch *vbranch;

    vbranch = alloc_vbranch(); // <<<
    if(!vbranch)
        return NULL;
    init_vbranch(vbranch, db_tag, name);
    return vbranch;
}

void init_vbranch(struct vbranch *vbranch, struct db_tag *db_tag, char *name){
    vbranch->name = name;
    vbranch->flag = 0;
    INIT_LIST_HEAD(&vbranch->dentries);
    INIT_LIST_HEAD(&vbranch->child);
    clean_stale(vbranch);
    mutex_init(vbranch->vbranch_lock);

    add_vbranch(&db_tag->vbranchs, vbranch); // <<<
}

int iter_branch(struct dir_context *ctx, const char *name, int len,
                loff_t pos, u64 ino, unsigned int d_type){  // <<<
    
    int err = 0;
    struct super_block *sb;
    struct vbranch *vbranch;
    struct vtag *vtag;
    struct dentry *dentry;
    struct datafile *datafile;
    struct dentry_list *dentry_list;

    struct getdents_callback_branch *getdents_branch = container_of(ctx, struct getdents_callback_branch, ctx);
    
    getdents_branch->sequence++;
    sb = getdents_branch->sb;
    vbranch = getdents_branch->vbranch;
    vtag = getdents_branch->vtag;

    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
		return 0;

    ctx->pos++; // ???

    if((getdents_branch->mode & SKIP_MODE))
        return 0;
    
    datafile = init_datafile(sb, name, ino); // <<<
    if(!datafile)
        return -ENOMEM;
    dentry = load_datafile(vtag, datafile);
    if(dentry){
        dentry_list = init_dentry_list(dentry); // <<<
        if(dentry_list)
            add_dentry_list(&vbranch->dentries, dentry_list);  // <<<
        else
            err = -ENOMEM;
    }
    put_datafile(datafile);
    return err;
}