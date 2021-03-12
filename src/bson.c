/*
 * This file is part of the libbson-mini distribution
 * (https://gitlab.com/exceenis/lib/libbson-mini or https://github.com/franck-exceenis/libbson-mini.git).
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

#include "./bson.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

uint32_t
bson_get_size(char const* obj, char const** next) {
  uint8_t* uobj   = (uint8_t*) obj;
  uint32_t result = uobj[0];
  result |= uobj[1] << 8;
  result |= uobj[2] << 16;
  result |= uobj[3] << 24;
  if (next) *next = obj + sizeof(uint32_t);
  return result;
}

bson_element_t
bson_get_element_type(char const* elem, char const** next) {
  bson_element_t const result = *elem;
  if (next) *next = elem + 1;
  return result;
}

char const*
bson_get_element_name(char const* elem, uint32_t* size, char const** next) {
  char const* result = elem;
  if (size || next) {
    uint32_t result_size = strlen(elem);
    if (size) *size = result_size;
    if (next) *next = elem + result_size + 1;
  }
  return result;
}

void const*
bson_get_element_value_binary(
    char const* elem,
    uint32_t* size,
    bson_binary_t* subtype,
    char const** next) {
  uint8_t* uelem = (uint8_t*) elem;

  uint32_t result_size = uelem[0];
  result_size |= uelem[1] << 8;
  result_size |= uelem[2] << 16;
  result_size |= uelem[3] << 24;

  uint8_t tp = uelem[4];

  char const* result = elem + sizeof(*size) + sizeof(tp);

  if (size) *size = result_size;
  if (subtype) *subtype = tp;
  if (next) *next = elem + result_size + sizeof(tp) + sizeof(*size);
  return result;
}

char const*
bson_get_element_value_string(char const* elem, uint32_t* size, char const** next) {
  char const* result   = elem + sizeof(*size);
  uint8_t* uelem       = (uint8_t*) elem;
  uint32_t result_size = uelem[0];
  result_size |= uelem[1] << 8;
  result_size |= uelem[2] << 16;
  result_size |= uelem[3] << 24;
  result_size -= 1;
  if (size) *size = result_size;
  if (next) *next = elem + result_size + sizeof(*size) + 1;
  return result;
}

int32_t
bson_get_element_value_int32(char const* elem, char const** next) {
  uint8_t* uelem = (uint8_t*) elem;
  int32_t result = uelem[0];
  result |= uelem[1] << 8;
  result |= uelem[2] << 16;
  result |= uelem[3] << 24;
  if (next) *next = elem + sizeof(result);
  return result;
}

int64_t
bson_get_element_value_int64(char const* elem, char const** next) {
  uint8_t* uelem = (uint8_t*) elem;
  int64_t result = uelem[4];
  result |= uelem[5] << 8;
  result |= uelem[6] << 16;
  result |= uelem[7] << 24;
  result <<= 32;
  result |= uelem[0] << 0;
  result |= uelem[1] << 8;
  result |= uelem[2] << 16;
  result |= uelem[3] << 24;

  if (next) *next = elem + sizeof(result);
  return result;
}

bool
bson_get_element_value_bool(char const* elem, char const** next) {
  bool result = elem[0];
  if (next) *next = elem + sizeof(bool);
  return result;
}

double
bson_get_element_value_double(char const* elem, char const** next) {
  double result = *((double*) elem);
  if (next) *next = elem + sizeof(double);
  return result;
}

uint32_t
bson_get_element_count(char const* object) {
  // Skip object size
  object += sizeof(uint32_t);

  uint32_t count = 0;
  while (1) {
    bson_element_t type = bson_get_element_type(object, NULL);
    if (type == BSON_END) break;

    ++count;
    bson_next(object, &object);
  }

  return count;
}

static void
print_indent(bson_fnprint_callback_t callback, void* callback_data, size_t indent) {
  for (size_t i = 0; i < indent; ++i) callback(callback_data, " ");
}

static void
bson_print_object_or_array(
    bson_fnprint_callback_t callback,
    void* callback_data,
    char const* obj,
    size_t indent,
    size_t indent_step,
    bson_element_t base_type,
    bool indent_first) {
  uint32_t size = bson_get_size(obj, &obj);

  if (indent_first) print_indent(callback, callback_data, indent);
  switch (base_type) {
    case BSON_OBJECT: {
      callback(callback_data, "object(size=%u) {\n", size);
      break;
    }

    case BSON_ARRAY: {
      callback(callback_data, "array(size=%u) [\n", size);
      break;
    }

    default: callback(callback_data, "Not handled: %d\n", (int) base_type); return;
  }

  bson_element_t type = bson_get_element_type(obj, &obj);
  while (type != BSON_END) {
    uint32_t name_size;
    char const* name = bson_get_element_name(obj, &name_size, &obj);
    print_indent(callback, callback_data, indent + indent_step);
    callback(callback_data, "\"%s\": ", name);

    switch (type) {
      case BSON_STRING: {
        char const* value = bson_get_element_value_string(obj, NULL, &obj);
        callback(callback_data, "\"%s\"", value);
        break;
      }

      case BSON_INT32: {
        int32_t value = bson_get_element_value_int32(obj, &obj);
        callback(callback_data, "int32(%ld)", (long) value);
        break;
      }

      case BSON_INT64: {
        int64_t value = bson_get_element_value_int64(obj, &obj);
        callback(callback_data, "int64(%lld)", (long long) value);
        break;
      }

      case BSON_BOOLEAN: {
        bool value = bson_get_element_value_bool(obj, &obj);
        callback(callback_data, "bool(%s)", value ? "true" : "false");
        break;
      }

      case BSON_DOUBLE: {
        double value = bson_get_element_value_double(obj, &obj);
        callback(callback_data, "double(%f)", value);
        break;
      }

      case BSON_BINARY: {
        uint32_t bin_size = 0;
        bson_binary_t subtype;
        char const* binary     = bson_get_element_value_binary(obj, &bin_size, &subtype, &obj);
        uint8_t const* ubinary = (uint8_t const*) binary;
        callback(callback_data, "binary(size=%u,subtype=%d) <", bin_size, (int) subtype);
        static const uint32_t line_size = 32;
        for (uint32_t i = 0; i < bin_size;) {
          callback(callback_data, "\n");
          print_indent(callback, callback_data, indent + 2 * indent_step);
          for (uint32_t len = 0; i < bin_size && len < line_size; ++len, ++i) {
            callback(callback_data, "%02x", ubinary[i]);
          }
        }
        callback(callback_data, "\n");
        print_indent(callback, callback_data, indent + indent_step);
        callback(callback_data, ">");
        break;
      }

      case BSON_OBJECT: {
        bson_print_object_or_array(
            callback, callback_data, obj, indent + indent_step, indent_step, BSON_OBJECT, false);
        uint32_t sizeCurrent = bson_get_size(obj, NULL);
        obj += sizeCurrent;
        break;
      }

      case BSON_ARRAY: {
        bson_print_object_or_array(
            callback, callback_data, obj, indent + indent_step, indent_step, BSON_ARRAY, false);
        uint32_t sizeCurrent = bson_get_size(obj, NULL);
        obj += sizeCurrent;
        break;
      }

      default: callback(callback_data, "Not handled: %d\n", (int) type); return;
    }

    type = bson_get_element_type(obj, &obj);
    if (type != BSON_END)
      callback(callback_data, ",\n");
    else
      callback(callback_data, "\n");
  }

  print_indent(callback, callback_data, indent);
  switch (base_type) {
    case BSON_OBJECT: {
      callback(callback_data, "}");
      break;
    }

    case BSON_ARRAY: {
      callback(callback_data, "]");
      break;
    }

    default: callback(callback_data, "Not handled: %d\n", (int) base_type); return;
  }
}

static int
dprint_callback(void* data, char const* format, ...) {
  va_list list;
  va_start(list, format);
  int ret = vdprintf(*(int*) data, format, list);
  va_end(list);
  return ret;
}

void
bson_print(char const* obj, size_t indent, size_t indent_step) {
  int fd = STDOUT_FILENO;
  bson_print_object_or_array(dprint_callback, &fd, obj, indent, indent_step, BSON_OBJECT, true);
}

void
bson_dprint(int fd, char const* obj, size_t indent, size_t indent_step) {
  bson_print_object_or_array(dprint_callback, &fd, obj, indent, indent_step, BSON_OBJECT, true);
}

typedef struct {
  char* buffer;
  size_t size;
} snprint_callback_data_t;

static int
snprint_callback(void* data_, char const* format, ...) {
  snprint_callback_data_t* data = (snprint_callback_data_t*) data_;

  va_list list;
  va_start(list, format);
  int ret = vsnprintf(data->buffer, data->size, format, list);
  if (ret > 0) {
    data->buffer += ret;
    data->size -= ret;
  }
  va_end(list);

  return ret;
}

int
bson_snprint(char* buffer, size_t buffer_size, char const* obj, size_t indent, size_t indent_step) {
  snprint_callback_data_t data = {
      .buffer = buffer,
      .size   = buffer_size,
  };

  bson_print_object_or_array(snprint_callback, &data, obj, indent, indent_step, BSON_OBJECT, true);
  return data.buffer - buffer;
}

void
bson_fnprint(
    bson_fnprint_callback_t callback,
    void* callback_data,
    char const* obj,
    size_t indent,
    size_t indent_step) {
  bson_print_object_or_array(callback, callback_data, obj, indent, indent_step, BSON_OBJECT, true);
}

void
bson_set_size(char* obj, uint32_t size, char** next) {
  uint8_t* uobj = (uint8_t*) obj;

  uobj[0] = size >> 0;
  uobj[1] = size >> 8;
  uobj[2] = size >> 16;
  uobj[3] = size >> 24;

  if (next) *next = obj + sizeof(uint32_t);
}

void
bson_set_element_type(char* elem, bson_element_t type, char** next) {
  *elem = type;
  if (next) *next = elem + 1;
}

void
bson_set_element_name(char* elem, char const* name, uint32_t name_size, char** next) {
  memcpy(elem, name, name_size);
  elem[name_size] = 0;
  if (next) *next = elem + name_size + 1;
}

void
bson_set_element_value_string(char* elem, char const* str, uint32_t str_size, char** next) {
  memcpy(elem + sizeof(uint32_t), str, str_size + 1);

  uint8_t* uelem = (uint8_t*) elem;
  uint32_t size  = str_size + 1;

  uelem[0] = size >> 0;
  uelem[1] = size >> 8;
  uelem[2] = size >> 16;
  uelem[3] = size >> 24;

  if (next) *next = elem + size + sizeof(str_size);
}

void
bson_set_element_value_int32(char* elem, int32_t value, char** next) {
  elem[0] = value >> 0;
  elem[1] = value >> 8;
  elem[2] = value >> 16;
  elem[3] = value >> 24;

  if (next) *next = elem + sizeof(value);
}

void
bson_set_element_value_int64(char* elem, int64_t value, char** next) {
  elem[0] = value >> 0;
  elem[1] = value >> 8;
  elem[2] = value >> 16;
  elem[3] = value >> 24;
  elem[4] = value >> 32;
  elem[5] = value >> 40;
  elem[6] = value >> 48;
  elem[7] = value >> 56;

  if (next) *next = elem + sizeof(value);
}

void
bson_set_element_value_double(char* elem, double value, char** next) {
  memcpy(elem, &value, sizeof(value));
  if (next) *next = elem + sizeof(value);
}

void
bson_set_element_value_binary(
    char* elem,
    void const* binary,
    uint32_t size,
    bson_binary_t subtype,
    char** next) {

  uint8_t* uelem = (uint8_t*) elem;

  uelem[0] = size >> 0;
  uelem[1] = size >> 8;
  uelem[2] = size >> 16;
  uelem[3] = size >> 24;

  uelem[4] = subtype;
  memcpy(elem + sizeof(uelem[4]) + sizeof(size), binary, size);

  if (next) *next = elem + size + sizeof(size) + sizeof(uelem[4]);
}

void
bson_set_element_value_bool(char* elem, bool value, char** next) {
  *elem = value;
  if (next) *next = elem + sizeof(value);
}

void
bson_next(char const* elem, char const** next) {
  bson_element_t type = bson_get_element_type(elem, &elem);
  bson_get_element_name(elem, NULL, &elem);
  switch (type) {
    case BSON_END: break;

    case BSON_OBJECT:
    case BSON_ARRAY: {
      uint32_t size = bson_get_size(elem, NULL);
      elem += size;
      break;
    }

    case BSON_STRING: {
      bson_get_element_value_string(elem, NULL, &elem);
      break;
    }

    case BSON_DOUBLE: {
      bson_get_element_value_double(elem, &elem);
      break;
    }

    case BSON_BOOLEAN: {
      bson_get_element_value_bool(elem, &elem);
      break;
    }

    case BSON_INT64: {
      bson_get_element_value_int64(elem, &elem);
      break;
    }

    case BSON_INT32: {
      bson_get_element_value_int32(elem, &elem);
      break;
    }

    case BSON_BINARY: {
      bson_get_element_value_binary(elem, NULL, NULL, &elem);
      break;
    }

    case BSON_UNDEFINED:
    case BSON_OBJECTID:
    case BSON_DATE:
    case BSON_NULL:
    case BSON_REGEX:
    case BSON_DBPOINTER:
    case BSON_JAVASCRIPT:
    case BSON_SYMBOL:
    case BSON_SCOPED_JAVASCRIPT:
    case BSON_TIMESTAMP:
    case BSON_DECI128: {
      // TODO
      break;
    }
  }

  if (next) *next = elem;
}
