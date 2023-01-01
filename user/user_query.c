#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "../api/user.h"

#define DEVICE_PATH	"/dev/tagfs"
#define FILE_PATH "/home/john/dir1/dir2/dir1/f1"
#define TAG_NAME "red"

int lookup_tag(int fd, char *tag){
    struct lookup_single_tag *request;
    int err;

    request = calloc(1, sizeof(struct lookup_single_tag));
    memcpy(request->tag, tag, strlen(tag));

    if (ioctl(fd, IOCTL_LOOKUP_TAG, request) < 0) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
}

int add_tag(int fd, char *path, char *tag){
    struct add_single_tag *request;
    int fd_file;
    int err;


    fd_file = open(path, O_RDONLY);

    if (fd_file < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

    request = calloc(1, sizeof(struct add_single_tag));
    memcpy(request->tag, tag, strlen(tag));
    request->fd = fd_file;

    if (ioctl(fd, IOCTL_ADD_TAG, request) < 0) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char **argv)
{
    unsigned int fd_dev;
    int err;
    fd_dev = open(DEVICE_PATH, O_RDONLY);

    if (fd_dev < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

    if(argc == 2){
        err = lookup_tag(fd_dev, argv[1]);
    }
    else if(argc == 3){
        err = add_tag(fd_dev, argv[1], argv[2]);
    }else{
        printf("error args\n");
        return -1;
    }

    // fclose(fd2);

    return 0;
}