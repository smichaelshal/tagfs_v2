#ifndef DB_FS_H_
#define DB_FS_H_

#include "../database.h"

#define DEFAULT_MODE_FILES 0755
#define INO_NAME "security.ino"
#define INO_PARENT_NAME "security.ino_parent"
#define NEXT_NAME "security.next"
#define NEXT_INIT_VALUE 0

extern struct dentry *db_lookup_dentry(struct dentry *parent, char *name);
extern struct dentry *db_mkdir(struct dentry *parent, char *name);
// extern int db_create_datafiles(struct datafile *df, struct tag *tag);
extern struct dentry *db_create_file_dmap(struct datafile *df, struct dentry *d_dmap);
extern struct dentry *db_create_file_rmap(struct datafile *df, struct dentry *d_rmap, struct dentry *dmap_file);
extern int db_remove_file_dmap(struct datafile *df, struct dentry *d_dmap);
extern int init_rmap(struct dentry *dir);
extern int db_read_datafile(struct dentry *dentry, struct datafile *df);

#endif /* DB_FS_H_ */
