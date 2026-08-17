#define _POSIX_C_SOURCE 200809L
#include "csv/csv.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *parser_init(void) {
  Parser *p = malloc(sizeof(Parser));

  if (p == NULL) {
    fprintf(stderr, "Unable to allocate space for parser\n");
    return NULL;
  }

  p->rows = NULL;
  p->count = 0;

  return p;
}

int add_data_to_parser(Parser *p, char **fields, size_t field_count) {
  CsvRow *new_rows = realloc(p->rows, (p->count + 1) * sizeof(CsvRow));
  if (new_rows == NULL) {
    fprintf(stderr, "Unable to allocate space for new row.\n");
    return -1;
  }
  p->rows = new_rows;

  CsvRow *row = &p->rows[p->count];

  row->fields = malloc(field_count * sizeof(char *));

  if (row->fields == NULL) {
    fprintf(stderr, "Unable to allocate space for new fields.\n");
    return -1;
  }

  for (size_t i = 0;i < field_count; i++) {
    row->fields[i] = strdup(fields[i]);

    if (row->fields[i] == NULL) {
      for (size_t j = 0; j < i; j++) {
        free(row->fields[j]);
      }
      return -1;
    }
  }

  row->count = field_count;
  p->count++;

  return 0;
}

static bool parse_line(const char *line, const char delimiter,
                       CsvRow *out_row) {
  size_t capacity = 4;
  char **fields = malloc(sizeof(char *) * capacity);
  if (fields == NULL) return false;

  size_t count = 0;
  size_t buf_cap = 64;
  char *buf = malloc(buf_cap);
  if (buf == NULL) {
    free(fields);
    return false;
  }

  size_t buf_len = 0;
  bool in_quotes = false;

  for (const char *p = line;; ++p) {
    char c = *p;

    if (buf_len + 1 >= buf_cap) {
      buf_cap *= 2;
      char *new_buf = realloc(buf, buf_cap);
      if (new_buf == NULL) {
        free(buf);
        free(fields);
        return false;
      }
      buf = new_buf;
    }

    if (in_quotes) {
      if (c == '"') {
        if (*(p + 1) == '"') {
          buf[buf_len++] = '"';
          ++p;
        } else {
          in_quotes = false;
        }
      } else if (c == '\0') {
        break;
      } else {
        buf[buf_len++] = c;
      }
      continue;
    }

    if (c == '"') {
      in_quotes = true;
      continue;
    }

    if (c == delimiter || c == '\0' || c == '\n') {
      buf[buf_len] = '\0';
      if (count >= capacity) {
        capacity *= 2;
        char **new_fields = realloc(fields, sizeof(char *) * capacity);
        if (new_fields == NULL) {
          free(buf);
          free(fields);
          return false;
        }
        fields = new_fields;
      }
      fields[count] = strdup(buf);
      if (fields[count] == NULL) {
        free(buf);
        free(fields);
        return false;
      }
      count++;
      buf_len = 0;

      if (c == '\0' || c == '\n') break;
    } else {
      buf[buf_len++] = c;
    }
  }

  free(buf);
  out_row->fields = fields;
  out_row->count = count;
  return true;
}

static bool fields_need_quoting(const char *field, const char delimiter) {
  for (const char *p = field; *p != '\0'; ++p) {
    if (*p == delimiter || *p == '"' || *p == '\n' || *p == '\r') {
      return true;
    }
  }
  return false;
}

static bool write_field(FILE *f, const char *field, const char delimiter) {
  if (!fields_need_quoting(field, delimiter)) {
    if (fputs(field, f) == EOF) return false;
    return true;
  }

  if (fputc('"', f) == EOF) return false;

  for (const char *p = field; *p != '\0'; ++p) {
    if (*p == '"') {
      if (fputs("\"\"", f) == EOF) {
        return false;
      }
    } else {
      if (fputc(*p, f) == EOF) return false;
    }
  }

  if (fputc('"', f) == EOF) return false;

  return true;
}

int parse_from_file(Parser *p, const char *path, const char delimiter) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "Unable to open file: %s\n", path);
    return -1;
  }

  if (p == NULL) {
    fclose(f);
    return -1;
  }

  char *line = NULL;
  size_t line_cap = 0;
  ssize_t line_len;

  while ((line_len = getline(&line, &line_cap, f)) != -1) {
    // Strip '\r' and/or '\n'
    while (line_len > 0 &&
           (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
      line[--line_len] = '\0';
    }
    CsvRow row;
    if (!parse_line(line, delimiter, &row)) {
      fprintf(stderr, "Failed to parse line %zu\n", p->count + 1);
      free(line);
      fclose(f);
      return -1;
    }

    CsvRow *new_rows = realloc(p->rows, sizeof(CsvRow) * (p->count + 1));
    if (new_rows == NULL) {
      for (size_t i = 0; i < p->count; ++i) {
        free(row.fields[i]);
        free(line);
        fclose(f);
        return -1;
      }
    }
    p->rows = new_rows;
    p->rows[p->count++] = row;
  }

  free(line);
  fclose(f);

  return 0;
}

int write_to_file(Parser *p, const char *path, const char delimiter) {
  if (p == NULL || path == NULL) return -1;

  FILE *f = fopen(path, "w");
  if (f == NULL) {
    fprintf(stderr, "Unable to open file for writing: %s\n", path);
    return -1;
  }

  for (size_t r = 0; r < p->count; ++r) {
    CsvRow *row = &p->rows[r];

    for (size_t c = 0; c < row->count; ++c) {
      if (!write_field(f, row->fields[c], delimiter)) {
        fprintf(stderr, "Write error at row %zu, field %zu\n", r, c);
        fclose(f);
        return -1;
      }

      if (c + 1 < row->count) {
        if (fputc(delimiter, f) == EOF) {
          fclose(f);
          return -1;
        }
      }
    }

    if (fputc('\n', f) == EOF) {
      fclose(f);
      return -1;
    }
  }

  if (fclose(f) != 0) {
    fprintf(stderr, "Error closing file: %s\n", path);
    return -1;
  }

  return 0;
}

void parser_free(Parser *p) {
  if (p == NULL) return;

  for (size_t r = 0; r < p->count; ++r) {
    for (size_t f = 0; f < p->rows[r].count; ++f) {
      free(p->rows[r].fields[f]);
    }
    free(p->rows[r].fields);
  }

  free(p->rows);
  free(p);
}
