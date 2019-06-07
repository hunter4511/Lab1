#define FUSE_USE_VERSION 31

#include <errno.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static int _counter = 0;

typedef struct File {
	char *path;
	int rights;
	bool isDirectory;
	char *contents;
} File;

#define MAX_FILES 1000
File files[MAX_FILES];

size_t filesize(const char *filename) {
	struct stat st;
	size_t retval = 0;
	if (stat(filename, &st))
		printf("cannot stat %s\n", filename);
	else retval = st.st_size;
	return retval;
}

bool is_slash(const char *str, int from) {
	for (int i = from; i < strlen(str); i++) {
		if (str[i] == '/') {
			return true;
		}
	}
	return false;
}

int counter(int i) {
	return i == _counter ? -ENOENT : 0;
}





static int q_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
	off_t offset, struct fuse_file_info *fi)
{
	(void)offset;
	(void)fi;
	int i;
	for (i = 0; i < _counter; i++) {
		printf("%s \n",files[i].path);
		int len_i = strlen(files[i].path);
		if (files[i].isDirectory && !strcmp(path, files[i].path)) {
			if (files[i].rights < 0400) {
				return -EACCES;
			}
			filler(buf, ".", NULL, 0);
			filler(buf, "..", NULL, 0);
			for (int j = 0; j < _counter; j++) {
				int len_j = strlen(files[j].path);
				if (len_i < len_j && !strncmp(files[j].path, files[i].path, len_i)) {
					if (!strcmp(files[i].path, "/") && !is_slash(files[j].path, len_i)) {
						filler(buf, files[j].path + len_i, NULL, 0);
					}
					else if (!is_slash(files[j].path, len_i + 1)) {
						filler(buf, files[j].path + len_i + 1, NULL, 0);
					}
				}
			}
			break;
		}
	}
	return counter(i);
}

static int q_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));
	int i;
	for (i = 0; i < _counter; i++) {
		if (!strcmp(path, files[i].path)) {
			int flag = files[i].isDirectory ? S_IFDIR : S_IFREG;
			stbuf->st_mode = flag | files[i].rights;
			stbuf->st_nlink = files[i].isDirectory ? 2 : 1;
			if (!files[i].isDirectory) {
				stbuf->st_size = strlen(files[i].contents);
			}
			break;
		}
	}
	return counter(i);
}
static int q_read(const char *path, char *buf, size_t size, off_t offset,
	struct fuse_file_info *fi)
{
	size_t len;
	(void)fi;

	int i;
	for (i = 0; i < _counter; i++) {
		if (!files[i].isDirectory && !strcmp(path, files[i].path)) {
			if (files[i].rights < 0400) {
				return -EACCES;
			}
			len = strlen(files[i].contents);
			if (offset < len) {
				if (offset + size > len)
					size = len - offset;
				memcpy(buf, files[i].contents + offset, size);
			}
			else size = 0;
			return size;
		}
	}
	return -ENOENT;
}
void add_file_base(const char *path, int rights, bool isDirectory, char *contents) {
	files[_counter].path = strdup(path);
	files[_counter].rights = rights;
	files[_counter].isDirectory = isDirectory;
	if (contents != NULL) {
		files[_counter].contents = strdup(contents);
	}
	_counter++;
}

void add_directory(const char *path, int rights) {
	add_file_base(path, rights, true, NULL);
}
void add_file(const char *path, int rights, char *contents) {
	add_file_base(path, rights, false, contents);
}



static int q_mkdir(const char *path, mode_t mode) {
	bool isNameOk = false;
	int len_path = strlen(path);
	for (int i = 0; i < _counter; i++) {
		if (!strcmp(files[i].path, path)) {
			return -EEXIST;
		}
		int len = strlen(files[i].path);
		if (len_path > len && !strncmp(path, files[i].path, len)) {
			isNameOk = true;
		}
	}

	if (!isNameOk) {
		return -ENAMETOOLONG;
	}

	add_directory(path, mode);

	return 0;
}
static int q_rename(const char* from, const char* to) {
    for(int i = 0; i < _counter; i++) {
        if(!strcmp(files[i].path, from)) {
		    files[i].path = strdup(to);
        }
    }
	return 0;
}

static struct fuse_operations operations = {
	.read = q_read,
	.readdir = q_readdir,
	.getattr = q_getattr,
	.mkdir = q_mkdir,
    .rename = q_rename,
};


int main(int argc, char *argv[])
{
	static const char *system_echo_path = "/home/plastb1r/OS/lab5/mnt_fuse_not_save";
	
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	add_directory("/", 0777);
	add_directory("/bar", 0755);
	add_directory("/bar/baz", 0744);

	int bufferSize = 40000;
	char echoBuffer[bufferSize];
	FILE *fecho = fopen(system_echo_path, "rb");
	unsigned char c;
	int i = 0;
	while (fread(&c, 1, 1, fecho)) {
		echoBuffer[i] = c;
		i++;
	}
	fclose(fecho);
    
    char str[51];
    for (int i = 0; i < 24; i++) {
        str[i * 2] = i;
        str[i * 2 + 1] = '\n';
    }
    str[50] = '\0';

	add_file("/bar/bin/head", 0177, echoBuffer);
	add_file("/bar/bin/readme.txt", 0544, "Tolstenko Lera 16160163\n");
	add_directory("/bar/bin/foo", 0711);
	add_file("/bar/foo/test.txt", 0444, str);
	add_file("/bar/foo/baz", 0777, str);
	add_file("/bar/foo/example", 0555, "Hello World!\n");
	return fuse_main(args.argc, args.argv, &operations, NULL);
}