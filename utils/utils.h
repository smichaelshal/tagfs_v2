#ifndef UTILS_H_
#define UTILS_H_

#define SPLITED "/"
#define SPLITED_CHAR '/'
#define SPLITED_LEN 1
#define PREFIX "::"
#define SYMLINK_FILENAME "sym1"

extern char *_join_path_str(int count, ...);

#define COUNT(...) _COUNT(__VA_ARGS__, 5, 4, 3, 2, 1)
#define _COUNT(a, b, c, d, e, count, ...) count
#define join_path_str(...) _join_path_str(COUNT(__VA_ARGS__), __VA_ARGS__)

extern char *dup_name(const char *name);
extern char *get_tag_name(char *src);
extern char *dup_name_user(const char __user *filename);

extern char *long_to_string(long num);
extern long string_to_long(char *buff, int base);

#endif /* UTILS_H_ */
