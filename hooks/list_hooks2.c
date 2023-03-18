asmlinkage long (*real_sys_write)(struct pt_regs *regs);
asmlinkage long (*real_sys_pwrite64)(struct pt_regs *regs);
asmlinkage long (*real_sys_inotify_add_watch)(struct pt_regs *regs);
asmlinkage long (*real_sys_name_to_handle_at)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimensat)(struct pt_regs *regs);
asmlinkage long (*real_sys_futimesat)(struct pt_regs *regs);
asmlinkage long (*real_sys_utime32)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimensat_time32)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimes_time32)(struct pt_regs *regs);
asmlinkage long (*real_sys_uselib)(struct pt_regs *regs);
asmlinkage long (*real_sys_execve)(struct pt_regs *regs);
asmlinkage long (*real_sys_open_tree)(struct pt_regs *regs);
asmlinkage long (*real_sys_pivot_root)(struct pt_regs *regs);
asmlinkage long (*real_sys_mount_setattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_setxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_lsetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fsetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_getxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_lgetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fgetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_listxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_llistxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_removexattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_lremovexattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fremovexattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_mknodat)(struct pt_regs *regs);
asmlinkage long (*real_sys_mknod)(struct pt_regs *regs);
asmlinkage long (*real_sys_mkdirat)(struct pt_regs *regs);
asmlinkage long (*real_sys_mkdir)(struct pt_regs *regs);
asmlinkage long (*real_sys_rmdir)(struct pt_regs *regs);
asmlinkage long (*real_sys_unlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_unlink)(struct pt_regs *regs);
asmlinkage long (*real_sys_symlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_symlink)(struct pt_regs *regs);
asmlinkage long (*real_sys_linkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_link)(struct pt_regs *regs);
asmlinkage long (*real_sys_renameat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_renameat)(struct pt_regs *regs);
asmlinkage long (*real_sys_rename)(struct pt_regs *regs);
asmlinkage long (*real_sys_statfs)(struct pt_regs *regs);
asmlinkage long (*real_sys_statfs64)(struct pt_regs *regs);
asmlinkage long (*real_sys_statfs)(struct pt_regs *regs);
asmlinkage long (*real_sys_statfs64)(struct pt_regs *regs);
asmlinkage long (*real_sys_stat)(struct pt_regs *regs);
asmlinkage long (*real_sys_lstat)(struct pt_regs *regs);
asmlinkage long (*real_sys_newstat)(struct pt_regs *regs);
asmlinkage long (*real_sys_newlstat)(struct pt_regs *regs);
asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs);
asmlinkage long (*real_sys_readlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_readlink)(struct pt_regs *regs);
asmlinkage long (*real_sys_stat64)(struct pt_regs *regs);
asmlinkage long (*real_sys_lstat64)(struct pt_regs *regs);
asmlinkage long (*real_sys_fstatat64)(struct pt_regs *regs);
asmlinkage long (*real_sys_newstat)(struct pt_regs *regs);
asmlinkage long (*real_sys_newlstat)(struct pt_regs *regs);
asmlinkage long (*real_sys_fsopen)(struct pt_regs *regs);
asmlinkage long (*real_sys_fspick)(struct pt_regs *regs);
asmlinkage long (*real_sys_truncate)(struct pt_regs *regs);
asmlinkage long (*real_sys_truncate)(struct pt_regs *regs);
asmlinkage long (*real_sys_truncate64)(struct pt_regs *regs);
asmlinkage long (*real_sys_faccessat)(struct pt_regs *regs);
asmlinkage long (*real_sys_faccessat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_access)(struct pt_regs *regs);
asmlinkage long (*real_sys_chdir)(struct pt_regs *regs);
asmlinkage long (*real_sys_chroot)(struct pt_regs *regs);
asmlinkage long (*real_sys_fchmodat)(struct pt_regs *regs);
asmlinkage long (*real_sys_chmod)(struct pt_regs *regs);
asmlinkage long (*real_sys_fchownat)(struct pt_regs *regs);
asmlinkage long (*real_sys_chown)(struct pt_regs *regs);
asmlinkage long (*real_sys_lchown)(struct pt_regs *regs);
asmlinkage long (*real_sys_open)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_open)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long (*real_sys_creat)(struct pt_regs *regs);
asmlinkage long (*real_sys_quotactl)(struct pt_regs *regs);
asmlinkage long (*real_sys_write)(struct pt_regs *regs);
asmlinkage long (*real_sys_pwrite64)(struct pt_regs *regs);
asmlinkage long (*real_sys_inotify_add_watch)(struct pt_regs *regs);
asmlinkage long (*real_sys_name_to_handle_at)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimensat)(struct pt_regs *regs);
asmlinkage long (*real_sys_futimesat)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimensat_time32)(struct pt_regs *regs);
asmlinkage long (*real_sys_open_tree)(struct pt_regs *regs);
asmlinkage long (*real_sys_mount_setattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fsetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fgetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fremovexattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_mknodat)(struct pt_regs *regs);
asmlinkage long (*real_sys_mkdirat)(struct pt_regs *regs);
asmlinkage long (*real_sys_unlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_symlink)(struct pt_regs *regs);
asmlinkage long (*real_sys_linkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_link)(struct pt_regs *regs);
asmlinkage long (*real_sys_renameat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_renameat)(struct pt_regs *regs);
asmlinkage long (*real_sys_rename)(struct pt_regs *regs);
asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs);
asmlinkage long (*real_sys_readlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_fstatat64)(struct pt_regs *regs);
asmlinkage long (*real_sys_fspick)(struct pt_regs *regs);
asmlinkage long (*real_sys_faccessat)(struct pt_regs *regs);
asmlinkage long (*real_sys_faccessat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_fchmodat)(struct pt_regs *regs);
asmlinkage long (*real_sys_fchownat)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long (*real_sys_quotactl)(struct pt_regs *regs);
asmlinkage long (*real_sys_write)(struct pt_regs *regs);
asmlinkage long (*real_sys_pwrite64)(struct pt_regs *regs);
asmlinkage long (*real_sys_inotify_add_watch)(struct pt_regs *regs);
asmlinkage long (*real_sys_name_to_handle_at)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimensat)(struct pt_regs *regs);
asmlinkage long (*real_sys_futimesat)(struct pt_regs *regs);
asmlinkage long (*real_sys_utimensat_time32)(struct pt_regs *regs);
asmlinkage long (*real_sys_open_tree)(struct pt_regs *regs);
asmlinkage long (*real_sys_mount_setattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fsetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fgetxattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_fremovexattr)(struct pt_regs *regs);
asmlinkage long (*real_sys_mknodat)(struct pt_regs *regs);
asmlinkage long (*real_sys_mkdirat)(struct pt_regs *regs);
asmlinkage long (*real_sys_unlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_symlink)(struct pt_regs *regs);
asmlinkage long (*real_sys_linkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_link)(struct pt_regs *regs);
asmlinkage long (*real_sys_renameat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_renameat)(struct pt_regs *regs);
asmlinkage long (*real_sys_rename)(struct pt_regs *regs);
asmlinkage long (*real_sys_newfstatat)(struct pt_regs *regs);
asmlinkage long (*real_sys_readlinkat)(struct pt_regs *regs);
asmlinkage long (*real_sys_fstatat64)(struct pt_regs *regs);
asmlinkage long (*real_sys_fspick)(struct pt_regs *regs);
asmlinkage long (*real_sys_faccessat)(struct pt_regs *regs);
asmlinkage long (*real_sys_faccessat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_fchmodat)(struct pt_regs *regs);
asmlinkage long (*real_sys_fchownat)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat2)(struct pt_regs *regs);
asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
asmlinkage long (*real_sys_quotactl)(struct pt_regs *regs);

asmlinkage long fh_sys_write(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_write, regs->di);
}
asmlinkage long fh_sys_pwrite64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_pwrite64, regs->di);
}
asmlinkage long fh_sys_inotify_add_watch(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_inotify_add_watch, regs->di);
}
asmlinkage long fh_sys_name_to_handle_at(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_name_to_handle_at, regs->di);
}
asmlinkage long fh_sys_utimensat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimensat, regs->di);
}
asmlinkage long fh_sys_futimesat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_futimesat, regs->di);
}
asmlinkage long fh_sys_utime32(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utime32, regs->di);
}
asmlinkage long fh_sys_utimensat_time32(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimensat_time32, regs->di);
}
asmlinkage long fh_sys_utimes_time32(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimes_time32, regs->di);
}
asmlinkage long fh_sys_uselib(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_uselib, regs->di);
}
asmlinkage long fh_sys_execve(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_execve, regs->di);
}
asmlinkage long fh_sys_open_tree(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_open_tree, regs->di);
}
asmlinkage long fh_sys_pivot_root(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_pivot_root, regs->di);
}
asmlinkage long fh_sys_mount_setattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mount_setattr, regs->di);
}
asmlinkage long fh_sys_setxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_setxattr, regs->di);
}
asmlinkage long fh_sys_lsetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lsetxattr, regs->di);
}
asmlinkage long fh_sys_fsetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fsetxattr, regs->di);
}
asmlinkage long fh_sys_getxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_getxattr, regs->di);
}
asmlinkage long fh_sys_lgetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lgetxattr, regs->di);
}
asmlinkage long fh_sys_fgetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fgetxattr, regs->di);
}
asmlinkage long fh_sys_listxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_listxattr, regs->di);
}
asmlinkage long fh_sys_llistxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_llistxattr, regs->di);
}
asmlinkage long fh_sys_removexattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_removexattr, regs->di);
}
asmlinkage long fh_sys_lremovexattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lremovexattr, regs->di);
}
asmlinkage long fh_sys_fremovexattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fremovexattr, regs->di);
}
asmlinkage long fh_sys_mknodat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mknodat, regs->di);
}
asmlinkage long fh_sys_mknod(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mknod, regs->di);
}
asmlinkage long fh_sys_mkdirat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mkdirat, regs->di);
}
asmlinkage long fh_sys_mkdir(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mkdir, regs->di);
}
asmlinkage long fh_sys_rmdir(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_rmdir, regs->di);
}
asmlinkage long fh_sys_unlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_unlinkat, regs->di);
}
asmlinkage long fh_sys_unlink(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_unlink, regs->di);
}
asmlinkage long fh_sys_symlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_symlinkat, regs->di);
}
asmlinkage long fh_sys_symlink(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_symlink, regs->di);
}
asmlinkage long fh_sys_linkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_linkat, regs->di);
}
asmlinkage long fh_sys_link(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_link, regs->di);
}
asmlinkage long fh_sys_renameat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_renameat2, regs->di);
}
asmlinkage long fh_sys_renameat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_renameat, regs->di);
}
asmlinkage long fh_sys_rename(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_rename, regs->di);
}
asmlinkage long fh_sys_statfs(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_statfs, regs->di);
}
asmlinkage long fh_sys_statfs64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_statfs64, regs->di);
}
asmlinkage long fh_sys_statfs(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_statfs, regs->di);
}
asmlinkage long fh_sys_statfs64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_statfs64, regs->di);
}
asmlinkage long fh_sys_stat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_stat, regs->di);
}
asmlinkage long fh_sys_lstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lstat, regs->di);
}
asmlinkage long fh_sys_newstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newstat, regs->di);
}
asmlinkage long fh_sys_newlstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newlstat, regs->di);
}
asmlinkage long fh_sys_newfstatat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newfstatat, regs->di);
}
asmlinkage long fh_sys_readlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_readlinkat, regs->di);
}
asmlinkage long fh_sys_readlink(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_readlink, regs->di);
}
asmlinkage long fh_sys_stat64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_stat64, regs->di);
}
asmlinkage long fh_sys_lstat64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lstat64, regs->di);
}
asmlinkage long fh_sys_fstatat64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fstatat64, regs->di);
}
asmlinkage long fh_sys_newstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newstat, regs->di);
}
asmlinkage long fh_sys_newlstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newlstat, regs->di);
}
asmlinkage long fh_sys_fsopen(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fsopen, regs->di);
}
asmlinkage long fh_sys_fspick(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fspick, regs->di);
}
asmlinkage long fh_sys_truncate(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_truncate, regs->di);
}
asmlinkage long fh_sys_truncate(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_truncate, regs->di);
}
asmlinkage long fh_sys_truncate64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_truncate64, regs->di);
}
asmlinkage long fh_sys_faccessat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_faccessat, regs->di);
}
asmlinkage long fh_sys_faccessat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_faccessat2, regs->di);
}
asmlinkage long fh_sys_access(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_access, regs->di);
}
asmlinkage long fh_sys_chdir(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_chdir, regs->di);
}
asmlinkage long fh_sys_chroot(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_chroot, regs->di);
}
asmlinkage long fh_sys_fchmodat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fchmodat, regs->di);
}
asmlinkage long fh_sys_chmod(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_chmod, regs->di);
}
asmlinkage long fh_sys_fchownat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fchownat, regs->di);
}
asmlinkage long fh_sys_chown(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_chown, regs->di);
}
asmlinkage long fh_sys_lchown(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lchown, regs->di);
}
asmlinkage long fh_sys_open(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_open, regs->di);
}
asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->di);
}
asmlinkage long fh_sys_openat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat2, regs->di);
}
asmlinkage long fh_sys_open(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_open, regs->di);
}
asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->di);
}
asmlinkage long fh_sys_creat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_creat, regs->di);
}
asmlinkage long fh_sys_quotactl(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_quotactl, regs->di);
}
asmlinkage long fh_sys_write(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_write, regs->si);
}
asmlinkage long fh_sys_pwrite64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_pwrite64, regs->si);
}
asmlinkage long fh_sys_inotify_add_watch(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_inotify_add_watch, regs->si);
}
asmlinkage long fh_sys_name_to_handle_at(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_name_to_handle_at, regs->si);
}
asmlinkage long fh_sys_utimensat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimensat, regs->si);
}
asmlinkage long fh_sys_futimesat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_futimesat, regs->si);
}
asmlinkage long fh_sys_utimensat_time32(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimensat_time32, regs->si);
}
asmlinkage long fh_sys_open_tree(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_open_tree, regs->si);
}
asmlinkage long fh_sys_mount_setattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mount_setattr, regs->si);
}
asmlinkage long fh_sys_fsetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fsetxattr, regs->si);
}
asmlinkage long fh_sys_fgetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fgetxattr, regs->si);
}
asmlinkage long fh_sys_fremovexattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fremovexattr, regs->si);
}
asmlinkage long fh_sys_mknodat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mknodat, regs->si);
}
asmlinkage long fh_sys_mkdirat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mkdirat, regs->si);
}
asmlinkage long fh_sys_unlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_unlinkat, regs->si);
}
asmlinkage long fh_sys_symlink(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_symlink, regs->si);
}
asmlinkage long fh_sys_linkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_linkat, regs->si);
}
asmlinkage long fh_sys_link(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_link, regs->si);
}
asmlinkage long fh_sys_renameat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_renameat2, regs->si);
}
asmlinkage long fh_sys_renameat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_renameat, regs->si);
}
asmlinkage long fh_sys_rename(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_rename, regs->si);
}
asmlinkage long fh_sys_newfstatat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newfstatat, regs->si);
}
asmlinkage long fh_sys_readlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_readlinkat, regs->si);
}
asmlinkage long fh_sys_fstatat64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fstatat64, regs->si);
}
asmlinkage long fh_sys_fspick(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fspick, regs->si);
}
asmlinkage long fh_sys_faccessat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_faccessat, regs->si);
}
asmlinkage long fh_sys_faccessat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_faccessat2, regs->si);
}
asmlinkage long fh_sys_fchmodat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fchmodat, regs->si);
}
asmlinkage long fh_sys_fchownat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fchownat, regs->si);
}
asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->si);
}
asmlinkage long fh_sys_openat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat2, regs->si);
}
asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->si);
}
asmlinkage long fh_sys_quotactl(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_quotactl, regs->si);
}
asmlinkage long fh_sys_write(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_write, regs->dx);
}
asmlinkage long fh_sys_pwrite64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_pwrite64, regs->dx);
}
asmlinkage long fh_sys_inotify_add_watch(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_inotify_add_watch, regs->dx);
}
asmlinkage long fh_sys_name_to_handle_at(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_name_to_handle_at, regs->dx);
}
asmlinkage long fh_sys_utimensat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimensat, regs->dx);
}
asmlinkage long fh_sys_futimesat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_futimesat, regs->dx);
}
asmlinkage long fh_sys_utimensat_time32(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_utimensat_time32, regs->dx);
}
asmlinkage long fh_sys_open_tree(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_open_tree, regs->dx);
}
asmlinkage long fh_sys_mount_setattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mount_setattr, regs->dx);
}
asmlinkage long fh_sys_fsetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fsetxattr, regs->dx);
}
asmlinkage long fh_sys_fgetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fgetxattr, regs->dx);
}
asmlinkage long fh_sys_fremovexattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fremovexattr, regs->dx);
}
asmlinkage long fh_sys_mknodat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mknodat, regs->dx);
}
asmlinkage long fh_sys_mkdirat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_mkdirat, regs->dx);
}
asmlinkage long fh_sys_unlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_unlinkat, regs->dx);
}
asmlinkage long fh_sys_symlink(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_symlink, regs->dx);
}
asmlinkage long fh_sys_linkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_linkat, regs->dx);
}
asmlinkage long fh_sys_link(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_link, regs->dx);
}
asmlinkage long fh_sys_renameat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_renameat2, regs->dx);
}
asmlinkage long fh_sys_renameat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_renameat, regs->dx);
}
asmlinkage long fh_sys_rename(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_rename, regs->dx);
}
asmlinkage long fh_sys_newfstatat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_newfstatat, regs->dx);
}
asmlinkage long fh_sys_readlinkat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_readlinkat, regs->dx);
}
asmlinkage long fh_sys_fstatat64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fstatat64, regs->dx);
}
asmlinkage long fh_sys_fspick(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fspick, regs->dx);
}
asmlinkage long fh_sys_faccessat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_faccessat, regs->dx);
}
asmlinkage long fh_sys_faccessat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_faccessat2, regs->dx);
}
asmlinkage long fh_sys_fchmodat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fchmodat, regs->dx);
}
asmlinkage long fh_sys_fchownat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_fchownat, regs->dx);
}
asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->dx);
}
asmlinkage long fh_sys_openat2(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat2, regs->dx);
}
asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->dx);
}
asmlinkage long fh_sys_quotactl(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_quotactl, regs->dx);
}

struct ftrace_hook generic_hooks[] = {
	HOOK(SYSCALL_NAME("sys_write"), fh_sys_write, &real_sys_write),
	HOOK(SYSCALL_NAME("sys_pwrite64"), fh_sys_pwrite64, &real_sys_pwrite64),
	HOOK(SYSCALL_NAME("sys_inotify_add_watch"), fh_sys_inotify_add_watch, &real_sys_inotify_add_watch),
	HOOK(SYSCALL_NAME("sys_name_to_handle_at"), fh_sys_name_to_handle_at, &real_sys_name_to_handle_at),
	HOOK(SYSCALL_NAME("sys_utimensat"), fh_sys_utimensat, &real_sys_utimensat),
	HOOK(SYSCALL_NAME("sys_futimesat"), fh_sys_futimesat, &real_sys_futimesat),
	HOOK(SYSCALL_NAME("sys_utime32"), fh_sys_utime32, &real_sys_utime32),
	HOOK(SYSCALL_NAME("sys_utimensat_time32"), fh_sys_utimensat_time32, &real_sys_utimensat_time32),
	HOOK(SYSCALL_NAME("sys_utimes_time32"), fh_sys_utimes_time32, &real_sys_utimes_time32),
	HOOK(SYSCALL_NAME("sys_uselib"), fh_sys_uselib, &real_sys_uselib),
	HOOK(SYSCALL_NAME("sys_execve"), fh_sys_execve, &real_sys_execve),
	HOOK(SYSCALL_NAME("sys_open_tree"), fh_sys_open_tree, &real_sys_open_tree),
	HOOK(SYSCALL_NAME("sys_pivot_root"), fh_sys_pivot_root, &real_sys_pivot_root),
	HOOK(SYSCALL_NAME("sys_mount_setattr"), fh_sys_mount_setattr, &real_sys_mount_setattr),
	HOOK(SYSCALL_NAME("sys_setxattr"), fh_sys_setxattr, &real_sys_setxattr),
	HOOK(SYSCALL_NAME("sys_lsetxattr"), fh_sys_lsetxattr, &real_sys_lsetxattr),
	HOOK(SYSCALL_NAME("sys_fsetxattr"), fh_sys_fsetxattr, &real_sys_fsetxattr),
	HOOK(SYSCALL_NAME("sys_getxattr"), fh_sys_getxattr, &real_sys_getxattr),
	HOOK(SYSCALL_NAME("sys_lgetxattr"), fh_sys_lgetxattr, &real_sys_lgetxattr),
	HOOK(SYSCALL_NAME("sys_fgetxattr"), fh_sys_fgetxattr, &real_sys_fgetxattr),
	HOOK(SYSCALL_NAME("sys_listxattr"), fh_sys_listxattr, &real_sys_listxattr),
	HOOK(SYSCALL_NAME("sys_llistxattr"), fh_sys_llistxattr, &real_sys_llistxattr),
	HOOK(SYSCALL_NAME("sys_removexattr"), fh_sys_removexattr, &real_sys_removexattr),
	HOOK(SYSCALL_NAME("sys_lremovexattr"), fh_sys_lremovexattr, &real_sys_lremovexattr),
	HOOK(SYSCALL_NAME("sys_fremovexattr"), fh_sys_fremovexattr, &real_sys_fremovexattr),
	HOOK(SYSCALL_NAME("sys_mknodat"), fh_sys_mknodat, &real_sys_mknodat),
	HOOK(SYSCALL_NAME("sys_mknod"), fh_sys_mknod, &real_sys_mknod),
	HOOK(SYSCALL_NAME("sys_mkdirat"), fh_sys_mkdirat, &real_sys_mkdirat),
	HOOK(SYSCALL_NAME("sys_mkdir"), fh_sys_mkdir, &real_sys_mkdir),
	HOOK(SYSCALL_NAME("sys_rmdir"), fh_sys_rmdir, &real_sys_rmdir),
	HOOK(SYSCALL_NAME("sys_unlinkat"), fh_sys_unlinkat, &real_sys_unlinkat),
	HOOK(SYSCALL_NAME("sys_unlink"), fh_sys_unlink, &real_sys_unlink),
	HOOK(SYSCALL_NAME("sys_symlinkat"), fh_sys_symlinkat, &real_sys_symlinkat),
	HOOK(SYSCALL_NAME("sys_symlink"), fh_sys_symlink, &real_sys_symlink),
	HOOK(SYSCALL_NAME("sys_linkat"), fh_sys_linkat, &real_sys_linkat),
	HOOK(SYSCALL_NAME("sys_link"), fh_sys_link, &real_sys_link),
	HOOK(SYSCALL_NAME("sys_renameat2"), fh_sys_renameat2, &real_sys_renameat2),
	HOOK(SYSCALL_NAME("sys_renameat"), fh_sys_renameat, &real_sys_renameat),
	HOOK(SYSCALL_NAME("sys_rename"), fh_sys_rename, &real_sys_rename),
	HOOK(SYSCALL_NAME("sys_statfs"), fh_sys_statfs, &real_sys_statfs),
	HOOK(SYSCALL_NAME("sys_statfs64"), fh_sys_statfs64, &real_sys_statfs64),
	HOOK(SYSCALL_NAME("sys_statfs"), fh_sys_statfs, &real_sys_statfs),
	HOOK(SYSCALL_NAME("sys_statfs64"), fh_sys_statfs64, &real_sys_statfs64),
	HOOK(SYSCALL_NAME("sys_stat"), fh_sys_stat, &real_sys_stat),
	HOOK(SYSCALL_NAME("sys_lstat"), fh_sys_lstat, &real_sys_lstat),
	HOOK(SYSCALL_NAME("sys_newstat"), fh_sys_newstat, &real_sys_newstat),
	HOOK(SYSCALL_NAME("sys_newlstat"), fh_sys_newlstat, &real_sys_newlstat),
	HOOK(SYSCALL_NAME("sys_newfstatat"), fh_sys_newfstatat, &real_sys_newfstatat),
	HOOK(SYSCALL_NAME("sys_readlinkat"), fh_sys_readlinkat, &real_sys_readlinkat),
	HOOK(SYSCALL_NAME("sys_readlink"), fh_sys_readlink, &real_sys_readlink),
	HOOK(SYSCALL_NAME("sys_stat64"), fh_sys_stat64, &real_sys_stat64),
	HOOK(SYSCALL_NAME("sys_lstat64"), fh_sys_lstat64, &real_sys_lstat64),
	HOOK(SYSCALL_NAME("sys_fstatat64"), fh_sys_fstatat64, &real_sys_fstatat64),
	HOOK(SYSCALL_NAME("sys_newstat"), fh_sys_newstat, &real_sys_newstat),
	HOOK(SYSCALL_NAME("sys_newlstat"), fh_sys_newlstat, &real_sys_newlstat),
	HOOK(SYSCALL_NAME("sys_fsopen"), fh_sys_fsopen, &real_sys_fsopen),
	HOOK(SYSCALL_NAME("sys_fspick"), fh_sys_fspick, &real_sys_fspick),
	HOOK(SYSCALL_NAME("sys_truncate"), fh_sys_truncate, &real_sys_truncate),
	HOOK(SYSCALL_NAME("sys_truncate"), fh_sys_truncate, &real_sys_truncate),
	HOOK(SYSCALL_NAME("sys_truncate64"), fh_sys_truncate64, &real_sys_truncate64),
	HOOK(SYSCALL_NAME("sys_faccessat"), fh_sys_faccessat, &real_sys_faccessat),
	HOOK(SYSCALL_NAME("sys_faccessat2"), fh_sys_faccessat2, &real_sys_faccessat2),
	HOOK(SYSCALL_NAME("sys_access"), fh_sys_access, &real_sys_access),
	HOOK(SYSCALL_NAME("sys_chdir"), fh_sys_chdir, &real_sys_chdir),
	HOOK(SYSCALL_NAME("sys_chroot"), fh_sys_chroot, &real_sys_chroot),
	HOOK(SYSCALL_NAME("sys_fchmodat"), fh_sys_fchmodat, &real_sys_fchmodat),
	HOOK(SYSCALL_NAME("sys_chmod"), fh_sys_chmod, &real_sys_chmod),
	HOOK(SYSCALL_NAME("sys_fchownat"), fh_sys_fchownat, &real_sys_fchownat),
	HOOK(SYSCALL_NAME("sys_chown"), fh_sys_chown, &real_sys_chown),
	HOOK(SYSCALL_NAME("sys_lchown"), fh_sys_lchown, &real_sys_lchown),
	HOOK(SYSCALL_NAME("sys_open"), fh_sys_open, &real_sys_open),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_openat2"), fh_sys_openat2, &real_sys_openat2),
	HOOK(SYSCALL_NAME("sys_open"), fh_sys_open, &real_sys_open),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_creat"), fh_sys_creat, &real_sys_creat),
	HOOK(SYSCALL_NAME("sys_quotactl"), fh_sys_quotactl, &real_sys_quotactl),
	HOOK(SYSCALL_NAME("sys_write"), fh_sys_write, &real_sys_write),
	HOOK(SYSCALL_NAME("sys_pwrite64"), fh_sys_pwrite64, &real_sys_pwrite64),
	HOOK(SYSCALL_NAME("sys_inotify_add_watch"), fh_sys_inotify_add_watch, &real_sys_inotify_add_watch),
	HOOK(SYSCALL_NAME("sys_name_to_handle_at"), fh_sys_name_to_handle_at, &real_sys_name_to_handle_at),
	HOOK(SYSCALL_NAME("sys_utimensat"), fh_sys_utimensat, &real_sys_utimensat),
	HOOK(SYSCALL_NAME("sys_futimesat"), fh_sys_futimesat, &real_sys_futimesat),
	HOOK(SYSCALL_NAME("sys_utimensat_time32"), fh_sys_utimensat_time32, &real_sys_utimensat_time32),
	HOOK(SYSCALL_NAME("sys_open_tree"), fh_sys_open_tree, &real_sys_open_tree),
	HOOK(SYSCALL_NAME("sys_mount_setattr"), fh_sys_mount_setattr, &real_sys_mount_setattr),
	HOOK(SYSCALL_NAME("sys_fsetxattr"), fh_sys_fsetxattr, &real_sys_fsetxattr),
	HOOK(SYSCALL_NAME("sys_fgetxattr"), fh_sys_fgetxattr, &real_sys_fgetxattr),
	HOOK(SYSCALL_NAME("sys_fremovexattr"), fh_sys_fremovexattr, &real_sys_fremovexattr),
	HOOK(SYSCALL_NAME("sys_mknodat"), fh_sys_mknodat, &real_sys_mknodat),
	HOOK(SYSCALL_NAME("sys_mkdirat"), fh_sys_mkdirat, &real_sys_mkdirat),
	HOOK(SYSCALL_NAME("sys_unlinkat"), fh_sys_unlinkat, &real_sys_unlinkat),
	HOOK(SYSCALL_NAME("sys_symlink"), fh_sys_symlink, &real_sys_symlink),
	HOOK(SYSCALL_NAME("sys_linkat"), fh_sys_linkat, &real_sys_linkat),
	HOOK(SYSCALL_NAME("sys_link"), fh_sys_link, &real_sys_link),
	HOOK(SYSCALL_NAME("sys_renameat2"), fh_sys_renameat2, &real_sys_renameat2),
	HOOK(SYSCALL_NAME("sys_renameat"), fh_sys_renameat, &real_sys_renameat),
	HOOK(SYSCALL_NAME("sys_rename"), fh_sys_rename, &real_sys_rename),
	HOOK(SYSCALL_NAME("sys_newfstatat"), fh_sys_newfstatat, &real_sys_newfstatat),
	HOOK(SYSCALL_NAME("sys_readlinkat"), fh_sys_readlinkat, &real_sys_readlinkat),
	HOOK(SYSCALL_NAME("sys_fstatat64"), fh_sys_fstatat64, &real_sys_fstatat64),
	HOOK(SYSCALL_NAME("sys_fspick"), fh_sys_fspick, &real_sys_fspick),
	HOOK(SYSCALL_NAME("sys_faccessat"), fh_sys_faccessat, &real_sys_faccessat),
	HOOK(SYSCALL_NAME("sys_faccessat2"), fh_sys_faccessat2, &real_sys_faccessat2),
	HOOK(SYSCALL_NAME("sys_fchmodat"), fh_sys_fchmodat, &real_sys_fchmodat),
	HOOK(SYSCALL_NAME("sys_fchownat"), fh_sys_fchownat, &real_sys_fchownat),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_openat2"), fh_sys_openat2, &real_sys_openat2),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_quotactl"), fh_sys_quotactl, &real_sys_quotactl),
	HOOK(SYSCALL_NAME("sys_write"), fh_sys_write, &real_sys_write),
	HOOK(SYSCALL_NAME("sys_pwrite64"), fh_sys_pwrite64, &real_sys_pwrite64),
	HOOK(SYSCALL_NAME("sys_inotify_add_watch"), fh_sys_inotify_add_watch, &real_sys_inotify_add_watch),
	HOOK(SYSCALL_NAME("sys_name_to_handle_at"), fh_sys_name_to_handle_at, &real_sys_name_to_handle_at),
	HOOK(SYSCALL_NAME("sys_utimensat"), fh_sys_utimensat, &real_sys_utimensat),
	HOOK(SYSCALL_NAME("sys_futimesat"), fh_sys_futimesat, &real_sys_futimesat),
	HOOK(SYSCALL_NAME("sys_utimensat_time32"), fh_sys_utimensat_time32, &real_sys_utimensat_time32),
	HOOK(SYSCALL_NAME("sys_open_tree"), fh_sys_open_tree, &real_sys_open_tree),
	HOOK(SYSCALL_NAME("sys_mount_setattr"), fh_sys_mount_setattr, &real_sys_mount_setattr),
	HOOK(SYSCALL_NAME("sys_fsetxattr"), fh_sys_fsetxattr, &real_sys_fsetxattr),
	HOOK(SYSCALL_NAME("sys_fgetxattr"), fh_sys_fgetxattr, &real_sys_fgetxattr),
	HOOK(SYSCALL_NAME("sys_fremovexattr"), fh_sys_fremovexattr, &real_sys_fremovexattr),
	HOOK(SYSCALL_NAME("sys_mknodat"), fh_sys_mknodat, &real_sys_mknodat),
	HOOK(SYSCALL_NAME("sys_mkdirat"), fh_sys_mkdirat, &real_sys_mkdirat),
	HOOK(SYSCALL_NAME("sys_unlinkat"), fh_sys_unlinkat, &real_sys_unlinkat),
	HOOK(SYSCALL_NAME("sys_symlink"), fh_sys_symlink, &real_sys_symlink),
	HOOK(SYSCALL_NAME("sys_linkat"), fh_sys_linkat, &real_sys_linkat),
	HOOK(SYSCALL_NAME("sys_link"), fh_sys_link, &real_sys_link),
	HOOK(SYSCALL_NAME("sys_renameat2"), fh_sys_renameat2, &real_sys_renameat2),
	HOOK(SYSCALL_NAME("sys_renameat"), fh_sys_renameat, &real_sys_renameat),
	HOOK(SYSCALL_NAME("sys_rename"), fh_sys_rename, &real_sys_rename),
	HOOK(SYSCALL_NAME("sys_newfstatat"), fh_sys_newfstatat, &real_sys_newfstatat),
	HOOK(SYSCALL_NAME("sys_readlinkat"), fh_sys_readlinkat, &real_sys_readlinkat),
	HOOK(SYSCALL_NAME("sys_fstatat64"), fh_sys_fstatat64, &real_sys_fstatat64),
	HOOK(SYSCALL_NAME("sys_fspick"), fh_sys_fspick, &real_sys_fspick),
	HOOK(SYSCALL_NAME("sys_faccessat"), fh_sys_faccessat, &real_sys_faccessat),
	HOOK(SYSCALL_NAME("sys_faccessat2"), fh_sys_faccessat2, &real_sys_faccessat2),
	HOOK(SYSCALL_NAME("sys_fchmodat"), fh_sys_fchmodat, &real_sys_fchmodat),
	HOOK(SYSCALL_NAME("sys_fchownat"), fh_sys_fchownat, &real_sys_fchownat),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_openat2"), fh_sys_openat2, &real_sys_openat2),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_quotactl"), fh_sys_quotactl, &real_sys_quotactl),
};