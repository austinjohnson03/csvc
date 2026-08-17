#ifndef CSV_CSV_
#define CSV_CSV_H

#include <stdlib.h>

typedef struct CsvRow {
  char **fields;
  size_t count;
} CsvRow;

typedef struct Parser {
  CsvRow *rows;
  size_t count;
} Parser;

Parser *parser_init(void);
int add_data_to_parser(Parser *p, char **fields, size_t field_count);
int parse_from_file(Parser *p, const char *path, const char delimiter);
int write_to_file(Parser *p, const char *path, const char delimiter);
void parser_free(Parser *p);

#endif  // CSV_CSV_H
