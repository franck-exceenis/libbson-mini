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

#include <cstdlib>

#include <iostream>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "bson.h"

static char const* message1 =
    "\x45\x00\x00\x00"
    "\x02" // <string>
    "dest"
    "\x00"
    "\x06\x00\x00\x00"
    "cloud"
    "\x00"
    "\x03" // <object> {
    "value"
    "\x00"
    "\x29\x00\x00\x00"
    "\x04" // <array> [
    "test"
    "\x00"
    "\x1e\x00\x00\x00"
    "\x10" // <int32>
    "0"
    "\x00"
    "\x0c\x00\x00\x00"
    "\x10" // <int32>
    "1"
    "\x00"
    "\x17\x00\x00\x00"
    "\x12" // <int64>
    "2"
    "\x00"
    "\x01\x23\x45\x67\x89\xab\xcd\xef"
    "\x00" // ]
    "\x00" // }
    "\x00";

TEST(bson, decode) {
  char const* message = message1;

  // Size
  uint32_t size = bson_get_size(message, &message);
  EXPECT_EQ(size, 0x45u);

  // 1st element
  {
    bson_element_t type = bson_get_element_type(message, &message);
    EXPECT_EQ(type, BSON_STRING);

    uint32_t name_size;
    char const* name = bson_get_element_name(message, &name_size, &message);
    EXPECT_STREQ(name, "dest");
    EXPECT_EQ(name_size, 4u);

    uint32_t value_size;
    char const* value = bson_get_element_value_string(message, &value_size, &message);
    EXPECT_STREQ(value, "cloud");
    EXPECT_EQ(value_size, 5u);
  }

  // 2nd element
  {
    bson_element_t type = bson_get_element_type(message, &message);
    EXPECT_EQ(type, BSON_OBJECT);

    uint32_t name_size;
    char const* name = bson_get_element_name(message, &name_size, &message);
    EXPECT_STREQ(name, "value");

    uint32_t value_size = bson_get_size(message, &message);
    EXPECT_EQ(value_size, 0x29u);

    // Subelement
    {
      bson_element_t subelem_type = bson_get_element_type(message, &message);
      EXPECT_EQ(subelem_type, BSON_ARRAY);

      uint32_t subname_size;
      char const* subname = bson_get_element_name(message, &subname_size, &message);
      EXPECT_STREQ(subname, "test");
      EXPECT_EQ(subname_size, 4u);

      uint32_t subvalue_size = bson_get_size(message, &message);
      EXPECT_EQ(subvalue_size, 0x1eu);

      // 1st subsubelement
      {
        bson_element_t subsubelem_type = bson_get_element_type(message, &message);
        EXPECT_EQ(subsubelem_type, BSON_INT32);

        uint32_t subsubname_size;
        char const* subsubname = bson_get_element_name(message, &subsubname_size, &message);
        EXPECT_STREQ(subsubname, "0");
        EXPECT_EQ(subsubname_size, 1u);

        int32_t subsubvalue = bson_get_element_value_int32(message, &message);
        EXPECT_EQ(subsubvalue, 0x0c);
      }

      // 2nd subsubelement
      {
        bson_element_t subsubelem_type = bson_get_element_type(message, &message);
        EXPECT_EQ(subsubelem_type, BSON_INT32);

        uint32_t subsubname_size;
        char const* subsubname = bson_get_element_name(message, &subsubname_size, &message);
        EXPECT_STREQ(subsubname, "1");
        EXPECT_EQ(subsubname_size, 1u);

        int32_t subsubvalue = bson_get_element_value_int32(message, &message);
        EXPECT_EQ(subsubvalue, 0x17);
      }

      // 3rd subsubelement
      {
        bson_element_t subsubelem_type = bson_get_element_type(message, &message);
        EXPECT_EQ(subsubelem_type, BSON_INT64);

        uint32_t subsubname_size;
        char const* subsubname = bson_get_element_name(message, &subsubname_size, &message);
        EXPECT_STREQ(subsubname, "2");
        EXPECT_EQ(subsubname_size, 1u);

        int64_t subsubvalue = bson_get_element_value_int64(message, &message);
        int64_t expected    = 0xefcdab8967452301;
        EXPECT_EQ(subsubvalue, expected);
      }

      // End
      {
        bson_element_t subsubelem_type = bson_get_element_type(message, &message);
        EXPECT_EQ(subsubelem_type, BSON_END);
      }
    }

    // End
    {
      bson_element_t subelem_type = bson_get_element_type(message, &message);
      EXPECT_EQ(subelem_type, BSON_END);
    }
  }

  // End
  {
    bson_element_t elem_type = bson_get_element_type(message, &message);
    EXPECT_EQ(elem_type, BSON_END);
  }

  EXPECT_EQ(message, message1 + size);

  bson_print(message1, 0, 2);
  printf("\n");
}

TEST(bson, encode) {
  uint32_t buffer_size = 1024 * 1024;
  char* buffer         = (char*) malloc(buffer_size);
  char* current        = buffer;

  char* size_ptr = current;
  bson_set_size(current, 0, &current);

  // 1st element
  {
    bson_set_element_type(current, BSON_STRING, &current);
    bson_set_element_name(current, "toto", strlen("toto"), &current);
    bson_set_element_value_string(
        current, "something to write", strlen("something to write"), &current);
  }

  // 2nd element
  {
    bson_set_element_type(current, BSON_INT32, &current);
    bson_set_element_name(current, "tutu-the-test", strlen("tutu-the-test"), &current);
    bson_set_element_value_int32(current, -123456, &current);
  }

  // 3rd element
  {
    bson_set_element_type(current, BSON_DOUBLE, &current);
    bson_set_element_name(current, "€2F", strlen("€2F"), &current);
    bson_set_element_value_double(current, 6.55957, &current);
  }

  // 4th element
  {
    bson_set_element_type(current, BSON_ARRAY, &current);
    bson_set_element_name(current, "an array[]", strlen("an array[]"), &current);
    char* array_size_ptr = current;
    bson_set_size(current, 0, &current);

    // 1st subelement
    {
      bson_set_element_type(current, BSON_BOOLEAN, &current);
      bson_set_element_name(current, "0", strlen("0"), &current);
      bson_set_element_value_bool(current, false, &current);
    }

    // 2nd subelement
    {
      char binary[4] = {'0', '\x34', '\xef', '\x00'};
      bson_set_element_type(current, BSON_BINARY, &current);
      bson_set_element_name(current, "1", strlen("1"), &current);
      bson_set_element_value_binary(current, binary, sizeof(binary), BSON_BINARY_BINARY, &current);
    }

    bson_set_element_type(current, BSON_END, &current);
    bson_set_size(size_ptr, current - array_size_ptr, NULL);
  }

  bson_set_element_type(current, BSON_END, &current);
  bson_set_size(size_ptr, current - size_ptr, NULL);

  bson_print(buffer, 0, 2);
  printf("\n");

  free(buffer);
}

char const* test_filepath = NULL;

TEST(bson, decode_large) {
  FILE* f = fopen(test_filepath, "rb");
  ASSERT_TRUE(f);

  fseek(f, 0L, SEEK_END);
  size_t file_size = ftell(f);
  fseek(f, 0L, SEEK_SET);

  ASSERT_GT(file_size, 0u);

  size_t buffer_size = file_size + 1;
  char* buffer       = (char*) malloc(buffer_size);
  fread(buffer, 1, buffer_size, f);
  fclose(f);

  char const* it = buffer;
  uint32_t size  = bson_get_size(it, &it);

  EXPECT_EQ(size, file_size);

  std::set<std::string> keys;
  bson_element_t type = bson_get_element_type(it, &it);
  int nested          = 1;
  while (true) {
    if (type == BSON_END) {
      --nested;
      if (nested > 0) {
        type = bson_get_element_type(it, &it);
        continue;
      } else {
        break;
      }
    }

    char const* key = bson_get_element_name(it, NULL, &it);
    keys.insert(std::string(key));

    switch (type) {
      case BSON_INT32: {
        bson_get_element_value_int32(it, &it);
        break;
      }

      case BSON_INT64: {
        bson_get_element_value_int64(it, &it);
        break;
      }

      case BSON_DOUBLE: {
        bson_get_element_value_double(it, &it);
        break;
      }

      case BSON_STRING: {
        bson_get_element_value_string(it, NULL, &it);
        break;
      }

      case BSON_ARRAY:
      case BSON_OBJECT: {
        bson_get_size(it, &it);
        ++nested;
        break;
      }

      case BSON_BINARY: {
        uint32_t bin_size;
        bson_binary_t bintype;
        bson_get_element_value_binary(it, &bin_size, &bintype, &it);
        break;
      }

      case BSON_BOOLEAN: {
        bson_get_element_value_bool(it, &it);
        break;
      }

      default: {
        std::cerr << "Unhandled: " << (int) type << std::endl;
        ASSERT_TRUE(false);
        break;
      }
    }

    type = bson_get_element_type(it, &it);
  }

  std::set<std::string> expected = {
      "0",
      "autoFocus",
      "autoKeystone",
      "beautifulMap",
      "brightness",
      "cellSize",
      "contrast",
      "displayMode",
      "errorTheta",
      "errorX",
      "errorY",
      "flip",
      "focus",
      "height",
      "id",
      "isLost",
      "keystone",
      "ledsOn",
      "map",
      "name",
      "orientation",
      "originTh",
      "originX",
      "originY",
      "payload",
      "pose",
      "projectionRatio",
      "rawMap",
      "roomId",
      "rooms",
      "spots",
      "stamp",
      "theta",
      "type",
      "version",
      "width",
      "x",
      "y",
      "zoom",
  };

  EXPECT_EQ(expected, keys);

  free(buffer);
}

int
main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (argc < 2) return 1;
  test_filepath = argv[1];

  return RUN_ALL_TESTS();
}
