#define _DEFAULT_SOURCE // for DT_DIR (enum)
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define USAGE                                                                                      \
    "purge [OPTIONS]... PATH TARGET\n"                                                             \
    "   OPTIONS\n"                                                                                 \
    "       -f      specifies that TARGET is a non-directory file\n"                               \
    "       -d      specifies that TARGET is a directory\n"                                        \
    "               by not specifying TARGET option the default TARGET will be any non-dir file\n" \
    "   PATH\n"                                                                                    \
    "       path where to start the purge.\n"                                                      \
    "   TARGET\n"                                                                                  \
    "       target file or directory to purge.\n"

int process_options(const char* opt, unsigned char* type)
{
    const char* start = opt;
    for(register char c = *opt; c; c = *++opt) {
        switch(c) {
            case 'f': *type = 0; break;
            case 'd': *type = DT_DIR; break;
            case '-':
                if(start == (opt - 1) && *++opt == '\0') continue; // will return on next iteration
                // FALLTHRU
            default: return -1;
        }
    }
    return 0;
}

int process_args(int argc, char** argv, const char** path, const char** target, unsigned char* type)
{
    while(argc-- > 0) {
        if((*argv)[0] == '-') { // HAVE OPTIONS ?
            if(process_options(&(*argv)[1], type) == -1) return -1;
        } else {
            if(argc == 0) return -1;
            *path = *argv++;
            *target = *argv;
            break;
        }
    }
    return 0;
}

int purge_dir(const char* path, const char* target, unsigned char in)
{
    DIR* dirp = opendir(path);
    if(dirp == NULL) return -1;
    errno = 0;
    struct dirent* de;
    while((de = readdir(dirp))) {
        if(in && de->d_type != DT_DIR) unlink(de->d_name);
        else if(
            de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            if(purge_dir(de->d_name, target, strcmp(de->d_name, target) == 0) == 0)
                rmdir(de->d_name);
        }
    }
    closedir(dirp);
    return errno;
}

int purge_nondir(const char* path, const char* target)
{
    DIR* dirp = opendir(path);
    if(dirp == NULL) return -1;
    errno = 0;
    struct dirent* de;
    while((de = readdir(dirp))) {
        if(de->d_type != DT_DIR && strcmp(de->d_name, target) == 0) unlink(de->d_name);
        else if(
            de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
            purge_nondir(de->d_name, target);
    }
    closedir(dirp);
    return errno;
}

int main(int argc, char* argv[])
{
    int status = 0;
    const char *path, *target = NULL;
    unsigned char type = 0;
    argv++, argc--;
    if(argc == 0 || process_args(argc, argv, &path, &target, &type) == -1) {
        fputs(USAGE, stderr);
        return 1;
    }
    if(type == 0) status = purge_nondir(path, target);
    else status = purge_dir(path, target, 0);
    if(status != 0) {
        perror("Errored while reading directory entries");
        return status;
    }
    return 0;
}
