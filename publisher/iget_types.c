#include "publisher.h"
#include "../hooks/hooks.h"

#include <linux/magic.h>

typedef enum {
		EXT4_IGET_NORMAL =	0,
		EXT4_IGET_SPECIAL =	0x0001, /* OK to iget a system inode */ // ???
		EXT4_IGET_HANDLE = 	0x0002	/* Inode # is from a handle */ // ???
	} ext4_iget_flags;

struct inode *(*__ext4_iget)(struct super_block *sb, unsigned long ino,
				ext4_iget_flags flags, const char *function,
				unsigned int line) = NULL;

unsigned long (*my_kallsyms_lookup_name)(const char *) = NULL;


struct inode *ext4_iget_wrapper(struct super_block *sb, unsigned long ino){
	
	if(!my_kallsyms_lookup_name)
		my_kallsyms_lookup_name = lookup_name("kallsyms_lookup_name");

	if(!my_kallsyms_lookup_name)
		return NULL;

	if(!__ext4_iget)
		__ext4_iget = my_kallsyms_lookup_name("__ext4_iget");

	if(!__ext4_iget)
		return NULL;

	return __ext4_iget(sb, ino, EXT4_IGET_NORMAL,  __func__, __LINE__);
}

struct inode *tmpfs_iget_wrapper(struct super_block *sb, unsigned long ino){
	return iget_locked(sb, ino);
}

struct inode *iget_generic(struct super_block *sb, unsigned long ino){
	struct inode *inode;
	
	switch (sb->s_magic) {
		case EXT4_SUPER_MAGIC:
			pr_info("sb is ext4\n");
			inode = ext4_iget_wrapper(sb, ino);
			break;
		
		// case TMPFS_MAGIC | RAMFS_MAGIC:
		// 	pr_info("sb is tmpfs\n");
		// 	inode = tmpfs_iget_wrapper(sb, ino);
		// 	break;
	}

	return inode;

}