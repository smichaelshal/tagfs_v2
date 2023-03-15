#include <linux/slab.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <linux/file.h>
#include <linux/fs_struct.h>

#include "../vtagfs.h"
#include "hooks.h"
#include "../utils/utils.h"


// struct proc_dir_entry *

// struct proc_dir_entry* demo_hooks[5];




#define PREFIX "::"
#define SPLITED '/'
static asmlinkage long fh_sys_generic(struct pt_regs *regs, asmlinkage long (*real_sys_func)(struct pt_regs *), void *reg_value);

bool is_taged_file(struct file *filp){
	char *buff;
	char *full_path;
	bool ret = false;

	buff = kzalloc(PATH_MAX, GFP_KERNEL);
	if(!buff)
		return ret;

	full_path = d_path(&filp->f_path, buff, PATH_MAX);
	if(IS_ERR(full_path))
		goto out;

	ret = !strncmp(full_path, root_tag_path, strlen(root_tag_path));
	if(ret)
		pr_info("path1: %s\n", full_path);

out:
	kfree(buff);
	return ret;
}

char *get_tag_name(char *src){
	char *end_item, *buff;
	long len;

	end_item = strchr(src, SPLITED);
	
	if(!end_item)
		end_item = src + strlen(src); // -1 ?

	len = end_item - src  - strlen(PREFIX);
	buff = kzalloc(len, GFP_KERNEL);
	strncpy(buff, src + strlen(PREFIX), len);
	return buff;
}


struct list_head *dentry_bridges;

struct dentry_bridge {
   struct list_head list;
   struct dentry *dentry;
};


struct list_head *init_list(void){
    struct list_head *list = kzalloc(sizeof(struct list_head), GFP_KERNEL);
    INIT_LIST_HEAD(list);
    return list;
}

int list_add_dentry(struct list_head *list, struct dentry *dentry){
    struct dentry_bridge *d_bridge;
	d_bridge = kzalloc(sizeof(struct dentry_bridge), GFP_KERNEL);
    if(!d_bridge)
        return -ENOMEM;

    INIT_LIST_HEAD(&d_bridge->list);
    list_add_tail(&d_bridge->list, list);
    return 1;
}


/* return true if drop dentry, else return false. */ 
bool d_drop_unused(struct dentry *dentry){
	bool ret = false;
	if(!dentry)
		return true;
	spin_lock(&dentry->d_lock);
	if(dentry->d_lockref.count == 1){
		ret = true;
		__d_drop(dentry);
	}
	spin_unlock(&dentry->d_lock);
	return ret;
}

void d_symlink_tag_release(struct dentry *dentry){
	pr_info("d_symlink_tag_release_1\n");
}

int d_symlink_tag_drop_dentry(const struct dentry *dentry)
{
	pr_info("d_symlink_tag_drop_dentry_1\n");
	return 0; // always save the dentry in cache
}


struct dentry_operations symlink_tag_dentry_operations = {
	.d_release = d_symlink_tag_release,
	.d_delete = d_symlink_tag_drop_dentry,
};

struct proc_dir_entry *red_proc = NULL;
struct dentry *red_dentry = NULL;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
unsigned long lookup_name(const char *name)
{
	struct kprobe kp = {
		.symbol_name = name
	};
	unsigned long retval;

	if (register_kprobe(&kp) < 0) return 0;
	retval = (unsigned long) kp.addr;
	unregister_kprobe(&kp);
	return retval;
}
#else
unsigned long lookup_name(const char *name)
{
	return kallsyms_lookup_name(name);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define FTRACE_OPS_FL_RECURSION FTRACE_OPS_FL_RECURSION_SAFE
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define ftrace_regs pt_regs

static __always_inline struct pt_regs *ftrace_get_regs(struct ftrace_regs *fregs)
{
	return fregs;
}
#endif

/*
 * There are two ways of preventing vicious recursive loops when hooking:
 * - detect recusion using function return address (USE_FENTRY_OFFSET = 0)
 * - avoid recusion by jumping over the ftrace call (USE_FENTRY_OFFSET = 1)
 */
#define USE_FENTRY_OFFSET 0

/**
 * struct ftrace_hook - describes a single hook to install
 *
 * @name:     name of the function to hook
 *
 * @function: pointer to the function to execute instead
 *
 * @original: pointer to the location where to save a pointer
 *            to the original function
 *
 * @address:  kernel address of the function entry
 *
 * @ops:      ftrace_ops state for this function hook
 *
 * The user should fill in only &name, &hook, &orig fields.
 * Other fields are considered implementation details.
 */
struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

static int fh_resolve_hook_address(struct ftrace_hook *hook)
{
	hook->address = lookup_name(hook->name);

	if (!hook->address) {
		pr_debug("unresolved symbol: %s\n", hook->name);
		return -ENOENT;
	}

#if USE_FENTRY_OFFSET
	*((unsigned long*) hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
	*((unsigned long*) hook->original) = hook->address;
#endif

	return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
		struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
	struct pt_regs *regs = ftrace_get_regs(fregs);
	struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
	regs->ip = (unsigned long)hook->function;
#else
	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long)hook->function;
#endif
}

/**
 * fh_install_hooks() - register and enable a single hook
 * @hook: a hook to install
 *
 * Returns: zero on success, negative error code otherwise.
 */
int fh_install_hook(struct ftrace_hook *hook)
{
	int err;

	err = fh_resolve_hook_address(hook);
	if (err)
		return err;

	/*
	 * We're going to modify %rip register so we'll need IPMODIFY flag
	 * and SAVE_REGS as its prerequisite. ftrace's anti-recursion guard
	 * is useless if we change %rip so disable it with RECURSION.
	 * We'll perform our own checks for trace function reentry.
	 */
	hook->ops.func = fh_ftrace_thunk;
	hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
	                | FTRACE_OPS_FL_RECURSION
	                | FTRACE_OPS_FL_IPMODIFY;

	err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
	if (err) {
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
		return err;
	}

	err = register_ftrace_function(&hook->ops);
	if (err) {
		pr_debug("register_ftrace_function() failed: %d\n", err);
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
		return err;
	}

	return 0;
}

/**
 * fh_remove_hooks() - disable and unregister a single hook
 * @hook: a hook to remove
 */
void fh_remove_hook(struct ftrace_hook *hook)
{
	int err;

	err = unregister_ftrace_function(&hook->ops);
	if (err) {
		pr_debug("unregister_ftrace_function() failed: %d\n", err);
	}

	err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
	if (err) {
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
	}
}

/**
 * fh_install_hooks() - register and enable multiple hooks
 * @hooks: array of hooks to install
 * @count: number of hooks to install
 *
 * If some hooks fail to install then all hooks will be removed.
 *
 * Returns: zero on success, negative error code otherwise.
 */
int fh_install_hooks(struct ftrace_hook *hooks, size_t count)
{
	int err;
	size_t i;

	for (i = 0; i < count; i++) {
		err = fh_install_hook(&hooks[i]);
		if (err)
			goto error;
	}

	return 0;

error:
	while (i != 0) {
		fh_remove_hook(&hooks[--i]);
	}

	return err;
}

/**
 * fh_remove_hooks() - disable and unregister multiple hooks
 * @hooks: array of hooks to remove
 * @count: number of hooks to remove
 */
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
		fh_remove_hook(&hooks[i]);
}

#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

/*
 * Tail call optimization can interfere with recursion detection based on
 * return address on the stack. Disable it to avoid machine hangups.
 */
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

static char *duplicate_filename(const char __user *filename)
{
	char *kernel_filename;

	kernel_filename = kmalloc(4096, GFP_KERNEL);
	if (!kernel_filename)
		return NULL;

	if (strncpy_from_user(kernel_filename, filename, 4096) < 0) {
		kfree(kernel_filename);
		return NULL;
	}

	return kernel_filename;
}

struct inode *symlink_tag(char *tag){
	struct path path;
	int err;
	struct inode *inode;
	char *symlink_str;

	symlink_str = join_path_str(root_tag_path, tag, SYMLINK_FILENAME);
	if(IS_ERR(symlink_str))
		return ERR_PTR(-ENOENT);

	pr_info("symlink_str: %s\n", symlink_str);

	err = kern_path(symlink_str, 0, &path);
	if(err)
		return ERR_PTR(-ENOENT);
	
	inode = path.dentry->d_inode;
	// ihold(inode);
	path_put(&path);
	return inode;
}

struct dentry *link_symlink_dentry(char *tag, struct inode *inode){
	struct path pwd;
	struct qstr qname;
	struct dentry *dentry, *dir;

	pwd = current->fs->pwd;
	dir = pwd.dentry;

	qname.name = tag;
	qname.hash_len = hashlen_string(dir, tag);

	dentry = d_lookup(dir, &qname);
	
	if(dentry){
		if(dentry->d_inode)
			goto out;
		else
			dput(dentry);
	}
	dentry = d_alloc_name(pwd.dentry, tag);
	
	if(dentry){
		// dentry->d_fsdata = (void *)ns->ops;
		d_set_d_op(dentry, &symlink_tag_dentry_operations);
		// __list_del_entry(&dentry->d_child); // ???
		d_add(dentry, inode);
		red_dentry = dentry; // dget(dentry);
		// list_add_dentry(dentry)
		// hlist_bl_node
	}else
		ERR_PTR(-ENOMEM);
out:
	return dentry;
}

struct dentry *link_tag_cwd(char *tag){
	struct inode *inode;
	struct dentry *dentry;
	char *buff;
	inode = symlink_tag(tag);

	if(IS_ERR(inode))
		return ERR_PTR(-ENOENT);
		
	buff = kzalloc(strlen(PREFIX) + strlen(tag), GFP_KERNEL);
	sprintf(buff, "%s%s", PREFIX, tag);
	pr_info("buff: %s\n", buff);
	dentry = link_symlink_dentry(buff, inode);
	kfree(buff);
	return dentry;
}

#ifdef PTREGS_SYSCALL_STUBS
static asmlinkage long (*real_sys_openat)(struct pt_regs *regs);
static asmlinkage long (*real_sys_getdents)(struct pt_regs *regs);
static asmlinkage long (*real_sys_getdents64)(struct pt_regs *regs);
static asmlinkage long (*real_sys_statx)(struct pt_regs *regs);
static asmlinkage long (*real_sys_stat)(struct pt_regs *regs);
// static asmlinkage long (*real_sys_stat64)(struct pt_regs *regs);
static asmlinkage long (*real_sys_lstat)(struct pt_regs *regs);
static asmlinkage long (*real_sys_lgetxattr)(struct pt_regs *regs);








static asmlinkage long fh_sys_openat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_openat, regs->si);
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

// static asmlinkage long fh_sys_stat64(struct pt_regs *regs){
// 	return fh_sys_generic(regs, real_sys_stat64, regs->di);
// }

static asmlinkage long fh_sys_lgetxattr(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lgetxattr, regs->di);
}

static asmlinkage long fh_sys_lstat(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_lstat, regs->di);
}

static asmlinkage long fh_sys_getdents(struct pt_regs *regs){
	return fh_sys_generic(regs, real_sys_getdents, regs->si);
}

static asmlinkage long (*real_sys_close)(struct pt_regs *regs);

static asmlinkage long sys_close(struct pt_regs *regs){
	long ret;
	struct file *filp;
	unsigned fd;

	fd = (unsigned)regs->di;

	filp = fget_raw(fd);
	if(!filp)
		goto out;

	if(!is_taged_file(filp))
		goto out_hook;

	pr_info("sys_close1\n");

out_hook:
	fput(filp);
	
out:
	ret = real_sys_close(regs);
	return ret;
}
#else
static asmlinkage long (*real_sys_openat)(int dfd, const char __user *filename,
				int flags, umode_t mode);

static asmlinkage long fh_sys_openat(int dfd, const char __user *filename,
				int flags, umode_t mode)
{
	long ret;
	char *kernel_filename;

	kernel_filename = duplicate_filename(filename);
	if (strncmp(kernel_filename, "red", 3) == 0)
	{
		pr_info("opened file: %s\n", kernel_filename);
		kfree(kernel_filename);
		ret = real_sys_openat(dfd, filename, flags, mode);
		pr_info("fd returned is %ld\n", ret);
		return ret;
		
	}

	kfree(kernel_filename);

	ret = real_sys_openat(filename, flags, mode); //  real_sys_openat(fd, filename, flags, mode); ??? (fd missing)

	return ret;
}
#endif


/*
 * x86_64 kernels have a special naming convention for syscall entry points in newer kernels.
 * That's what you end up with if an architecture has 3 (three) ABIs for system calls.
 */
#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif

#define HOOK(_name, _function, _original)	\
	{					\
		.name = (_name),	\
		.function = (_function),	\
		.original = (_original),	\
	}

static struct ftrace_hook demo_hooks[] = {
	HOOK(SYSCALL_NAME("sys_statx"), fh_sys_statx, &real_sys_statx),
	HOOK(SYSCALL_NAME("sys_stat"), fh_sys_stat, &real_sys_stat),
	// HOOK(SYSCALL_NAME("sys_stat64"), fh_sys_stat64, &real_sys_stat64),
	HOOK(SYSCALL_NAME("sys_lstat"), fh_sys_lstat, &real_sys_lstat),
	// HOOK(SYSCALL_NAME("sys_lgetxattr"), fh_sys_lgetxattr, &real_sys_lgetxattr),
	HOOK(SYSCALL_NAME("sys_openat"), fh_sys_openat, &real_sys_openat),
	HOOK(SYSCALL_NAME("sys_getdents"), fh_sys_getdents, &real_sys_getdents),
	HOOK(SYSCALL_NAME("sys_getdents64"), fh_sys_getdents64, &real_sys_getdents64),
	HOOK(SYSCALL_NAME("sys_close"), sys_close, &real_sys_close),
};

int start_hooks(void)
{
	int err;
	dentry_bridges = init_list();

	err = fh_install_hooks(demo_hooks, ARRAY_SIZE(demo_hooks));
	if (err)
		return err;

	pr_info("start_hooks end\n");

	return 0;
}

void close_hooks(void)
{
	fh_remove_hooks(demo_hooks, ARRAY_SIZE(demo_hooks));

	if(red_dentry){
		d_drop(red_dentry); //  or d_invalidate(red_dentry);
	}
	
	if(red_proc)
		proc_remove(red_proc);

}
// wrapper
static asmlinkage long fh_sys_generic(struct pt_regs *regs, asmlinkage long (*real_sys_func)(struct pt_regs *), void *reg_value) // <<<
{
	
	long ret;
	int err;
	char *kernel_filename;
	char *tag_name;
	struct dentry *dentry;

	kernel_filename = duplicate_filename((void *)reg_value);//regs->si

	if (strncmp(kernel_filename, PREFIX, strlen(PREFIX)) == 0)
	{
		pr_info("is taged request\n");

		tag_name = get_tag_name(kernel_filename);

		/*
		* Pass only the first item in path without prefix:
		* kernel_filename = "::red/a1" => link_tag_cwd("red")
		*/

		dentry = link_tag_cwd(tag_name);
		
		if(IS_ERR(dentry)){
			return -EINVAL;
		}
		kfree(kernel_filename);
		ret = real_sys_func(regs);

		if(dentry && d_drop_unused(dentry))
			red_dentry = NULL;
		
		return ret;
	}

	kfree(kernel_filename);
	ret = real_sys_func(regs);

	return ret;
}