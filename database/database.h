// database.h
#ifndef DATABASE_H_
#define DATABASE_H_

#include <linux/atomic.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/kref.h>


#define MAX_FILE_BRANCH 4096

#define DMAP_DIR_NAME "dmap0003"
#define RMAP_DIR_NAME "rmap0003"
#define BASE_NAME_BRANCH 10


#define TAG_MAGIC 1234


struct vtag {
    char *name;
    struct list_head db_tags;
    struct dentry *vdir;
    struct kref kref; // needed ???
    // long magic;
};

struct vbranch {
    char *name; // ?
    unsigned long nr; // ?
    atomic_t is_stale;
    struct list_head dentries;
    struct list_head child;
    unsigned long flag;
	struct mutex vbranch_lock;
    struct db_tag* db_tag;
    struct kref kref;
};

struct db_tag {
    char *name;
    struct dentry *dir;
    struct super_block *sb;

    struct list_head child;

    struct list_head vbranchs;
	struct mutex vbranchs_lock;

    unsigned long flag;
    struct vtag *vtag;
    
    struct vfsmount *mnt;

    struct path path; // delete
};

struct tag_context {
    struct vtag *vtag;

    struct file *file_tag;
    struct file *file_branch;

    struct db_tag *db_tag_cursor;
    struct vbranch *vbranch_cursor;
    struct dentry_list *dentry_cursor;

    unsigned long flag;

	spinlock_t vbranch_cursor_lock;
	spinlock_t db_tag_cursor_lock;
	spinlock_t dentry_cursor_lock;

	struct mutex file_tag_lock;
	struct mutex file_branch_lock;

    bool is_locked_vbranch;
};

struct dentry_list {
    struct list_head child;
    struct dentry *dentry;
    struct vtag *vtag;
    struct vbranch *vbranch;
    unsigned long flag;
};

struct datafile {
    char *name;
    unsigned long ino;
    unsigned long ino_parent;
    struct super_block *sb;
};

struct db_file {
    struct datafile *datafile;
    char *branch_name;
};


// ------------------------------------------

// classes ???


struct database {
    struct list_head t_child;
    struct super_block *sb;
};



// <<<< add count ref
struct branch { // 16 bytes + sizeof(list_head) ?= 20~24 bytes ???
    char *name;
    unsigned long nr;
    atomic_t is_stale;
    struct list_head subdirs; // list of all dentries in branch
    struct list_head child; // cursor in tag parent list

    struct file *filp; // ???? needed
    struct dentry *dir;

    struct kref kref;
    // struct path path;
    // struct list_head d_subdirs; // ???
};


struct db_branch {
    char *name;
    unsigned long nr;
    struct file *filp;
    struct dentry *dir;
    // struct path path;
};



#include "db_fs/db_fs.h" // ::: ???


extern int taged_file(struct dentry *d_file, char *name, struct vfsmount *mnt);
extern int untaged_file(struct dentry *d_file, char *name, struct vfsmount *mnt);

// extern struct tag *lookup_tag_test(char *name);
// extern struct datafile *lookup_datafile(struct db_tag *tag, char *name);
// extern struct branch *lookup_branch(struct db_tag *tag, unsigned long nr);

// extern struct vtag *alloc_vtag(void); // :::
// extern struct db_tag *alloc_db_tag(void); // :::
// extern struct datafile *alloc_datafile(void);
// extern struct tag_context *alloc_tag_context(void);

// extern void put_branch(struct branch *branch);
// extern void put_db_tag(struct db_tag *db_tag);
// extern void put_vtag(struct vtag *vtag);
// extern void put_datafile(struct datafile *datafile);
// extern void put_tag_context(struct tag_context *tag_ctx);
// extern void put_database(struct database *db);
// extern void put_dentry_list(struct dentry_list *dentry_list);




// extern int fill_tag(struct db_tag *tag, char *name, struct dentry *dir);
// extern int fill_branch(struct branch *branch, char *name, struct dentry *dir);
// extern int fill_datafile(struct datafile *datafile, char *name, struct dentry *dir);


// extern bool is_branch_stale(struct branch *branch);

// extern int list_add_sb(struct list_head *list, struct super_block *sb);
// extern struct db_tag *lookup_tag_by_sb(struct super_block *sb, char *name);

// extern struct vbranch *add_vbranch(struct list_head *list, struct vbranch *vbranch);

#endif /* DATABASE_H_ */
