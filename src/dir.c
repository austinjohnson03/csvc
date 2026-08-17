#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

char **list_directory(const char *path) {
  DIR *dir = opendir(path);
  if (dir == NULL) {
    fprintf(stderr, "Unable to open directory: %s\n", path);
    return NULL;
  }

  size_t capacity = 8;
  size_t count = 0;
  char **names = malloc(sizeof(char *) * capacity);
  if (names == NULL) {
    closedir(dir);
    return NULL;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    if (count + 1 >= capacity) {
      capacity *= 2;
      char **new_names = realloc(names, sizeof(char *) * capacity);
      if (new_names == NULL) {
        for (size_t i = 0; i < count; ++i) free(names[i]);

        free(names);
        closedir(dir);
        return NULL;
      }
      names = new_names;
    }

    names[count] = strdup(entry->d_name);
    if (names[count] == NULL) {
      for (size_t i = 0; i < count; ++i) free(names[i]);
      free(names);
      closedir(dir);
      return NULL;
    }
    count++;
  }

  names[count] = NULL;
  closedir(dir);
  return names;
}

char *join_path(const char *dir, const char *filename) {
  size_t len = strlen(dir) + strlen(filename) + 2;
  char *path = malloc(len);
  if (path == NULL) {
    fprintf(stderr, "Unable to allocate space for path\n");
    return NULL;
  }

  snprintf(path, len, "%s/%s", dir, filename);
  return path;
}

