#ifndef VTAGFS_H_
#define VTAGFS_H_

#include <linux/module.h>
#include <linux/export.h>


#define DBG_VERSION "1.4"
#undef pr_fmt
#define pr_fmt(fmt) "%s: %s: " fmt, __func__, DBG_VERSION


#include "utils/utils.h"
#include "publisher/publisher.h"
#include "hooks/hooks.h"
#include "api/api.h"

// #define root_tag_path 

extern char *root_tag_path;
// module_param(root_tag_path, charp ,0660);





MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Hacker");



#endif /* VTAGFS_H_ */
