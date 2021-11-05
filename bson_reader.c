/*
 * This file is part of the libbson-mini distribution
 * (https://gitlab.com/exceenis/lib/libbson-mini or https://github.com/franck-exceenis/libbson-mini).
 * Copyright (c) 2020 Franck Duriez
 *
 * Free Licensing:
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, version 3.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial Licensing:
 *   You should have received a copy of the commercial licensing condition
 *   along with this program. If not, contact us at <contact@exceenis.com>.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./bson.h"

int
parse_file(char const* filepath) {
  FILE* f = fopen(filepath, "rb");

  fseek(f, 0L, SEEK_END);
  size_t file_size = ftell(f);
  fseek(f, 0L, SEEK_SET);

  size_t buffer_size = file_size + 1;
  char* buffer       = (char*) malloc(buffer_size);
  fread(buffer, 1, buffer_size, f);
  fclose(f);

  bson_print(buffer, 0, 2);

  free(buffer);
  return 0;
}

int
parse_stdin(void) {
  uint32_t size         = 0;
  char header_buffer[4] = {0};
  {
    ssize_t readBytes = read(STDIN_FILENO, header_buffer, sizeof(header_buffer));
    if (readBytes != sizeof(header_buffer)) {
      fprintf(stderr, "Unable to read bson size");
      return 1;
    }

    size = bson_get_size(header_buffer, NULL);

    if (size < 5) {
      fprintf(stderr, "Invalid bson size: %lu", (long unsigned) size);
      return 2;
    }
  }

  char* buffer = (char*) malloc(size);
  memcpy(buffer, header_buffer, sizeof(header_buffer));
  {
    ssize_t readBytes =
        read(STDIN_FILENO, buffer + sizeof(header_buffer), size - sizeof(header_buffer));
    if (readBytes != size - sizeof(header_buffer)) {
      fprintf(
          stderr,
          "Unable to read bson body expect %lu bytes but got only %lu",
          (long unsigned) size,
          (long unsigned) (readBytes + sizeof(header_buffer)));
      return 3;
    }

    bson_print(buffer, 0, 2);
  }

  free(buffer);
}

int
main(int argc, char** argv) {
  if (isatty(fileno(stdin))) {
    if (argc < 2) {
      printf("usage: %s <file.bson>\n", argv[0]);
      return 1;
    }
    return parse_file(argv[1]);
  }

  return parse_stdin();
}
