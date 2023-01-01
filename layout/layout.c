#include "layout.h"
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/file.h>

// #include <linux/cred.h>


// #include <linux/fs.h> 
// #include <linux/dcache.h>
// #include <linux/fdtable.h>

// #include <linux/fs_struct.h>

// #include <linux/init.h>
// #include <linux/rcupdate.h>

// #include <linux/exportfs.h>
// #include <linux/module.h>
// #include <linux/mount.h>
// #include <linux/namei.h>
// #include <linux/sched.h>

#include "hardlinking/hardlinking.h"
#include "../vtagfs.h"

#define DIR_TAG ".tag_u36"
#define DIR_CONTAIN "contain_u36"
#define DIR_THIS "this_u36"

#define DEFAULT_MODE_FILES 0755


#define DOT_STR "."
#define DOTDOT_STR ".."

struct dentry_component {
   struct list_head list;
   char *name;
   unsigned long ino; // ??? need?
};


struct vfsmount *my_vfsmount;

struct getdents_callback { // ^^^
	struct dir_context ctx;
	char *name;		/* name that was found. It already points to a <<<???
				   buffer NAME_MAX+1 is size */
	u64 ino;		/* the inum we are looking for */
	int sequence;		/* sequence counter */
    // struct dentry *parent; // ??? delete field
    struct list_head *dentry_component_lists; // %%%
};


void delete_dentry_component(struct dentry_component *dc){ // ^^^
    list_del(&dc->list);
    kfree(dc);
}

struct list_head *init_dentry_components(void){ // ^^^
    struct list_head *list_dc = kzalloc(sizeof(struct list_head), GFP_KERNEL);
    INIT_LIST_HEAD(list_dc);
    return list_dc;
}


int add_dentry_component(struct list_head *list, char *name, unsigned long ino){ // ^^^
    struct dentry_component *dc;
    int len = strlen(name);
    if(len > NAME_MAX)
        return -ENAMETOOLONG;

    dc = kzalloc(sizeof(struct dentry_component), GFP_KERNEL);
    if(!dc)
        return -ENOMEM;
    
    dc->name = kzalloc(NAME_MAX, GFP_KERNEL);
    
    if(!dc->name){
        kfree(dc);
        return -ENOMEM;
    }

    memcpy(dc->name, name, len); // memcpy ???
    dc->name[len] = '\0';
    dc->ino = ino;

    INIT_LIST_HEAD(&dc->list);
    list_add_tail(&dc->list, list);
    return 1;
}

static int filldir_one(struct dir_context *ctx, const char *name, int len, // ^^^
			loff_t pos, u64 ino, unsigned int d_type)
{
	struct getdents_callback *buf =
		container_of(ctx, struct getdents_callback, ctx);
	int result = 0;
    int err;
    struct list_head *list_dc = buf->dentry_component_lists;
    

	buf->sequence++;
    // pr_info("name1: %s, ino1: %ld\n", name, ino);
    // if(buf->parent){
    //     parent = buf->parent;
    //     pr_info("parent1: %s, p_ino1: %ld\n", parent->d_name.name, parent->d_inode->i_ino);
    // }
    if(!strcmp(name, DOT_STR) || !strcmp(name, DOTDOT_STR))
       goto out;
       
    err = add_dentry_component(list_dc, name, ino); // ^^^&&&
    if(IS_ERR_OR_NULL(err)){
        pr_info("err6\n");
        goto out;
    }

    // tmp_child = lookup_dentry(parent, filename);
    // lookup_tag(tmp_child);
out:
	return result;
}

static int fill_list_subfiles(struct dentry *base, struct list_head *list_dc)
{   
	const struct cred *cred = current_cred();
	struct inode *dir = base->d_inode;
	int error;
	struct file *file;
    

    struct path base_path = {
		.mnt = my_vfsmount,
		.dentry = base,
	};

	struct getdents_callback buffer = {
		.ctx.actor = filldir_one,
        .dentry_component_lists = list_dc, // ^^^
	};

	error = -ENOTDIR;
	if (!dir || !S_ISDIR(dir->i_mode))
		goto out;
	error = -EINVAL;
	if (!dir->i_fop)
		goto out;
	
	file = dentry_open(&base_path, O_RDONLY, cred);
	error = PTR_ERR(file);
	if (IS_ERR(file))
		goto out;

	error = -EINVAL;
	if (!file->f_op->iterate && !file->f_op->iterate_shared)
		goto out_close;

	buffer.sequence = 0;
	while (1) {
        int old_seq = buffer.sequence;
		error = iterate_dir(file, &buffer.ctx);
		if (error < 0)
			break;

		error = -ENOENT;
		if (old_seq == buffer.sequence)
			break;
	}

out_close:
	fput(file);
out:
	return error;
}

// lookup dentry by name, if not found return NULL.
struct dentry *lookup_dentry(struct dentry *dentry, char *name){
    struct dentry *d_target;
    struct inode *inode;
    if(!dentry){
        pr_info("dentry is null\n");
        return NULL;
    }

    if(dentry && !dentry->d_inode){
        pr_info("d_inode is null, d_inode: %zu, i_rwsem: %zu, d_subdirs: %zu\n", offsetof(struct dentry, d_inode), offsetof(struct inode, i_rwsem), offsetof(struct dentry, d_subdirs));
    }

    pr_info("rrr0\n");
    pr_info("rrr0.1 dentry: %s, name: %s\n", dentry->d_name.name, name);
    
    inode = d_inode(dentry);
    pr_info("rrr1\n");
    inode_lock(inode);
    pr_info("rrr2\n");
    d_target = lookup_one(&init_user_ns, name, dentry, strlen(name));
    pr_info("rrr3\n");
    inode_unlock(inode);
    pr_info("rrr4\n");

    if(IS_ERR(d_target)){
        pr_info("rrr5\n");
        dput(d_target);
        pr_info("rrr6\n");
        return NULL;
    }

    pr_info("rrr7\n");
    if(d_target && !d_target->d_inode){
        pr_info("rrr8\n");
        dput(d_target);
        return NULL;

    }
    return d_target;
}

// lookup dentry by name, if not found create new directory.
struct dentry *force_lookup(struct dentry *dentry, char *name, umode_t mode){
    struct dentry *child;
    int err;
    child = lookup_dentry(dentry, name);
    if(child && child->d_inode){
        goto out;
    }
    if(child){
        pr_info("childsss03\n"); // <<<< // dput(child);
        pr_info("child: d_lockref:\n", child->d_lockref.count); // <<<< // dput(child);
        dput(child);
    }
    // else
    child = d_alloc_name(dentry, name);
   
    if(!child)
        return ERR_PTR(-ENOMEM);
    
    err = vfs_mkdir(&init_user_ns, d_inode(dentry), child, mode);
    d_add(child, NULL); // ??? &&&
    if(err < 0)
        return ERR_PTR(err);
out:
    return child;
}

// get directory (base) and build in the directory the "contain" layout.
struct dentry *build_layout_contain_single(struct dentry *base, char *tag_name, struct dentry *back_dir, char *back_dir_name, umode_t mode){
    struct dentry *root_tag, *dir_tag, *dir_contain, *d_link;
    root_tag = force_lookup(base, DIR_TAG, mode);

    if(IS_ERR_OR_NULL(root_tag)){
        pr_info("err1\n");
        return NULL;
    }
    
    dir_tag = force_lookup(root_tag, tag_name, mode);
    dput(root_tag);
    
    if(IS_ERR_OR_NULL(dir_tag)){
        pr_info("err2\n");
        return NULL;
    }
    
    dir_contain = force_lookup(dir_tag, DIR_CONTAIN, mode);
    if(IS_ERR_OR_NULL(dir_contain)){
        pr_info("err3\n");
        dput(dir_tag);
        return NULL;
    }

    // d_link = lookup_dentry(dir_contain, back_dir_name);
    d_link = NULL;
    pr_info("d_link_0\n");
    pr_info("dir_contain0, name: %s, ino: %ld\n", dir_contain->d_name.name, dir_contain->d_inode->i_ino);

    if(IS_ERR_OR_NULL(d_link))
        pr_info("d_link_0 is err or null\n");
    else{     
        pr_info("back_dir_name1: %s\n", back_dir_name);
        pr_info("d_link_0.1: %s, ino: %ld\n", d_link->d_name.name);
    }
    
    
    

    if(!d_link){
        pr_info("d_link_1\n");
        d_link = link_any(back_dir, dir_contain, back_dir_name); // <<<
        pr_info("d_link_2\n");
    }else{
        pr_info("d_link_2.1\n");
    }

    if(d_link){
        pr_info("d_link_3\n");
        dput(d_link); // ???<<<
    }else{
        pr_info("d_link_4\n");
    }
    
    pr_info("d_link_5\n");
   
    dput(dir_contain);
    return dir_tag;
}

int build_layout_contain_loop(struct dentry *base, char *tag_name, struct dentry *back_dir, char *back_dir_name, umode_t mode){
    struct dentry *parent;

    while(!IS_ROOT(base)){
        pr_info("base: %s\n", base->d_name.name);
        back_dir = build_layout_contain_single(base, tag_name, back_dir, back_dir_name, mode);
        if(!back_dir)
            return -ENOENT;
        
        // if(!back_dir) // true if error <<<
        //     return NULL;
        back_dir_name = base->d_name.name;
        parent = dget_parent(base);
        // if(!parent) // true if error <<<
        //     return NULL;

        dput(base);
        base = parent;
    }

    build_layout_contain_single(base, tag_name, back_dir, back_dir_name, mode);
    dput(back_dir);
    dput(base);
    return 1;
}


int build_layout_this(struct dentry *dir_tag, struct dentry *d_target, umode_t mode){
    int err;
    struct dentry *d_link;
    struct dentry *d_this = force_lookup(dir_tag, DIR_THIS, mode);
    if(IS_ERR_OR_NULL(d_this))
        return -ENOENT; // x
    
    pr_info("d_link1: %s\n", d_target->d_name.name);
    d_link = NULL;
    // d_link = lookup_dentry(d_this, d_target->d_name.name);
    if(!d_link){
        pr_info("d_link2\n");
        d_link = link_any(d_target, d_this, d_target->d_name.name); // <<<
        pr_info("d_link3\n");
    }

    if(d_link){
        pr_info("d_link4\n");
        dput(d_link);
    }
    else{
        pr_info("d_link5\n");
        return -ENOENT; // x
    }

    pr_info("d_link6\n");
    
    dput(d_this);
    return 1;
}

struct dentry *build_layout_tag(struct dentry *dentry, char *tag_name, umode_t mode){
    struct dentry *dir, *root_tag, *dir_tag;
    dir = dget_parent(dentry);
    if(!dir)
        return NULL;
    
    root_tag = force_lookup(dir, DIR_TAG, mode);
    dput(dir);

    if(IS_ERR_OR_NULL(root_tag)){
        pr_info("err4\n");
        return NULL;
    }
    
    dir_tag = force_lookup(root_tag, tag_name, mode);
    dput(root_tag);

    if(IS_ERR_OR_NULL(dir_tag)){
        pr_info("err5, root_tag: %s, tag_name: %s\n", root_tag->d_name.name, tag_name);
        return NULL;
    }

    return dir_tag;
}

int build_layout(struct dentry *dentry, char *tag_name){
    struct dentry *dir_tag, *parent, *dir;
    char *dir_name;
    umode_t mode = DEFAULT_MODE_FILES;
    int err = 0;

    dir_tag = build_layout_tag(dentry, tag_name, mode);
    if(!dir_tag)
        return -ENOENT; // X
    
    err = build_layout_this(dir_tag, dentry, mode);
    if(IS_ERR_OR_NULL(err)){
        pr_info("err7\n");
        goto out;
    }
    
    
    dir = dget_parent(dentry);

    if(IS_ROOT(dir)){ // <<<
        dput(dir);
        goto out;
    }
    
    parent = dget_parent(dir);
    dir_name = dir->d_name.name;
    dput(dir);

    err = build_layout_contain_loop(parent, tag_name, dir_tag, dir_name, mode);
    dput(parent); // <<<? &&&
    if(IS_ERR_OR_NULL(err)){
        pr_info("err5\n");
        // pr_info("err5: parent: %s, tag_name: %s, dir_tag: %s, dir_name: %s\n", parent->d_name.name, tag_name, dir_tag->d_name.name, dir_name);
        // pr_info("err5_01,  parent: %s, tag_name: %s\n", parent->d_name.name, tag_name);
        // if(dir_tag)
        //     pr_info("err5_02, dir_tag: %s\n", dir_tag->d_name.name);
        // else
        //     pr_info("err5_02, dir_tag is null\n");
        // if(dir_name)
        //     pr_info("err5_02, dir_name: %s\n", dir_name);
        // else
        //     pr_info("err5_02, dir_name is null\n");

        goto out;
    }
    
     // <<<?
out:
    dput(dir_tag);// <<<?
    return err;
}

struct dentry *add_tag(unsigned int fd, char *tag_name){
    struct file *file;
    struct dentry *dentry;
    struct vfsmount *mnt;

    pr_info("fd: %d\n", fd);

    if(!fd)
        goto out;

    file = fget(fd);

    if(!file)
        goto out;

    dentry = file->f_path.dentry;
    mnt = file->f_path.mnt;

    build_layout(dentry, tag_name); // return value ???
    
    // parent = dget_parent(dentry);
    // dput(parent);
    fput(file);
out:
    return NULL;
}

struct dentry *lookup_tag(struct dentry *root){
    struct dentry *dir_contain, *dir_this, *tmp_child, *dentry;
    struct list_head *dentry_component_list = init_dentry_components();
    struct dentry_component *ptr, *next;
    char *copy_name;

    pr_info("ee1\n");

    dir_this = lookup_dentry(root, DIR_THIS);

    pr_info("ee2\n");
    
    if(dir_this){
        dentry = dir_this;
        fill_list_subfiles(dir_this, dentry_component_list); // ^^^

        list_for_each_entry_safe(ptr, next, dentry_component_list, list){
            copy_name = ptr->name;
            delete_dentry_component(ptr);

            pr_info("ee3\n");

            tmp_child = lookup_dentry(dir_this, copy_name);

            
            if(tmp_child){
                pr_info("childdd3: %s, ino: %ld\n", tmp_child->d_name.name, tmp_child->d_inode->i_ino);

                // pr_info("d_subdirs= %zu\n", offsetof(struct dentry, d_subdirs)); // <<<

                dput(tmp_child);
                kfree(copy_name);
                pr_info("ee4\n");
            }

            pr_info("ee5\n");
        }

        pr_info("ee6\n");
        
        dput(dir_this);

        pr_info("ee7\n");
    }

    pr_info("ee8\n");
    
    dir_contain = lookup_dentry(root, DIR_CONTAIN);

    pr_info("ee9\n");
   
    if(dir_contain){
        pr_info("ee10\n");
        // ------------------
        fill_list_subfiles(dir_contain, dentry_component_list); // dentry_components // ^^^
        
        list_for_each_entry_safe(ptr, next, dentry_component_list, list){
            copy_name = ptr->name;
            delete_dentry_component(ptr);
            pr_info("ee11\n");
            tmp_child = lookup_dentry(dir_contain, copy_name);
            if(tmp_child){
                lookup_tag(tmp_child);
                dput(tmp_child);
                pr_info("ee12\n");
                kfree(copy_name);
            }
            pr_info("ee13\n");
        }
        pr_info("ee14\n");
        // ------------------
        dput(dir_contain);
        pr_info("ee15\n");
    }
    pr_info("ee16\n");

    return NULL;
}

struct dentry *lookup_tag_root(char *tag_name){
    struct dentry *root_tag, *dir_tag;
    struct path root;
    int err;
    // char *tmp_str;

    // task_lock(&init_task);
    // get_fs_root(init_task.fs, &root);
    // task_unlock(&init_task);

    err = kern_path("/", LOOKUP_DIRECTORY, &root);
    if(err){
        pr_info("root lookup faild: %d\n", err);
        return NULL;
    }
    my_vfsmount = root.mnt;

    pr_info("ww1\n");

    root_tag = lookup_dentry(root.dentry, DIR_TAG);
    if(!root_tag) // error <<<
        goto out;

    pr_info("ww2\n");

    dir_tag = lookup_dentry(root_tag, tag_name);
    dput(root_tag);

    pr_info("ww3\n");

    if(!dir_tag) // error <<<
        goto out;

    pr_info("ww4\n");
    
    lookup_tag(dir_tag); // ??? <<<is_err
    dput(dir_tag);

    pr_info("ww5\n");
    
out:
    path_put(&root);
    return NULL;
}
