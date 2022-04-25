#define _XOPEN_SOURCE 600 /* Get nftw() */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>
#include <pwd.h>
#include <grp.h>

#define FLAGS 0

int directories = 0, files = 0;

int list(const char *name, const struct stat *status, int type, struct FTW *ftwb) { 

    // unknown type of file
    if (type == FTW_NS) {
        return 0;
    }

    // hidden files
    size_t index = 0;
    while (name[index] != '\0') {
        if (name[index] == '/' & name[index+1] == '.') {
            return 0;
        }
        index++;
    }

    char file_type;
    char permissions[9];
    int constants[9] = {S_IRUSR, S_IWUSR, S_IXUSR,
                    S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
    uid_t user_id = status->st_uid;
    gid_t group_id = status->st_gid;
    off_t size = status->st_size;
    size_t name_len = strlen(name);

    switch (status->st_mode & S_IFMT) {  /* Print file type */
        case S_IFREG:  file_type = '-'; break;
        case S_IFDIR:  file_type = 'd'; break;
        case S_IFCHR:  file_type = 'c'; break;
        case S_IFBLK:  file_type = 'b'; break;
        case S_IFLNK:  file_type = 'l'; break;
        case S_IFIFO:  file_type = 'p'; break;
        case S_IFSOCK: file_type = 's'; break;
        default:       file_type = '?'; break; /* Should never happen (on Linux) */
    }   
        
    // permissions
    for (size_t i = 0; i < 9; i++) {
        if (i%3 == 0) {
            if (status->st_mode & constants[i]) {
                permissions[i] = 'r';
            }
            else {
                permissions[i] = '-';
            }
        }
        if (i%3 == 1) {
            if (status->st_mode & constants[i]) {
                permissions[i] = 'w';
            }
            else {
                permissions[i] = '-';
            }
        }
        if (i%3 == 2) {
            if (status->st_mode & constants[i]) {
                permissions[i] = 'x';
            }
            else {
                permissions[i] = '-';
            }
        }
    }

    // user name
    struct passwd *psw = getpwuid(user_id);
    char *user_name = psw->pw_name;

    // group name
    struct group *grp = getgrgid(group_id);
    char *group_name = grp->gr_name;

    // size
    int len = 0;
    int i = name_len-1;
    while (name[i] != '/' && i >= 0) {
        len++;
        i--;
    }

    // file name
    char file_name[len];
    i = name_len-1;
    int offset = name_len - len;
    strcpy(file_name, name + offset);

    // print lines like tree
    if (ftwb->level != 0) {
        for (int i = 1; i < ftwb->level; i++) {
            printf("│   ");
        }
        printf("├── ");
        // printf("└── ");
    }

    int indentation = ftwb->level;
    if (indentation != 0) {
        if (file_type == 'd') {
            directories++;
        }
        else {
            files++;
        }
    }
    else {
        printf("%s\n", file_name); 
        return 0;
    }

    printf("[%c%s %s\t %s\t\t%ld]  %s\n", file_type, permissions, user_name, group_name ,size, file_name);

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc == 1) {
        nftw(".", list, 10, FLAGS);
    }
    else if (argc == 2) {
        nftw(argv[1], list, 10, FLAGS);
    }
    else {
        printf("Usage: %s directory-path\n", argv[0]);
        exit(1);
    }

    printf("\n%d directories, %d files\n", directories, files);
        
    return 0;
}
