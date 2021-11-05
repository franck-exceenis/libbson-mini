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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  BSON_END               = 0x00,
  BSON_DOUBLE            = 0x01,
  BSON_STRING            = 0x02,
  BSON_OBJECT            = 0x03,
  BSON_ARRAY             = 0x04,
  BSON_BINARY            = 0x05,
  BSON_UNDEFINED         = 0x06,
  BSON_OBJECTID          = 0x07,
  BSON_BOOLEAN           = 0x08,
  BSON_DATE              = 0x09,
  BSON_NULL              = 0x0a,
  BSON_REGEX             = 0x0b,
  BSON_DBPOINTER         = 0x0c,
  BSON_JAVASCRIPT        = 0x0d,
  BSON_SYMBOL            = 0x0e,
  BSON_SCOPED_JAVASCRIPT = 0x0f,
  BSON_INT32             = 0x10,
  BSON_TIMESTAMP         = 0x11,
  BSON_INT64             = 0x12,
  BSON_DECI128           = 0x13,
} bson_element_t;

typedef enum {
  BSON_BINARY_BINARY       = 0x00,
  BSON_BINARY_FUNCTION     = 0x01,
  BSON_BINARY_OLD_BINARY   = 0x02,
  BSON_BINARY_OLD_UUID     = 0x03,
  BSON_BINARY_UUID         = 0x04,
  BSON_BINARY_MD5          = 0x05,
  BSON_BINARY_USER_DEFINED = 0x80,
} bson_binary_t;

uint32_t
bson_get_size(char const* obj, char const** next);

bson_element_t
bson_get_element_type(char const* elem, char const** next);

char const*
bson_get_element_name(char const* elem, uint32_t* size, char const** next);

void const*
bson_get_element_value_binary(
    char const* elem,
    uint32_t* size,
    bson_binary_t* subtype,
    char const** next);

char const*
bson_get_element_value_string(char const* elem, uint32_t* size, char const** next);

double
bson_get_element_value_double(char const* elem, char const** next);

int32_t
bson_get_element_value_int32(char const* elem, char const** next);

int64_t
bson_get_element_value_int64(char const* elem, char const** next);

bool
bson_get_element_value_bool(char const* elem, char const** next);

uint32_t
bson_get_element_count(char const* object);

void
bson_print(char const* obj, size_t indent, size_t indent_step);

void
bson_dprint(int fd, char const* obj, size_t indent, size_t indent_step);

int
bson_snprint(char* buffer, size_t buffer_size, char const* obj, size_t indent, size_t indent_step);

typedef int (*bson_fnprint_callback_t)(void* data, char const* format, ...);

void
bson_fnprint(
    bson_fnprint_callback_t callback,
    void* callback_data,
    char const* obj,
    size_t indent,
    size_t indent_step);

void
bson_set_size(char* obj, uint32_t size, char** next);

void
bson_set_element_type(char* elem, bson_element_t type, char** next);

void
bson_set_element_name(char* elem, char const* name, uint32_t name_size, char** next);

void
bson_set_element_value_binary(
    char* elem,
    void const* binary,
    uint32_t size,
    bson_binary_t subtype,
    char** next);

void
bson_set_element_value_string(char* elem, char const* str, uint32_t str_size, char** next);

void
bson_set_element_value_double(char* elem, double value, char** next);

void
bson_set_element_value_int32(char* elem, int32_t value, char** next);

void
bson_set_element_value_int64(char* elem, int64_t value, char** next);

void
bson_set_element_value_bool(char* elem, bool value, char** next);

void
bson_next(char const* elem, char const** next);

#ifdef __cplusplus
}
#endif
