#ifndef _FILE_H_
#define _FILE_H_
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#define INITIAL_CONTENT_LENGTH 1024
#define INODES_AMOUNT 1024
#define MAX_PATH 256
#define ERROR -1

#define a "bin\nboot\ndev\netc\nhome\nlib\nmedia\n\0"

enum inode_type { INODE_FILE, INODE_DIR };

typedef struct inode {
	char path[MAX_PATH];
	char* content;
	int size;
	enum inode_type type;
	time_t last_access;
	time_t last_modification;
	time_t creation_time;
	gid_t group;
	uid_t owner;
	mode_t permissions;
} inode_t;

typedef struct superblock {
	inode_t inodes[INODES_AMOUNT];
	int inode_bitmap[INODES_AMOUNT];
	int inode_amount;
} superblock_t;

extern superblock_t superblock;

int search_inode(const char *path);
void get_next_entry(char *content, off_t *offset, char *buff);
int search_free_inode();
int create_inode(const char *path, mode_t mode, enum inode_type type);
inode_t* get_parent(const char path[MAX_PATH], int *error);
void add_dentry_to_content(char **content, int *content_size, char *dentry);
void remove_inode(const char *path, int inode_index);
int add_dentry_to_parent_dir(const char *path);
void serialize(int fp);
void deserialize(int fp);

#endif  // _FILE_H_