#ifndef DATABASE_H_
#define DATABASE_H_

#include <linux/atomic.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/file.h>

#define MAX_FILE_BRANCH 4096

#define DMAP_DIR_NAME "dmap0003"
#define RMAP_DIR_NAME "rmap0003"

// classes ???

struct datafile {
    char *name;
    unsigned long ino;
    unsigned long ino_parent;
};

struct branch { // 16 bytes + sizeof(list_head) ?= 20~24 bytes ???
    char *name;
    unsigned long nr;
    atomic_t is_stale;
    struct dentry *dir;
    struct file *filp;
    struct path path;
    // struct list_head d_subdirs; // ???
};

struct tag {
    char *name;
    struct dentry *dir; // dir in the disk
    struct dentry *vdir; // dir in the dcache (with ramfs)
    struct file *filp;
    struct branch *last_branch;
    struct path path; // the path of the db (on disk)
    // struct dentry *dmap;
    // struct dentry *rmap;
};

#include "db_fs/db_fs.h" // ::: ???


extern int taged_file(struct dentry *d_file, char *name);
extern int untaged_file(struct dentry *d_file, char *name);

extern struct tag *lookup_tag(char *name);
extern struct datafile *lookup_datafile(struct tag *tag, char *name);
extern struct branch *lookup_branch(struct tag *tag, unsigned long nr);

extern struct branch *alloc_branch(void);
extern struct tag *alloc_tag(void); // :::
extern struct datafile *alloc_datafile(void);

extern void put_branch(struct branch *branch);
extern void put_tag(struct tag *tag);
extern void put_datafile(struct datafile *datafile);

extern int fill_tag(struct tag *tag, char *name, struct dentry *dir);
extern int fill_branch(struct branch *branch, char *name, struct dentry *dir);
extern int fill_datafile(struct datafile *datafile, char *name, struct dentry *dir);


#endif /* DATABASE_H_ */
