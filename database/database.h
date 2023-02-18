// database.h
#ifndef DATABASE_H_
#define DATABASE_H_

#include <linux/atomic.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/file.h>

#define MAX_FILE_BRANCH 4096

#define DMAP_DIR_NAME "dmap0003"
#define RMAP_DIR_NAME "rmap0003"

#define TAG_MAGIC 1234


// classes ???

struct tag_context {
    struct vtag *vtag;
    struct db_tag *db_tag;
    // struct dentry *d_cursor;
    struct list_head *cursor_tag;
    struct list_head *cursor_branchs; // ??? btree or list
    struct list_head cursor_subdirs;
    
    struct file *file_tag; // the file of dir of rmap. for exmaple: /mnt/vtagfs/red/rmap
    struct file *file_branch; // the file of dir of branch in rmap. for exmaple:  /mnt/vtagfs/red/rmap/1

    // struct kref refcount;
};

struct database {
    struct list_head t_child;
    struct super_block *sb;
};

struct dentry_list {
    struct list_head d_child;
    struct dentry *dentry;
};


struct datafile {
    char *name;
    unsigned long ino;
    unsigned long ino_parent;
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

    // struct kref refcount;
    // struct path path;
    // struct list_head d_subdirs; // ???
};


// struct tag {
//     long magic;
//     char *name;
//     struct list_head sub_branchs;
//     struct list_head dbs;
//     struct dentry *vdir; // dir in the dcache (with ramfs)

//     struct dentry *dir; // dir in the disk
//     struct file *filp;
//     struct branch *last_branch;
//     struct path path; // the path of the db (on disk)
//     // struct dentry *dmap;
//     // struct dentry *rmap;
// };

// <<<< add count ref
struct db_tag {
    char *name;
    struct dentry *dir; // dir in the disk
    struct file *filp;
    struct branch *last_branch;
    struct path path; // the path of the db (on disk)

    // struct kref refcount;
};

struct vbranch { // 16 bytes + sizeof(list_head) ?= 20~24 bytes ???
    char *name;
    unsigned long nr;
    atomic_t is_stale;
    struct list_head subdirs; // list of all dentries in branch
    struct list_head child; // cursor in tag parent list

    // struct list_head d_subdirs; // ???
};

struct db_branch {
    char *name;
    unsigned long nr;
    struct file *filp;
    struct dentry *dir;
    // struct path path;
};


struct vtag { // <<<< add count ref
    long magic;
    char *name;
    struct list_head sub_branchs;
    struct list_head dbs;
    struct dentry *vdir; // dir in the dcache (with ramfs)
};

#include "db_fs/db_fs.h" // ::: ???


extern int taged_file(struct dentry *d_file, char *name);
extern int untaged_file(struct dentry *d_file, char *name);

// extern struct tag *lookup_tag_test(char *name);
extern struct datafile *lookup_datafile(struct db_tag *tag, char *name);
extern struct branch *lookup_branch(struct db_tag *tag, unsigned long nr);

extern struct branch *alloc_branch(void);
extern struct vtag *alloc_vtag(void); // :::
extern struct db_tag *alloc_db_tag(void); // :::
extern struct datafile *alloc_datafile(void);
extern struct tag_context *alloc_tag_context(void);

extern void put_branch(struct branch *branch);
extern void put_db_tag(struct db_tag *db_tag);
extern void put_vtag(struct vtag *vtag);
extern void put_datafile(struct datafile *datafile);
extern void put_tag_context(struct tag_context *tag_ctx);
extern void put_database(struct database *db);
extern void put_dentry_list(struct dentry_list *dentry_list);




extern int fill_tag(struct db_tag *tag, char *name, struct dentry *dir);
extern int fill_branch(struct branch *branch, char *name, struct dentry *dir);
extern int fill_datafile(struct datafile *datafile, char *name, struct dentry *dir);


extern void make_stale(struct branch *branch);
extern void clean_stale(struct branch *branch);
extern bool is_branch_stale(struct branch *branch);

extern int list_add_sb(struct list_head *list, struct super_block *sb);
extern struct db_tag *lookup_tag_by_sb(struct super_block *sb, char *name);

#endif /* DATABASE_H_ */
