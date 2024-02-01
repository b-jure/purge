/* ---------------------------------------------------------------------
 * Copyright (C) 2024 Jure Bagić
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * --------------------------------------------------------------------- */

#define _DEFAULT_SOURCE // for DT_DIR (enum)
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATH_SEP '/'

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


struct Buffer {
    char path[BUFSIZ];
    size_t len;
};

int path_append(struct Buffer* buffer, const char* str, unsigned char root)
{
    size_t len = strlen(str);
    size_t n = len + buffer->len + (root ? 2 : 1);
    if(n > sizeof(buffer->path)) return -1;
    char* end = &buffer->path[buffer->len];
    if(!root) {
        *end++ = PATH_SEP;
        buffer->len++;
    }
    memcpy(end, str, len);
    buffer->len += len;
    buffer->path[buffer->len] = '\0';
    return 0;
}


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

int process_args(
    int argc,
    char** argv,
    struct Buffer* path,
    const char** target,
    unsigned char* type)
{
    if(argc == 0) return -1;
    while(argc-- > 0) {
        if((*argv)[0] == '-') { // HAVE OPTIONS ?
            if(process_options(&(*argv)[1], type) == -1) return -1;
            argv++;
        } else {
            if(argc == 0) return -1;
            if(path_append(path, *argv++, 1) == -1) return -2;
            *target = *argv;
            break;
        }
    }
    return 0;
}

int purge_dir(struct Buffer* path, const char* target, unsigned char in)
{
    struct Buffer dirpath = *path;
    int status;
    DIR* dirp = opendir(path->path);
    if(dirp == NULL) goto l_errno_error;
    status = errno = 0;
    struct dirent* de;
    while((de = readdir(dirp))) {
        if(path_append(path, de->d_name, 0) == -1) {
            status = -1;
            goto l_close_dir;
        }
        if(in && de->d_type != DT_DIR) {
            unlink(path->path);
        } else if(
            de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            unsigned char istarget = (in || strcmp(de->d_name, target) == 0);
            if((status = purge_dir(path, target, istarget) == -1)) {
                goto l_close_dir;
            } else if(status > 0) {
                goto l_errno_error;
            }
        }
        *path = dirpath; // restore path
    }
    if(in) {
        closedir(dirp);
        if(rmdir(path->path) != 0) {
            perror("purge");
            return errno;
        }
        return 0;
    }
l_errno_error:
    if((status = errno) != 0) perror("purge");
l_close_dir:
    closedir(dirp);
    return status;
}

int purge_nondir(struct Buffer* path, const char* target)
{
    struct Buffer dirpath = *path;
    int status;
    DIR* dirp = opendir(path->path);
    if(dirp == NULL) goto l_errno_error;
    status = errno = 0;
    struct dirent* de;
    while((de = readdir(dirp))) {
        if(path_append(path, de->d_name, 0) == -1) {
            status = -1;
            goto l_close_dir;
        }
        if(de->d_type != DT_DIR && strcmp(de->d_name, target) == 0) {
            unlink(path->path);
        } else if(
            de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            if((status = purge_nondir(path, target) == -1)) {
                goto l_close_dir;
            } else if(status > 0) {
                break;
            }
        }
        *path = dirpath; // restore path
    }
l_errno_error:
    if((status = errno) != 0) perror("purge");
l_close_dir:
    closedir(dirp);
    return status;
}

int main(int argc, char* argv[])
{
    struct Buffer path;
    path.path[0] = '\0';
    path.len = 0;
    int status = 0;
    const char* target = NULL;
    unsigned char type = 0;
    argv++, argc--;
    status = process_args(argc, argv, &path, &target, &type);
    if(status == -1) {
        fputs(USAGE, stderr);
        return 1;
    } else if(status == -2) {
    l_path_error:
        fputs("purge: PATH length exceeded.\n", stderr);
        return 1;
    }
    if(type == 0) status = purge_nondir(&path, target);
    else status = purge_dir(&path, target, 0);
    if(status == -1) goto l_path_error;
    if(status != 0) {
        perror("purge");
        return status;
    }
    return 0;
}
