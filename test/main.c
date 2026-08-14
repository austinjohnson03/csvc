#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

char **list_directory(const char *path);
char *join_path(const char *dir, const char *filename);

int main(void) {
  char *read_dir = "./test/original";
  char *write_dir = "./test/actual";
  char **files = list_directory(read_dir);

  if (files == NULL) {
    fprintf(stderr, "Unable to open directory: %s\n", read_dir);
    return EXIT_FAILURE;
  }

  for (size_t i = 0; files[i] != NULL; ++i) {
    char *read_path = join_path(read_dir, files[i]);
    if (read_path == NULL) {
      continue;
    }
    char *write_path = join_path(write_dir, files[i]);
    if (write_path == NULL) {
      continue;
    }

    Parser *p = parser_init();
    if (p == NULL) {
      free(read_path);
      free(write_path);
      continue;
    }
    if (parse_from_file(p, read_path, ',') == -1) {
      free(read_path);
      free(write_path);
      parser_free(p);
      continue;
    }

    if (write_to_file(p, write_path, ',') == -1) {
      free(read_path);
      free(write_path);
      parser_free(p);
      continue;
    }
    free(read_path);
    free(write_path);
    parser_free(p);
  }

  for (size_t i = 0; files[i] != NULL; ++i) {
    free(files[i]);
  }
  free(files);

  return EXIT_SUCCESS;
}

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
