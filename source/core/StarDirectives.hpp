#pragma once

#include "StarImageProcessing.hpp"
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"

namespace Star {

class Directives;
class DirectivesGroup;
struct DirectivesExceptionTag { static constexpr char const* typeName = "DirectivesException"; };
using DirectivesException = TypedException<StarException, DirectivesExceptionTag>;

// Kae: My attempt at reducing memory allocation and per-frame string parsing for extremely long directives
class Directives {
public:
  struct Shared;
  struct Entry {
    mutable ImageOperation operation;
    size_t begin;
    size_t length;

    [[nodiscard]] ImageOperation const& loadOperation(Shared const& parent) const;
    [[nodiscard]] StringView string(Shared const& parent) const;
    Entry(ImageOperation&& newOperation, size_t begin, size_t end);
    Entry(ImageOperation const& newOperation, size_t begin, size_t end);
    Entry(Entry const& other);
  };

  struct Shared {
    List<Entry> entries;
    String string;
    size_t hash = 0;
    mutable Mutex mutex;

    [[nodiscard]] bool empty() const;
    Shared() = default;
    Shared(List<Entry>&& givenEntries, String&& givenString);
  };

  Directives();
  Directives(String const& directives);
  Directives(String&& directives);
  Directives(const char* directives);
  Directives(Directives const& directives);
  Directives(Directives&& directives) noexcept;
  ~Directives();

  Directives& operator=(String const& s);
  Directives& operator=(String&& s);
  Directives& operator=(const char* s);
  Directives& operator=(Directives&& other) noexcept;
  Directives& operator=(Directives const& other);

  void loadOperations() const;
  void parse(String&& directives);
  [[nodiscard]] StringView prefix() const;
  [[nodiscard]] String string() const;
  [[nodiscard]] String const* stringPtr() const;
  [[nodiscard]] String buildString() const;
  [[nodiscard]] String& addToString(String& out) const;
  [[nodiscard]] size_t hash() const;
  [[nodiscard]] size_t size() const;
  [[nodiscard]] bool empty() const;
  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] Shared const& operator*() const;
  [[nodiscard]] Shared const* operator->() const;

  [[nodiscard]] bool equals(Directives const& other) const;
  [[nodiscard]] bool equals(String const& string) const;

  bool operator==(Directives const& other) const;
  bool operator==(String const& string) const;
  bool operator!=(Directives const& other) const;
  bool operator!=(String const& string) const;

  friend DataStream& operator>>(DataStream& ds, Directives& directives);
  friend DataStream& operator<<(DataStream& ds, Directives const& directives);

  //friend bool operator==(Directives const& d1, String const& d2);
  //friend bool operator==(Directives const& directives, String const& string);
  //friend bool operator==(String const& string, Directives const& directives);

  std::shared_ptr<Shared const> m_shared;
};

class DirectivesGroup {
public:
  DirectivesGroup() = default;
  DirectivesGroup(String const& directives);
  DirectivesGroup(String&& directives);

  [[nodiscard]] bool empty() const;
  [[nodiscard]] explicit operator bool() const;
  [[nodiscard]] bool compare(DirectivesGroup const& other) const;
  void append(Directives const& other);
  void clear();

  DirectivesGroup& operator+=(Directives const& other);

  [[nodiscard]] String toString() const;
  void addToString(String& string) const;

  using DirectivesCallback = function<void(Directives::Entry const&, Directives const&)>;
  using AbortableDirectivesCallback = function<bool(Directives::Entry const&, Directives const&)>;

  void forEach(DirectivesCallback callback) const;
  [[nodiscard]] bool forEachAbortable(AbortableDirectivesCallback callback) const;

  [[nodiscard]] Image applyNewImage(const Image& image, ImageReferenceCallback refCallback = {}) const;
  void applyExistingImage(Image& image, ImageReferenceCallback refCallback = {}) const;
  
  [[nodiscard]] size_t hash() const;
  [[nodiscard]] const List<Directives>& list() const;

  friend bool operator==(DirectivesGroup const& a, DirectivesGroup const& b);
  friend bool operator!=(DirectivesGroup const& a, DirectivesGroup const& b);

  friend DataStream& operator>>(DataStream& ds, DirectivesGroup& directives);
  friend DataStream& operator<<(DataStream& ds, DirectivesGroup const& directives);
private:
  void buildString(String& string, const DirectivesGroup& directives) const;

  List<Directives> m_directives;
  size_t m_count = 0;
};

template <>
struct hash<DirectivesGroup> {
  [[nodiscard]] size_t operator()(DirectivesGroup const& s) const;
};

using ImageDirectives = DirectivesGroup;

inline Directives::Shared const& Directives::operator*() const {
  return *m_shared;
}
inline Directives::Shared const* Directives::operator->() const {
  if (!m_shared)
    throw DirectivesException("Directives::operator-> nullptr");
  return m_shared.get();
}

}
