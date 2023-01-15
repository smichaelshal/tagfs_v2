#ifndef UTILS_H_
#define UTILS_H_


#define SPLITED "/"
#define SPLITED_LEN 1

extern char *_join_path_str(int count, ...);

#define COUNT(...) _COUNT(__VA_ARGS__, 5, 4, 3, 2, 1)
#define _COUNT(a, b, c, d, e, count, ...) count
#define join_path_str(...) _join_path_str(COUNT(__VA_ARGS__), __VA_ARGS__)



#endif /* UTILS_H_ */
