#ifndef CSVC_FILE_DIR_H
#define CSVC_FILE_DIR_H

char **list_directory(const char *path);
char *join_path(const char *dir, const char *filename);
int create_directory(const char *path);

#endif // CSVC_FILE_DIR_H
