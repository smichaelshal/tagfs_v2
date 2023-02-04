#ifndef VTAGFS_USER_API_H_
#define VTAGFS_USER_API_H_

#include <asm/ioctl.h>
#include <linux/fs.h>

#define TAGFS_DEVICE_NAME "tagfs"
#define TAGFS_DEVICE_FILE_NAME "tagfs" // dev/tagfs
#define MAX_TAG_LEN 253 // <<< update

struct add_single_tag {
    char tag[MAX_TAG_LEN];
    unsigned int fd;
};


struct lookup_single_tag {
    char tag[MAX_TAG_LEN];
};

#define IOCTL_BASE 'm' + 0x58
#define IOCTL_QUERY 0x67
#define IOCTL_LOOKU_QUERY 0x68
#define IOCTL_ADD_TAG _IOR(IOCTL_BASE, IOCTL_QUERY, struct add_single_tag)
#define IOCTL_DELETE_TAG _IOW(IOCTL_BASE, IOCTL_QUERY, struct add_single_tag)
#define IOCTL_LOOKUP_TAG _IOR(IOCTL_BASE, IOCTL_LOOKU_QUERY, struct lookup_single_tag)


#endif /* VTAGFS_USER_API_H_ */
