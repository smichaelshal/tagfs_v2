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

unsigned long (*kallsyms_lookup_name)(const char *) = NULL;


struct inode *ext4_iget_wrapper(struct super_block *sb, unsigned long ino){
	
	if(!kallsyms_lookup_name)
		kallsyms_lookup_name = lookup_name("kallsyms_lookup_name");

	if(!kallsyms_lookup_name)
		return NULL;

	if(!__ext4_iget)
		__ext4_iget = kallsyms_lookup_name("__ext4_iget");

	if(!__ext4_iget)
		return NULL;

	return __ext4_iget(sb, ino, EXT4_IGET_NORMAL,  __func__, __LINE__);
}

struct inode *iget_generic(struct super_block *sb, unsigned long ino){
	struct inode *inode;
	
	switch (sb->s_magic) {
		case EXT4_SUPER_MAGIC:
			inode = ext4_iget_wrapper(sb, ino);
			break;
	}

	return inode;
	// if(sb->s_magic == EXT4_SUPER_MAGIC)

}