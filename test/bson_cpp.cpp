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

#include <cstdlib>

#include <iostream>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "bson.hpp"

static char const* const message1 =
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
  uint32_t message_size = bson_get_size(message1, NULL);

  std::unique_ptr<char[]> message(new char[message_size]);
  memcpy(message.get(), message1, message_size);
  {
    bson::Object obj = bson::decode(message.get());

    {
      auto it = obj.find("dest");
      ASSERT_NE(it, obj.end());

      bson::Variant const& var = it->second;
      ASSERT_EQ(var.getType(), BSON_STRING);
      EXPECT_STREQ(var.asString(), "cloud");
    }

    {
      auto it = obj.find("value");
      ASSERT_NE(it, obj.end());

      bson::Variant const& var = it->second;
      ASSERT_EQ(var.getType(), BSON_OBJECT);

      bson::Object const& value = var.asObject();
      {
        auto sub_it = value.find("test");
        ASSERT_NE(sub_it, value.end());

        bson::Variant const& sub_var = sub_it->second;
        EXPECT_EQ(sub_var.getType(), BSON_ARRAY);

        bson::Object const& sub_value = sub_var.asArray();
        {
          auto sub_sub_it = sub_value.find(0);
          ASSERT_NE(sub_sub_it, sub_value.end());

          bson::Variant const& sub_sub_var = sub_sub_it->second;
          EXPECT_EQ(sub_sub_var.getType(), BSON_INT32);
          EXPECT_EQ(sub_sub_var.asInt32(), (int32_t) 0x0c);
        }

        {
          auto sub_sub_it = sub_value.find(1);
          ASSERT_NE(sub_sub_it, sub_value.end());

          bson::Variant const& sub_sub_var = sub_sub_it->second;
          EXPECT_EQ(sub_sub_var.getType(), BSON_INT32);
          EXPECT_EQ(sub_sub_var.asInt32(), (int32_t) 0x17);
        }

        {
          auto sub_sub_it = sub_value.find(2);
          ASSERT_NE(sub_sub_it, sub_value.end());

          bson::Variant const& sub_sub_var = sub_sub_it->second;
          EXPECT_EQ(sub_sub_var.getType(), BSON_INT64);
          EXPECT_EQ(sub_sub_var.asInt64(), (int64_t) 0xefcdab8967452301);
        }
      }
    }
  }

  // Ensure parsing has not modified data
  EXPECT_EQ(memcmp(message1, message.get(), message_size), 0);
}

TEST(bson, encode) {
  char const* message = message1;

  bson::Object obj = bson::decode(message);

  uint32_t size = bson_get_size(message, NULL);

  uint32_t encoded_len = bson::encode_len(obj);
  EXPECT_EQ(encoded_len, size);

  auto encoded = bson::encode(obj);
  EXPECT_EQ(memcmp(encoded.data(), message, size), 0);
}

TEST(Variant, can_copy_binary) {
  bson::Variant var;
  var = bson::Binary(BSON_BINARY_BINARY);
  EXPECT_EQ(var.getType(), BSON_BINARY);

  bson::Variant cpy1(var);
  EXPECT_EQ(cpy1.getType(), BSON_BINARY);

  bson::Variant cpy2 = var;
  EXPECT_EQ(cpy2.getType(), BSON_BINARY);
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
  std::unique_ptr<char[]> message(new char[buffer_size]);
  fread(message.get(), 1, buffer_size, f);
  fclose(f);

  bson::Object const obj = bson::decode(message.get());
  //std::cout << obj << std::endl;

  {
    ASSERT_NE(obj.find("payload"), obj.end());
    ASSERT_EQ(obj["payload"].getType(), BSON_OBJECT);
    bson::Object const& payload = obj["payload"].asObject();

    ASSERT_NE(payload.find("map"), payload.end());
    ASSERT_EQ(payload["map"].getType(), BSON_OBJECT);
    bson::Object const& map = payload["map"].asObject();

    ASSERT_NE(map.find("rawMap"), map.end());
    ASSERT_EQ(map["rawMap"].getType(), BSON_BINARY);
  }

  uint32_t size = bson_get_size(message.get(), NULL);

  uint32_t encoded_len = bson::encode_len(obj);
  EXPECT_EQ(encoded_len, size);

  auto encoded = bson::encode(obj);
  EXPECT_EQ(memcmp(encoded.data(), message.get(), size), 0);
}

int
main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  if (argc < 2) return 1;
  test_filepath = argv[1];

  return RUN_ALL_TESTS();
}
