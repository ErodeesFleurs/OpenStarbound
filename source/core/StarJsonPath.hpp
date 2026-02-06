#pragma once

#include "StarException.hpp"
#include "StarLexicalCast.hpp"
#include "StarJson.hpp"

import std;



namespace Star::JsonPath {
  enum class TypeHint {
    Array,
    Object
  };

  using PathParser = std::function<TypeHint(String&, String const&, String::const_iterator&, String::const_iterator)>;

  using ParsingException = ExceptionDerived<"ParsingException", JsonException>;
  using TraversalException = ExceptionDerived<"TraversalException", JsonException>;

  // Parses RFC 6901 JSON Pointers, e.g. /foo/bar/4/baz
  auto parsePointer(String& outputBuffer, String const& path, String::const_iterator& iterator, String::const_iterator end) -> TypeHint;

  // Parses JavaScript-like paths, e.g. foo.bar[4].baz
  auto parseQueryPath(String& outputBuffer, String const& path, String::const_iterator& iterator, String::const_iterator end) -> TypeHint;

  // Retrieves the portion of the Json document referred to by the given path.
  template <typename Jsonlike>
  auto pathGet(Jsonlike base, PathParser parser, String const& path) -> Jsonlike;

  // Find a given portion of the JSON document, if it exists.  Instead of
  // throwing a TraversalException if a portion of the path is invalid, simply
  // returns nothing.
  template <typename Jsonlike>
  auto pathFind(Jsonlike base, PathParser parser, String const& path) -> std::optional<Jsonlike>;

  template <typename Jsonlike>
  using JsonOp = std::function<Jsonlike(Jsonlike const&, std::optional<String> const&)>;

  // Applies a function to the portion of the Json document referred to by the
  // given path, returning the resulting new document.  If the end of the path
  // doesn't exist, the JsonOp is called with None, and its result will be
  // inserted into the document.  If the path already existed and the JsonOp
  // returns None, it is erased.  This is not as well-optimized as pathGet, but
  // also not on the critical path for anything.
  template <typename Jsonlike>
  auto pathApply(Jsonlike const& base, PathParser parser, String const& path, JsonOp<Jsonlike> op) -> Jsonlike;

  // Sets a value on a Json document at the location referred to by path,
  // returning the resulting new document.
  template <typename Jsonlike>
  auto pathSet(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value) -> Jsonlike;

  // Erases the location referred to by the path from the document
  template <typename Jsonlike>
  auto pathRemove(Jsonlike const& base, PathParser parser, String const& path) -> Jsonlike;

  // Performs RFC6902 (JSON Patching) add operation. Inserts into arrays, or
  // appends if the last path segment is "-". On objects, does the same as
  // pathSet.
  template <typename Jsonlike>
  auto pathAdd(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value) -> Jsonlike;

  template <typename Jsonlike>
  using EmptyPathOp = std::function<Jsonlike(Jsonlike const&)>;
  template <typename Jsonlike>
  using ObjectOp = std::function<Jsonlike(Jsonlike const&, String const&)>;
  template <typename Jsonlike>
  using ArrayOp = std::function<Jsonlike(Jsonlike const&, std::optional<size_t>)>;

  template <typename Jsonlike>
  auto genericObjectArrayOp(String path, EmptyPathOp<Jsonlike> emptyPathOp, ObjectOp<Jsonlike> objectOp, ArrayOp<Jsonlike> arrayOp) -> JsonOp<Jsonlike>;

  class Path {
  public:
    Path(PathParser parser, String  path) : m_parser(std::move(parser)), m_path(std::move(path)) {}

    template <typename Jsonlike>
    auto get(Jsonlike const& base) -> Jsonlike {
      return pathGet(base, m_parser, m_path);
    }

    template <typename Jsonlike>
    auto apply(Jsonlike const& base, JsonOp<Jsonlike> op) -> Jsonlike {
      return pathApply(base, m_parser, m_path, op);
    }

    template <typename Jsonlike>
    auto apply(Jsonlike const& base,
        EmptyPathOp<Jsonlike> emptyPathOp,
        ObjectOp<Jsonlike> objectOp,
        ArrayOp<Jsonlike> arrayOp) -> Jsonlike {
      JsonOp<Jsonlike> combinedOp = genericObjectArrayOp(m_path, emptyPathOp, objectOp, arrayOp);
      return pathApply(base, m_parser, m_path, combinedOp);
    }

    template <typename Jsonlike>
    auto set(Jsonlike const& base, Jsonlike const& value) -> Jsonlike {
      return pathSet(base, m_parser, m_path, value);
    }

    template <typename Jsonlike>
    auto remove(Jsonlike const& base) -> Jsonlike {
      return pathRemove(base, m_parser, m_path);
    }

    template <typename Jsonlike>
    auto add(Jsonlike const& base, Jsonlike const& value) -> Jsonlike {
      return pathAdd(base, m_parser, m_path, value);
    }

    [[nodiscard]] auto path() const -> String const& {
      return m_path;
    }

  private:
    PathParser m_parser;
    String m_path;
  };

  class Pointer : public Path {
  public:
    Pointer(String const& path) : Path(parsePointer, path) {}
  };

  class QueryPath : public Path {
  public:
    QueryPath(String const& path) : Path(parseQueryPath, path) {}
  };

  template <typename Jsonlike>
  auto pathGet(Jsonlike value, PathParser parser, String const& path) -> Jsonlike {
    String buffer;
    buffer.reserve(path.size());

    auto pos = path.begin();

    while (pos != path.end()) {
      parser(buffer, path, pos, path.end());

      if (value.type() == Json::Type::Array) {
        if (buffer == "-")
          throw TraversalException::format("Tried to get key '{}' in non-object type in pathGet(\"{}\")", buffer, path);
        std::optional<size_t> i = maybeLexicalCast<size_t>(buffer);
        if (!i)
          throw TraversalException::format("Cannot parse '{}' as index in pathGet(\"{}\")", buffer, path);

        if (*i < value.size())
          value = value.get(*i);
        else
          throw TraversalException::format("Index {} out of range in pathGet(\"{}\")", buffer, path);

      } else if (value.type() == Json::Type::Object) {
        if (value.contains(buffer))
          value = value.get(buffer);
        else
          throw TraversalException::format("No such key '{}' in pathGet(\"{}\")", buffer, path);

      } else {
        throw TraversalException::format("Tried to get key '{}' in non-object type in pathGet(\"{}\")", buffer, path);
      }
    }
    return value;
  }

  template <typename Jsonlike>
  auto pathFind(Jsonlike value, PathParser parser, String const& path) -> std::optional<Jsonlike> {
    String buffer;
    buffer.reserve(path.size());

    auto pos = path.begin();

    while (pos != path.end()) {
      parser(buffer, path, pos, path.end());

      if (value.type() == Json::Type::Array) {
        if (buffer == "-")
          return std::nullopt;

        std::optional<size_t> i = maybeLexicalCast<size_t>(buffer);
        if (i && *i < value.size())
          value = value.get(*i);
        else
          return {};

      } else if (value.type() == Json::Type::Object) {
        if (value.contains(buffer))
          value = value.get(buffer);
        else
          return std::nullopt;

      } else {
        return std::nullopt;
      }
    }
    return value;
  }

  template <typename Jsonlike>
  auto pathApply(String& buffer,
      Jsonlike const& value,
      PathParser parser,
      String const& path,
      String::const_iterator const current,
      JsonOp<Jsonlike> op) -> Jsonlike {
    if (current == path.end())
      return op(value, std::nullopt);

    String::const_iterator iterator = current;
    parser(buffer, path, iterator, path.end());

    if (value.type() == Json::Type::Array) {
      if (iterator == path.end()) {
        return op(value, buffer);
      } else {
        std::optional<size_t> i = maybeLexicalCast<size_t>(buffer);
        if (!i)
          throw TraversalException::format("Cannot parse '{}' as index in pathApply(\"{}\")", buffer, path);

        if (*i >= value.size())
          throw TraversalException::format("Index {} out of range in pathApply(\"{}\")", buffer, path);

        return value.set(*i, pathApply(buffer, value.get(*i), parser, path, iterator, op));
      }

    } else if (value.type() == Json::Type::Object) {
      if (iterator == path.end()) {
        return op(value, buffer);

      } else {
        if (!value.contains(buffer))
          throw TraversalException::format("No such key '{}' in pathApply(\"{}\")", buffer, path);

        Jsonlike newChild = pathApply(buffer, value.get(buffer), parser, path, iterator, op);
        iterator = current;
        // pathApply just mutated buffer. Recover the current path component:
        parser(buffer, path, iterator, path.end());
        return value.set(buffer, newChild);
      }

    } else {
      throw TraversalException::format("Tried to get key '{}' in non-object type in pathApply(\"{}\")", buffer, path);
    }
  }

  template <typename Jsonlike>
  auto pathApply(Jsonlike const& base, PathParser parser, String const& path, JsonOp<Jsonlike> op) -> Jsonlike {
    String buffer;
    return pathApply(buffer, base, parser, path, path.begin(), op);
  }

  template <typename Jsonlike>
  auto genericObjectArrayOp(String path, EmptyPathOp<Jsonlike> emptyPathOp, ObjectOp<Jsonlike> objectOp, ArrayOp<Jsonlike> arrayOp) -> JsonOp<Jsonlike> {
    return [emptyPathOp, arrayOp, objectOp, path](Jsonlike const& parent, std::optional<String> const& key) -> Jsonlike {
      if (!key.has_value())
        return emptyPathOp(parent);
      if (parent.type() == Json::Type::Array) {
        if (*key == "-")
          return arrayOp(parent, std::nullopt);
        std::optional<size_t> i = maybeLexicalCast<size_t>(*key);
        if (!i)
          throw TraversalException::format("Cannot parse '{}' as index in Json path \"{}\"", *key, path);
        if (i && *i > parent.size())
          throw TraversalException::format("Index {} out of range in Json path \"{}\"", *key, path);
        if (i && *i == parent.size())
          i = std::nullopt;
        return arrayOp(parent, i);
      } else if (parent.type() == Json::Type::Object) {
        return objectOp(parent, *key);
      } else {
        throw TraversalException::format("Tried to set key '{}' in non-object type in pathSet(\"{}\")", *key, path);
      }
    };
  }

  template <typename Jsonlike>
  auto pathSet(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value) -> Jsonlike {
    EmptyPathOp<Jsonlike> emptyPathOp = [&value](Jsonlike const&) -> auto {
      return value;
    };
    ObjectOp<Jsonlike> objectOp = [&value](Jsonlike const& object, String const& key) -> auto {
      return object.set(key, value);
    };
    ArrayOp<Jsonlike> arrayOp = [&value](Jsonlike const& array, std::optional<size_t> i) -> auto {
      if (i.has_value())
        return array.set(*i, value);
      return array.append(value);
    };
    return pathApply(base, parser, path, genericObjectArrayOp(path, emptyPathOp, objectOp, arrayOp));
  }

  template <typename Jsonlike>
  auto pathRemove(Jsonlike const& base, PathParser parser, String const& path) -> Jsonlike {
    EmptyPathOp<Jsonlike> emptyPathOp = [](Jsonlike const&) -> auto { return Json{}; };
    ObjectOp<Jsonlike> objectOp = [](Jsonlike const& object, String const& key) -> auto {
      if (!object.contains(key))
        throw TraversalException::format("Could not find \"{}\" to remove", key);
      return object.eraseKey(key);
    };
    ArrayOp<Jsonlike> arrayOp = [](Jsonlike const& array, std::optional<size_t> i) -> auto {
      if (i.has_value())
        return array.eraseIndex(*i);
      throw TraversalException("Could not remove element after end of array");
    };
    return pathApply(base, parser, path, genericObjectArrayOp(path, emptyPathOp, objectOp, arrayOp));
  }

  template <typename Jsonlike>
  auto pathAdd(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value) -> Jsonlike {
    EmptyPathOp<Jsonlike> emptyPathOp = [&value](Jsonlike const& document) -> auto {
      if (document.type() == Json::Type::Null)
        return value;
      throw JsonException("Cannot add a value to the entire document, it is not empty.");
    };
    ObjectOp<Jsonlike> objectOp = [&value](Jsonlike const& object, String const& key) -> auto {
      return object.set(key, value);
    };
    ArrayOp<Jsonlike> arrayOp = [&value](Jsonlike const& array, std::optional<size_t> i) -> auto {
      if (i.has_value())
        return array.insert(*i, value);
      return array.append(value);
    };
    return pathApply(base, parser, path, genericObjectArrayOp(path, emptyPathOp, objectOp, arrayOp));
  }
}
