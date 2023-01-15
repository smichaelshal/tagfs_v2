#include "utils.h"

#include <linux/slab.h>
#include <linux/string.h>

char *_join_path_str(int count, ...) {
	va_list	arg_ptr;
	int i, err, len;
	char *path, *current_item, *pointer;

	if(!count)
		return ERR_PTR(-EINVAL);

	path = kzalloc(PATH_MAX, GFP_KERNEL);

	if(!path)
		return ERR_PTR(-ENOMEM);

	pointer = path;
	va_start(arg_ptr, count);
	for (i = 0; i < count; i++) {
		current_item = va_arg(arg_ptr, char*);
		len = strlen(current_item);
		
		if(!len || (pointer - path) + len + 1 > PATH_MAX){
			err = -EINVAL;
			goto out_error;
		}

		strncpy(pointer, SPLITED, SPLITED_LEN);
		pointer += SPLITED_LEN;

		if(current_item[0] == SPLITED[0])
			pointer -= SPLITED_LEN;

		strncpy(pointer, current_item, strlen(current_item));
		pointer += len;
	}	
out:
	return path;

out_error:
	kfree(path);
	return ERR_PTR(err);
}
