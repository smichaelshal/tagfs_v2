#ifndef HOOKS_H_
#define HOOKS_H_

#include <linux/version.h>
#include <linux/kprobes.h>

// #include <linux/ftrace.h>
// #include <linux/kallsyms.h>
// #include <linux/kernel.h>
// #include <linux/linkage.h>
// #include <linux/module.h>
// #include <linux/slab.h>
// #include <linux/uaccess.h>




#define USE_FENTRY_OFFSET 0


/*
 * There are two ways of preventing vicious recursive loops when hooking:
 * - detect recusion using function return address (USE_FENTRY_OFFSET = 0)
 * - avoid recusion by jumping over the ftrace call (USE_FENTRY_OFFSET = 1)
 */

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


extern int start_hooks(void);
extern void close_hooks(void);

extern unsigned long lookup_name(const char *name);

#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif

#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
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


extern struct ftrace_hook generic_hooks[];
extern size_t generic_hooks_size;
extern asmlinkage long fh_sys_generic(struct pt_regs *regs, asmlinkage long (*real_sys_func)(struct pt_regs *), void *reg_value);

#endif /* HOOKS_H_ */
