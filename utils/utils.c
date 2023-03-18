#include "utils.h"

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>





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


char *dup_name(const char *name){
    /*
    * alloc new buffer and copy to buffer the name.
    * if success return buffer with the name, else return NULL.
    */

    char *new_name = kzalloc(strlen(name), GFP_KERNEL);
    if(new_name)
        strcpy(new_name, name);
    return new_name;
}

char *get_tag_name(char *src){
	char *end_item, *buff;
	long len;

	end_item = strchr(src, SPLITED_CHAR);
	
	if(!end_item)
		end_item = src + strlen(src); // -1 ?

	len = end_item - src  - strlen(PREFIX);
	buff = kzalloc(len, GFP_KERNEL);
	strncpy(buff, src + strlen(PREFIX), len);
	return buff;
}

char *dup_name_user(const char __user *filename)
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

char *long_to_string(long num){
    char *buff;
    buff = kzalloc(sizeof(10), GFP_KERNEL);
    if(!buff)
        return -ENOMEM;
    sprintf(buff, "%ld", num);
    return buff;
}

long string_to_long(char *buff, int base){
    long nr;
    int err;
    err = kstrtol(buff, base, &nr);
    if(IS_ERR(err))
        return err;
    return nr;
}

