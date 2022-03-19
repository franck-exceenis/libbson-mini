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

#include "./bson.hpp"

#include <cassert>

// Object
namespace bson {

Object::Object(void) {}

Variant&
Object::operator[](std::string const& key) {
  for (auto& value : _data) {
    if (value.first == key) return value.second;
  }

  _data.push_back(std::make_pair(key, Variant()));
  return _data.rbegin()->second;
}

Variant const&
Object::operator[](std::string const& key) const {
  for (auto& value : _data) {
    if (value.first == key) return value.second;
  }

  static Variant const end;
  return end;
}

Variant&
Object::operator[](uint32_t key) {
  return operator[](std::to_string(key));
}

Variant const&
Object::operator[](uint32_t key) const {
  return operator[](std::to_string(key));
}

Object::iterator
Object::find(std::string const& key) {
  for (auto it = _data.begin(); it != _data.end(); ++it) {
    if (it->first == key) return it;
  }

  return _data.end();
}

Object::const_iterator
Object::find(std::string const& key) const {
  for (auto it = _data.begin(); it != _data.end(); ++it) {
    if (it->first == key) return it;
  }

  return _data.end();
}

Object::iterator
Object::find(uint32_t key) {
  return find(std::to_string(key));
}

Object::const_iterator
Object::find(uint32_t key) const {
  return find(std::to_string(key));
}

} // namespace bson

// Variant
namespace bson {

Variant::Variant(void)
    : _type(BSON_END) {}

Variant::Variant(bson_element_t type)
    : _type(type) {
  switch (_type) {
    case BSON_END: break;

    case BSON_DOUBLE: break;
    case BSON_BOOLEAN: break;
    case BSON_INT32: break;
    case BSON_INT64: break;
    case BSON_STRING: _string = new char[1]{0}; break;
    case BSON_BINARY: _binary = new Binary(); break;

    case BSON_OBJECT:
    case BSON_ARRAY: {
      _object = new Object();
      break;
    }

    // Not yet supported
    case BSON_UNDEFINED: break;
    case BSON_OBJECTID: break;
    case BSON_DATE: break;
    case BSON_NULL: break;
    case BSON_REGEX: break;
    case BSON_DBPOINTER: break;
    case BSON_JAVASCRIPT: break;
    case BSON_SYMBOL: break;
    case BSON_SCOPED_JAVASCRIPT: break;
    case BSON_TIMESTAMP: break;
    case BSON_DECI128: break;
  }
}

Variant::Variant(Variant const& rhs)
    : _type(BSON_END) {
  switch (rhs._type) {
    case BSON_END: break;

    case BSON_DOUBLE: *this = rhs._double; break;
    case BSON_BOOLEAN: *this = rhs._boolean; break;
    case BSON_INT32: *this = rhs._int32; break;
    case BSON_INT64: *this = rhs._int64; break;
    case BSON_STRING: *this = rhs._string; break;

    case BSON_BINARY: {
      _type   = BSON_BINARY;
      _binary = new Binary(*rhs._binary);
      break;
    }

    case BSON_ARRAY:
    case BSON_OBJECT: {
      _type   = rhs._type;
      _object = new Object(*rhs._object);
      break;
    }

    // Not yet supported
    case BSON_UNDEFINED: break;
    case BSON_OBJECTID: break;
    case BSON_DATE: break;
    case BSON_NULL: break;
    case BSON_REGEX: break;
    case BSON_DBPOINTER: break;
    case BSON_JAVASCRIPT: break;
    case BSON_SYMBOL: break;
    case BSON_SCOPED_JAVASCRIPT: break;
    case BSON_TIMESTAMP: break;
    case BSON_DECI128: break;
  }
}

Variant::~Variant(void) { _free(); }

void
Variant::_free(void) {
  switch (_type) {
    case BSON_END: break;

    case BSON_DOUBLE: break;
    case BSON_BOOLEAN: break;
    case BSON_INT32: break;
    case BSON_INT64: break;
    case BSON_STRING: delete[] _string; break;
    case BSON_BINARY: delete _binary; break;

    case BSON_OBJECT:
    case BSON_ARRAY: {
      delete _object;
      break;
    }

    // Not yet supported
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
    case BSON_DECI128: assert(false); break;
  }
}

} // namespace bson

namespace bson {

Object
decode(char const* input) {
  Object result;

  char const* obj = input + sizeof(uint32_t);

  bson_element_t type = bson_get_element_type(obj, &obj);
  while (type != BSON_END) {
    uint32_t name_size;
    char const* name = bson_get_element_name(obj, &name_size, &obj);

    switch (type) {
      case BSON_STRING: {
        char const* value = bson_get_element_value_string(obj, NULL, &obj);
        result[name]      = value;
        break;
      }

      case BSON_INT32: {
        int32_t value = bson_get_element_value_int32(obj, &obj);
        result[name]  = value;
        break;
      }

      case BSON_INT64: {
        int64_t value = bson_get_element_value_int64(obj, &obj);
        result[name]  = value;
        break;
      }

      case BSON_BOOLEAN: {
        bool value   = bson_get_element_value_bool(obj, &obj);
        result[name] = value;
        break;
      }

      case BSON_DOUBLE: {
        double value = bson_get_element_value_double(obj, &obj);
        result[name] = value;
        break;
      }

      case BSON_BINARY: {
        uint32_t bin_size = 0;
        bson_binary_t subtype;
        uint8_t const* ubinary =
            (uint8_t const*) bson_get_element_value_binary(obj, &bin_size, &subtype, &obj);

        Binary binary = Binary(subtype);
        binary.set(ubinary, ubinary + bin_size);
        result[name] = binary;
        break;
      }

      case BSON_OBJECT: {
        result[name].setObject(decode(obj));
        uint32_t sizeCurrent = bson_get_size(obj, NULL);
        obj += sizeCurrent;
        break;
      }

      case BSON_ARRAY: {
        result[name].setArray(decode(obj));
        uint32_t sizeCurrent = bson_get_size(obj, NULL);
        obj += sizeCurrent;
        break;
      }

      default: assert(false); return result;
    }

    type = bson_get_element_type(obj, &obj);
  }

  return result;
}

uint32_t
encode_len(Object const& obj) {
  uint32_t result = sizeof(uint32_t);

  for (auto const& value : obj) {
    result += 1; // type
    result += value.first.length() + 1;

    switch (value.second.getType()) {
      case BSON_END: assert(false); break;

      case BSON_DOUBLE: result += sizeof(double); break;
      case BSON_BOOLEAN: result += sizeof(bool); break;
      case BSON_INT32: result += sizeof(int32_t); break;
      case BSON_INT64: result += sizeof(int64_t); break;
      case BSON_STRING: result += sizeof(uint32_t) + strlen(value.second.asString()) + 1; break;
      case BSON_BINARY: {
        result += sizeof(uint8_t) + sizeof(uint32_t) + value.second.asBinary().length();
        break;
      }
      case BSON_OBJECT: result += encode_len(value.second.asArray()); break;
      case BSON_ARRAY: result += encode_len(value.second.asObject()); break;

      // Not yet supported
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
      case BSON_DECI128: assert(false); break;
    }
  }

  result += 1; // BSON_END

  return result;
}

void
encode(Object const& obj, char* output, char** next) {
  char* it = output + sizeof(uint32_t);
  for (auto const& value : obj) {
    bson_set_element_type(it, value.second.getType(), &it);
    bson_set_element_name(it, value.first.c_str(), value.first.length(), &it);

    switch (value.second.getType()) {
      case BSON_END: assert(false); break;

      case BSON_DOUBLE: bson_set_element_value_double(it, value.second.asDouble(), &it); break;
      case BSON_BOOLEAN: bson_set_element_value_bool(it, value.second.asBoolean(), &it); break;
      case BSON_INT32: bson_set_element_value_int32(it, value.second.asInt32(), &it); break;
      case BSON_INT64: bson_set_element_value_int64(it, value.second.asInt64(), &it); break;

      case BSON_STRING: {
        bson_set_element_value_string(
            it, value.second.asString(), strlen(value.second.asString()), &it);
        break;
      }

      case BSON_BINARY: {
        bson_set_element_value_binary(
            it,
            value.second.asBinary().get().data(),
            value.second.asBinary().get().size(),
            value.second.asBinary().getType(),
            &it);
        break;
      }

      case BSON_OBJECT:
      case BSON_ARRAY: {
        encode(value.second.asObject(), it, &it);
        break;
      }

      // Not yet supported
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
      case BSON_DECI128: assert(false); break;
    }
  }

  bson_set_element_type(it, BSON_END, &it);
  bson_set_size(output, it - output, NULL);
  if (next) *next = it;
}

std::vector<char>
encode(Object const& obj) {
  uint32_t len = encode_len(obj);
  std::vector<char> result(len);
  encode(obj, result.data());
  return result;
}

static void
print_indent(std::ostream& os, size_t indent) {
  for (size_t i = 0; i < indent; ++i) os << " ";
}

static void
bson_print_object_or_array(
    std::ostream& os,
    Object const& obj,
    size_t indent,
    size_t indent_step,
    bson_element_t base_type,
    bool indent_first) {
  uint32_t size = encode_len(obj);

  if (indent_first) print_indent(os, indent);
  switch (base_type) {
    case BSON_OBJECT: {
      os << "object(size=" << size << ") {\n";
      break;
    }

    case BSON_ARRAY: {
      os << "array(size=" << size << ") [\n";
      break;
    }

    default: os << "Not handled: " << (int) base_type << "\n"; return;
  }

  for (auto it = obj.begin(); it != obj.end(); ++it) {
    auto const& value = *it;
    print_indent(os, indent + indent_step);
    os << '"' << value.first << "\": ";

    switch (value.second.getType()) {
      case BSON_STRING: os << '"' << value.second.asString() << '"'; break;
      case BSON_INT32: os << "int32(" << value.second.asInt32() << ')'; break;
      case BSON_INT64: os << "int64(" << value.second.asInt64() << ')'; break;
      case BSON_DOUBLE: os << "double(" << value.second.asDouble() << ')'; break;
      case BSON_BOOLEAN: {
        os << "bool(" << (value.second.asBoolean() ? "true" : "false") << ')';
        break;
      }

      case BSON_BINARY: {
        Binary const& bin = value.second.asBinary();
        os << "binary(size=" << bin.length() << ",subtype=" << bin.getType() << ") <";
        static const uint32_t line_size = 32;
        char buffer[4]                  = {0};
        for (uint32_t i = 0; i < bin.length();) {
          os << "\n";
          print_indent(os, indent + 2 * indent_step);
          for (uint32_t len = 0; i < bin.length() && len < line_size; ++len, ++i) {
            snprintf(buffer, sizeof(buffer), "%02x", bin.get()[i]);
            os << buffer;
          }
        }
        os << "\n";
        print_indent(os, indent + indent_step);
        os << ">";
        break;
      }

      case BSON_OBJECT: {
        bson_print_object_or_array(
            os, value.second.asObject(), indent + indent_step, indent_step, BSON_OBJECT, false);
        break;
      }

      case BSON_ARRAY: {
        bson_print_object_or_array(
            os, value.second.asArray(), indent + indent_step, indent_step, BSON_ARRAY, false);
        break;
      }

      default: os << "Not handled: " << (int) value.second.getType() << "\n"; return;
    }

    if (std::next(it) != obj.end())
      os << ",\n";
    else
      os << "\n";
  }

  print_indent(os, indent);
  switch (base_type) {
    case BSON_OBJECT: {
      os << "}";
      break;
    }

    case BSON_ARRAY: {
      os << "]";
      break;
    }

    default: os << "Not handled: " << (int) base_type << "\n"; return;
  }
}

void
print(std::ostream& os, bson::Object const& obj, size_t indent, size_t indent_step) {
  bson_print_object_or_array(os, obj, indent, indent_step, BSON_OBJECT, true);
}

} // namespace bson

namespace std {

ostream&
operator<<(ostream& os, bson::Object const& obj) {
  bson::print(os, obj, 0, 2);
  return os;
}

} // namespace std