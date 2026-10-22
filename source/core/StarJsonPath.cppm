module;
import std;
import star.string;
import star.unicode;
import star.format;
import star.json;
import star.json_extra;

import star.lexical_cast;

namespace Star {

namespace JsonPath {
  enum class TypeHint {
    Array,
    Object
  };

  typedef std::function<std::expected<TypeHint, StarException>(String&, String const&, Utf8Cursor&)> PathParser;

  struct ParsingExceptionTag {
    static constexpr char const* name() { return "ParsingException"; }
  };
  using ParsingException = StarError<ParsingExceptionTag, JsonException>;
  struct TraversalExceptionTag {
    static constexpr char const* name() { return "TraversalException"; }
  };
  using TraversalException = StarError<TraversalExceptionTag, JsonException>;

  // Parses RFC 6901 JSON Pointers, e.g. /foo/bar/4/baz
  std::expected<TypeHint, StarException> parsePointer(String& outputBuffer, String const& path, Utf8Cursor& cursor);

  // Parses JavaScript-like paths, e.g. foo.bar[4].baz
  std::expected<TypeHint, StarException> parseQueryPath(String& outputBuffer, String const& path, Utf8Cursor& cursor);

  // Retrieves the portion of the Json document referred to by the given path.
  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathGet(Jsonlike base, PathParser parser, String const& path);

  // Missing or nontraversable locations are nullopt; malformed paths remain errors.
  template <typename Jsonlike>
  std::expected<std::optional<Jsonlike>, StarException> pathFind(Jsonlike base, PathParser parser, String const& path);

  template <typename Jsonlike>
  using JsonOp = std::function<std::expected<Jsonlike, StarException>(Jsonlike const&, std::optional<String> const&)>;

  // Applies a function to the portion of the Json document referred to by the
  // given path, returning the resulting new document.  If the end of the path
  // doesn't exist, the JsonOp is called with None, and its result will be
  // inserted into the document.  If the path already existed and the JsonOp
  // returns None, it is erased.  This is not as well-optimized as pathGet, but
  // also not on the critical path for anything.
  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathApply(Jsonlike const& base, PathParser parser, String const& path, JsonOp<Jsonlike> op);

  // Sets a value on a Json document at the location referred to by path,
  // returning the resulting new document.
  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathSet(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value);

  // Erases the location referred to by the path from the document
  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathRemove(Jsonlike const& base, PathParser parser, String const& path);

  // Performs RFC6902 (JSON Patching) add operation. Inserts into arrays, or
  // appends if the last path segment is "-". On objects, does the same as
  // pathSet.
  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathAdd(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value);

  template <typename Jsonlike>
  using EmptyPathOp = std::function<std::expected<Jsonlike, StarException>(Jsonlike const&)>;
  template <typename Jsonlike>
  using ObjectOp = std::function<std::expected<Jsonlike, StarException>(Jsonlike const&, String const&)>;
  template <typename Jsonlike>
  using ArrayOp = std::function<std::expected<Jsonlike, StarException>(Jsonlike const&, std::optional<std::size_t>)>;

  template <typename Jsonlike>
  JsonOp<Jsonlike> genericObjectArrayOp(String path, EmptyPathOp<Jsonlike> emptyPathOp, ObjectOp<Jsonlike> objectOp, ArrayOp<Jsonlike> arrayOp);

  class Path {
  public:
    Path(PathParser parser, String const& path) : m_parser(parser), m_path(path) {}

    template <typename Jsonlike>
    std::expected<Jsonlike, StarException> get(Jsonlike const& base) {
      return pathGet(base, m_parser, m_path);
    }

    template <typename Jsonlike>
    std::expected<Jsonlike, StarException> apply(Jsonlike const& base, JsonOp<Jsonlike> op) {
      return pathApply(base, m_parser, m_path, op);
    }

    template <typename Jsonlike>
    std::expected<Jsonlike, StarException> apply(Jsonlike const& base,
        EmptyPathOp<Jsonlike> emptyPathOp,
        ObjectOp<Jsonlike> objectOp,
        ArrayOp<Jsonlike> arrayOp) {
      JsonOp<Jsonlike> combinedOp = genericObjectArrayOp(m_path, emptyPathOp, objectOp, arrayOp);
      return pathApply(base, m_parser, m_path, combinedOp);
    }

    template <typename Jsonlike>
    std::expected<Jsonlike, StarException> set(Jsonlike const& base, Jsonlike const& value) {
      return pathSet(base, m_parser, m_path, value);
    }

    template <typename Jsonlike>
    std::expected<Jsonlike, StarException> remove(Jsonlike const& base) {
      return pathRemove(base, m_parser, m_path);
    }

    template <typename Jsonlike>
    std::expected<Jsonlike, StarException> add(Jsonlike const& base, Jsonlike const& value) {
      return pathAdd(base, m_parser, m_path, value);
    }

    String const& path() const {
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
  std::expected<Jsonlike, StarException> pathGet(Jsonlike value, PathParser parser, String const& path) {
    if (!parser)
      return std::unexpected(ParsingException("Cannot invoke an empty Json path parser"));
    String buffer;
    buffer.reserve(path.utf8Size());
    Utf8Cursor cursor(path.utf8());
    while (!cursor.atEnd()) {
      auto parsed = parser(buffer, path, cursor);
      if (!parsed)
        return std::unexpected(std::move(parsed.error()));
      if (value.type() == Json::Type::Array) {
        if (buffer == "-")
          return std::unexpected(TraversalException::format("Tried to get key '{}' in non-object type in pathGet(\"{}\")", buffer, path));
        auto i = maybeLexicalCast<std::size_t>(buffer);
        if (!i)
          return std::unexpected(TraversalException::format("Cannot parse '{}' as index in pathGet(\"{}\")", buffer, path));
        auto size = value.size();
        if (!size)
          return std::unexpected(std::move(size.error()));
        if (*i >= *size)
          return std::unexpected(TraversalException::format("Index {} out of range in pathGet(\"{}\")", buffer, path));
        auto child = value.get(*i);
        if (!child)
          return std::unexpected(std::move(child.error()));
        value = std::move(*child);
      } else if (value.type() == Json::Type::Object) {
        auto contains = value.contains(buffer);
        if (!contains)
          return std::unexpected(std::move(contains.error()));
        if (!*contains)
          return std::unexpected(TraversalException::format("No such key '{}' in pathGet(\"{}\")", buffer, path));
        auto child = value.get(buffer);
        if (!child)
          return std::unexpected(std::move(child.error()));
        value = std::move(*child);
      } else {
        return std::unexpected(TraversalException::format("Tried to get key '{}' in non-object type in pathGet(\"{}\")", buffer, path));
      }
    }
    return value;
  }

  // Optional misses are normal results, not diagnostics to construct and discard.
  template <typename Jsonlike>
  std::expected<std::optional<Jsonlike>, StarException> pathFind(Jsonlike value, PathParser parser, String const& path) {
    if (!parser)
      return std::unexpected(ParsingException("Cannot invoke an empty Json path parser"));
    String buffer;
    buffer.reserve(path.utf8Size());
    Utf8Cursor cursor(path.utf8());
    while (!cursor.atEnd()) {
      auto parsed = parser(buffer, path, cursor);
      if (!parsed)
        return std::unexpected(std::move(parsed.error()));
      if (value.type() == Json::Type::Array) {
        if (buffer == "-")
          return std::optional<Jsonlike>{};
        auto i = maybeLexicalCast<std::size_t>(buffer);
        if (!i)
          return std::optional<Jsonlike>{};
        auto size = value.size();
        if (!size)
          return std::unexpected(std::move(size.error()));
        if (*i >= *size)
          return std::optional<Jsonlike>{};
        auto child = value.get(*i);
        if (!child)
          return std::unexpected(std::move(child.error()));
        value = std::move(*child);
      } else if (value.type() == Json::Type::Object) {
        auto contains = value.contains(buffer);
        if (!contains)
          return std::unexpected(std::move(contains.error()));
        if (!*contains)
          return std::optional<Jsonlike>{};
        auto child = value.get(buffer);
        if (!child)
          return std::unexpected(std::move(child.error()));
        value = std::move(*child);
      } else {
        return std::optional<Jsonlike>{};
      }
    }
    return std::optional<Jsonlike>(std::move(value));
  }

  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathApply(String& buffer,
      Jsonlike const& value, PathParser parser, String const& path,
      Utf8Cursor const current, JsonOp<Jsonlike> op) {
    if (current.atEnd())
      return op(value, {});
    auto cursor = current;
    auto parsed = parser(buffer, path, cursor);
    if (!parsed)
      return std::unexpected(std::move(parsed.error()));
    if (value.type() == Json::Type::Array) {
      if (cursor.atEnd())
        return op(value, buffer);
      auto i = maybeLexicalCast<std::size_t>(buffer);
      if (!i)
        return std::unexpected(TraversalException::format("Cannot parse '{}' as index in pathApply(\"{}\")", buffer, path));
      auto size = value.size();
      if (!size)
        return std::unexpected(std::move(size.error()));
      if (*i >= *size)
        return std::unexpected(TraversalException::format("Index {} out of range in pathApply(\"{}\")", buffer, path));
      auto child = value.get(*i);
      if (!child)
        return std::unexpected(std::move(child.error()));
      auto changed = pathApply(buffer, *child, parser, path, cursor, op);
      if (!changed)
        return std::unexpected(std::move(changed.error()));
      return value.set(*i, std::move(*changed));
    } else if (value.type() == Json::Type::Object) {
      if (cursor.atEnd())
        return op(value, buffer);
      auto contains = value.contains(buffer);
      if (!contains)
        return std::unexpected(std::move(contains.error()));
      if (!*contains)
        return std::unexpected(TraversalException::format("No such key '{}' in pathApply(\"{}\")", buffer, path));
      auto child = value.get(buffer);
      if (!child)
        return std::unexpected(std::move(child.error()));
      auto changed = pathApply(buffer, *child, parser, path, cursor, op);
      if (!changed)
        return std::unexpected(std::move(changed.error()));
      cursor = current;
      // Recursion reused the buffer; recover the current path component.
      parsed = parser(buffer, path, cursor);
      if (!parsed)
        return std::unexpected(std::move(parsed.error()));
      return value.set(buffer, std::move(*changed));
    }
    return std::unexpected(TraversalException::format("Tried to get key '{}' in non-object type in pathApply(\"{}\")", buffer, path));
  }

  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathApply(Jsonlike const& base, PathParser parser, String const& path, JsonOp<Jsonlike> op) {
    if (!parser)
      return std::unexpected(ParsingException("Cannot invoke an empty Json path parser"));
    if (!op)
      return std::unexpected(TraversalException("Cannot invoke an empty Json path operation"));
    String buffer;
    return pathApply(buffer, base, parser, path, Utf8Cursor(path.utf8()), op);
  }

  template <typename Jsonlike>
  JsonOp<Jsonlike> genericObjectArrayOp(String path, EmptyPathOp<Jsonlike> emptyPathOp, ObjectOp<Jsonlike> objectOp, ArrayOp<Jsonlike> arrayOp) {
    return [=](Jsonlike const& parent, std::optional<String> const& key) -> std::expected<Jsonlike, StarException> {
      if (!key) {
        if (!emptyPathOp)
          return std::unexpected(TraversalException("Cannot invoke an empty Json path operation"));
        return emptyPathOp(parent);
      }
      if (parent.type() == Json::Type::Array) {
        if (!arrayOp)
          return std::unexpected(TraversalException("Cannot invoke an empty Json array operation"));
        if (*key == "-")
          return arrayOp(parent, {});
        auto i = maybeLexicalCast<std::size_t>(*key);
        if (!i)
          return std::unexpected(TraversalException::format("Cannot parse '{}' as index in Json path \"{}\"", *key, path));
        auto size = parent.size();
        if (!size)
          return std::unexpected(std::move(size.error()));
        if (*i > *size)
          return std::unexpected(TraversalException::format("Index {} out of range in Json path \"{}\"", *key, path));
        if (*i == *size)
          i = {};
        return arrayOp(parent, i);
      } else if (parent.type() == Json::Type::Object) {
        if (!objectOp)
          return std::unexpected(TraversalException("Cannot invoke an empty Json object operation"));
        return objectOp(parent, *key);
      }
      return std::unexpected(TraversalException::format("Tried to set key '{}' in non-object type in pathSet(\"{}\")", *key, path));
    };
  }

  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathSet(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value) {
    EmptyPathOp<Jsonlike> emptyPathOp = [&value](Jsonlike const&) -> std::expected<Jsonlike, StarException> { return value; };
    ObjectOp<Jsonlike> objectOp = [&value](Jsonlike const& object, String const& key) { return object.set(key, value); };
    ArrayOp<Jsonlike> arrayOp = [&value](Jsonlike const& array, std::optional<std::size_t> i) {
      return i ? array.set(*i, value) : array.append(value);
    };
    return pathApply(base, parser, path, genericObjectArrayOp(path, emptyPathOp, objectOp, arrayOp));
  }

  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathRemove(Jsonlike const& base, PathParser parser, String const& path) {
    EmptyPathOp<Jsonlike> emptyPathOp = [](Jsonlike const&) -> std::expected<Jsonlike, StarException> { return Jsonlike{}; };
    ObjectOp<Jsonlike> objectOp = [](Jsonlike const& object, String const& key) -> std::expected<Jsonlike, StarException> {
      auto contains = object.contains(key);
      if (!contains)
        return std::unexpected(std::move(contains.error()));
      if (!*contains)
        return std::unexpected(TraversalException::format("Could not find \"{}\" to remove", key));
      return object.eraseKey(key);
    };
    ArrayOp<Jsonlike> arrayOp = [](Jsonlike const& array, std::optional<std::size_t> i) -> std::expected<Jsonlike, StarException> {
      if (i)
        return array.eraseIndex(*i);
      return std::unexpected(TraversalException("Could not remove element after end of array"));
    };
    return pathApply(base, parser, path, genericObjectArrayOp(path, emptyPathOp, objectOp, arrayOp));
  }

  template <typename Jsonlike>
  std::expected<Jsonlike, StarException> pathAdd(Jsonlike const& base, PathParser parser, String const& path, Jsonlike const& value) {
    EmptyPathOp<Jsonlike> emptyPathOp = [&value](Jsonlike const& document) -> std::expected<Jsonlike, StarException> {
      if (document.type() == Json::Type::Null)
        return value;
      return std::unexpected(JsonException("Cannot add a value to the entire document, it is not empty."));
    };
    ObjectOp<Jsonlike> objectOp = [&value](Jsonlike const& object, String const& key) { return object.set(key, value); };
    ArrayOp<Jsonlike> arrayOp = [&value](Jsonlike const& array, std::optional<std::size_t> i) {
      return i ? array.insert(*i, value) : array.append(value);
    };
    return pathApply(base, parser, path, genericObjectArrayOp(path, emptyPathOp, objectOp, arrayOp));
  }
}

}

export module star.json_path;

export namespace Star::JsonPath {
  using ::Star::JsonPath::TypeHint;
  using ::Star::JsonPath::PathParser;
  using ::Star::JsonPath::ParsingExceptionTag;
  using ::Star::JsonPath::ParsingException;
  using ::Star::JsonPath::TraversalExceptionTag;
  using ::Star::JsonPath::TraversalException;
  using ::Star::JsonPath::parsePointer;
  using ::Star::JsonPath::parseQueryPath;
  using ::Star::JsonPath::pathGet;
  using ::Star::JsonPath::pathFind;
  using ::Star::JsonPath::JsonOp;
  using ::Star::JsonPath::pathApply;
  using ::Star::JsonPath::pathSet;
  using ::Star::JsonPath::pathRemove;
  using ::Star::JsonPath::pathAdd;
  using ::Star::JsonPath::EmptyPathOp;
  using ::Star::JsonPath::ObjectOp;
  using ::Star::JsonPath::ArrayOp;
  using ::Star::JsonPath::genericObjectArrayOp;
  using ::Star::JsonPath::Path;
  using ::Star::JsonPath::Pointer;
  using ::Star::JsonPath::QueryPath;
}
