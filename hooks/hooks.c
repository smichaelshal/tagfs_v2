#define pr_fmt(fmt) "ftrace_hook: " fmt

// #include <linux/kernel.h>
// #include <linux/linkage.h>
// #include <linux/module.h>
// #include <linux/slab.h>
// #include <linux/uaccess.h>
// #include <linux/version.h>

// #include <linux/kallsyms.h>
// #include <linux/ftrace.h>
// #include <linux/kprobes.h>

#include <linux/fs_struct.h>

#include "hooks.h"
#include "../vtagfs.h"
#include "../utils/utils.h"


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
static unsigned long lookup_name(const char *name)
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



/*
 * Tail call optimization can interfere with recursion detection based on
 * return address on the stack. Disable it to avoid machine hangups.
 */
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif



// ------------------------------------

static asmlinkage long (*real_sys_path_umount)(struct pt_regs *regs);

static asmlinkage long fh_sys_path_umount(struct pt_regs *regs){
	return real_sys_path_umount(regs);
}

struct ftrace_hook special_hooks[] = {
	HOOK("path_umount", fh_sys_path_umount, &real_sys_path_umount),
};

size_t special_hooks_size = ARRAY_SIZE(special_hooks);

int start_hooks(void){
	int err;

	// err = fh_install_hooks(special_hooks, special_hooks_size);
	// if (err)
	// 	return err;

	err = fh_install_hooks(generic_hooks, generic_hooks_size);
	if (err)
		return err;

	pr_info("start_hooks end\n");

	return 0;
}

void close_hooks(void){
	fh_remove_hooks(generic_hooks, generic_hooks_size);
}

// ###################


struct proc_dir_entry *red_proc = NULL;
struct dentry *red_dentry = NULL;

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
		// d_set_d_op(dentry, &symlink_tag_dentry_operations);
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

asmlinkage long fh_sys_generic(struct pt_regs *regs, asmlinkage long (*real_sys_func)(struct pt_regs *), void *reg_value) // <<<
{
	
	long ret;
	int err;
	char *kernel_filename;
	char *tag_name;
	struct dentry *dentry;

	kernel_filename = dup_name_user((void *)reg_value);

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


