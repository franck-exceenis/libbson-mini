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

#include <cstring>

#include <map>
#include <ostream>
#include <string>
#include <vector>

#include <bson.h>

namespace bson {

class Variant;

class Binary {
 public:
  explicit inline Binary(void)
      : _type(BSON_BINARY_BINARY)
      , _value() {}

  explicit inline Binary(bson_binary_t type)
      : _type(type)
      , _value() {}

  inline bson_binary_t
  getType(void) const {
    return _type;
  }

  inline std::vector<uint8_t> const&
  get(void) const {
    return _value;
  }

  inline std::vector<uint8_t>&
  get(void) {
    return _value;
  }

  template<typename _Iterator>
  inline void
  set(_Iterator beg, _Iterator end) {
    _value = std::vector<uint8_t>(beg, end);
  }

  inline std::size_t
  length(void) const {
    return _value.size();
  }

 private:
  bson_binary_t _type;
  std::vector<uint8_t> _value;
};

class Object {
  typedef std::vector<std::pair<std::string const, Variant>> data;

 public:
  typedef data::iterator iterator;
  typedef data::const_iterator const_iterator;

  Object(void);

  Variant&
  operator[](std::string const& key);

  Variant const&
  operator[](std::string const& key) const;

  Variant&
  operator[](uint32_t key);

  Variant const&
  operator[](uint32_t key) const;

  inline iterator
  begin(void) {
    return _data.begin();
  }

  inline const_iterator
  begin(void) const {
    return _data.begin();
  }

  inline iterator
  end(void) {
    return _data.end();
  }

  inline const_iterator
  end(void) const {
    return _data.end();
  }

  iterator
  find(std::string const& key);

  const_iterator
  find(std::string const& key) const;

  iterator
  find(uint32_t key);

  const_iterator
  find(uint32_t key) const;

 private:
  data _data;
};

class Variant {
 public:
  Variant(void);

  explicit Variant(bson_element_t type);

  Variant(Variant const& rhs);
  Variant(Variant&& rhs) = delete;

  ~Variant(void);

  inline bson_element_t
  getType(void) const {
    return _type;
  }

  inline double
  asDouble(void) const {
    return _double;
  }

  inline bool
  asBoolean(void) const {
    return _boolean;
  }

  inline int32_t
  asInt32(void) const {
    return _int32;
  }

  inline int64_t
  asInt64(void) const {
    return _int64;
  }

  inline char const*
  asString(void) const {
    return _string;
  }

  inline Object const&
  asObject(void) const {
    return *_object;
  }

  inline Object&
  asObject(void) {
    return *_object;
  }

  inline Object const&
  asArray(void) const {
    return *_object;
  }

  inline Object&
  asArray(void) {
    return *_object;
  }

  inline Binary const&
  asBinary(void) const {
    return *_binary;
  }

  inline Binary&
  asBinary(void) {
    return *_binary;
  }

  inline Variant&
  operator[](std::string const& key) {
    return (*_object)[key];
  }

  inline Variant const&
  operator[](std::string const& key) const {
    return (*_object)[key];
  }

  inline Variant&
  operator[](uint32_t key) {
    return (*_object)[key];
  }

  inline Variant const&
  operator[](uint32_t key) const {
    return (*_object)[key];
  }

  inline Variant&
  operator=(Variant const& rhs) {
    this->~Variant();
    new (this) Variant(rhs);
    return *this;
  }

  inline Variant&
  operator=(double value) {
    if (_type != BSON_DOUBLE) {
      _free();
      _type = BSON_DOUBLE;
    }

    _double = value;
    return *this;
  }

  inline Variant&
  operator=(bool value) {
    if (_type != BSON_BOOLEAN) {
      _free();
      _type = BSON_BOOLEAN;
    }

    _boolean = value;
    return *this;
  }

  inline Variant&
  operator=(int32_t value) {
    if (_type != BSON_INT32) {
      _free();
      _type = BSON_INT32;
    }

    _int32 = value;
    return *this;
  }

  inline Variant&
  operator=(int64_t value) {
    if (_type != BSON_INT64) {
      _free();
      _type = BSON_INT64;
    }

    _int64 = value;
    return *this;
  }

  inline Variant&
  operator=(char const* value) {
    _free();
    _type = BSON_STRING;

    size_t len = strlen(value);
    _string    = new char[len + 1];
    memcpy(_string, value, len + 1);
    return *this;
  }

  inline Variant&
  operator=(std::string const& value) {
    return operator=(value.c_str());
  }

  inline Variant&
  operator=(Binary const& value) {
    if (_type != BSON_BINARY) {
      _free();
      _type = BSON_BINARY;
    }

    _binary = new Binary(value);
    return *this;
  }

  inline Variant&
  setArray(Object&& value) {
    if (_type != BSON_ARRAY || _type != BSON_OBJECT) _free();

    _type   = BSON_ARRAY;
    _object = new Object(std::move(value));
    return *this;
  }

  inline Variant&
  setArray(Object const& value) {
    return setArray(Object(value));
  }

  inline Variant&
  setObject(Object&& value) {
    if (_type != BSON_ARRAY || _type != BSON_OBJECT) _free();

    _type   = BSON_OBJECT;
    _object = new Object(std::move(value));
    return *this;
  }

  inline Variant&
  setObject(Object const& value) {
    return setObject(Object(value));
  }

 private:
  void
  _free(void);

  bson_element_t _type;

  union {
    double _double;
    bool _boolean;
    int32_t _int32;
    int64_t _int64;

    char* _string;
    Object* _object;
    Binary* _binary;
  };
};

Object
decode(char const* input);

uint32_t
encode_len(Object const& obj);

void
encode(Object const& obj, char* output, char** next = nullptr);

std::vector<char>
encode(Object const& obj);

void
print(std::ostream& os, bson::Object const& obj, size_t indent, size_t indent_step);

} // namespace bson

namespace std {

ostream&
operator<<(ostream& os, bson::Object const& obj);

} // namespace std
