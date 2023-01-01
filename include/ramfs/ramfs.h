#ifndef RAMFS_H_
#define RAMFS_H_


// internal.h
extern const struct inode_operations ramfs_file_inode_operations;


// libfs.c
extern const struct address_space_operations ram_aops;

int simple_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata);

int simple_readpage(struct file *file, struct page *page);


// file-mmu.c

extern const struct file_operations ramfs_file_operations;
extern const struct inode_operations ramfs_file_inode_operations;
unsigned long ramfs_mmu_get_unmapped_area(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags);


#endif /* RAMFS_H_ */
