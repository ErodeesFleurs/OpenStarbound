#pragma once

#include "StarDataStream.hpp"
#include "StarException.hpp"
#include "StarHash.hpp"
#include "StarImageProcessing.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

using DirectivesException = ExceptionDerived<"DirectivesException">;

// Kae: My attempt at reducing memory allocation and per-frame string parsing for extremely long directives
class Directives {
public:
  struct Shared;
  struct Entry {
    mutable ImageOperation operation;
    std::size_t begin;
    std::size_t length;

    auto loadOperation(Shared const& parent) const -> ImageOperation const&;
    auto string(Shared const& parent) const -> StringView;
    Entry(ImageOperation&& newOperation, std::size_t begin, std::size_t end);
    Entry(ImageOperation const& newOperation, std::size_t begin, std::size_t end);
    Entry(Entry const& other);
  };

  struct Shared {
    List<Entry> entries;
    String string;
    std::size_t hash = 0;
    mutable Mutex mutex;

    auto empty() const -> bool;
    Shared();
    Shared(List<Entry>&& givenEntries, String&& givenString);
  };

  Directives();
  Directives(String const& directives);
  Directives(String&& directives);
  Directives(const char* directives);
  Directives(Directives const& directives);
  Directives(Directives&& directives) noexcept;
  ~Directives();

  auto operator=(String const& s) -> Directives&;
  auto operator=(String&& s) -> Directives&;
  auto operator=(const char* s) -> Directives&;
  auto operator=(Directives&& other) noexcept -> Directives&;
  auto operator=(Directives const& other) -> Directives&;

  void loadOperations() const;
  void parse(String&& directives);
  [[nodiscard]] auto prefix() const -> StringView;
  [[nodiscard]] auto string() const -> String;
  [[nodiscard]] auto stringPtr() const -> String const*;
  [[nodiscard]] auto buildString() const -> String;
  auto addToString(String& out) const -> String&;
  [[nodiscard]] auto hash() const -> std::size_t;
  [[nodiscard]] auto size() const -> std::size_t;
  [[nodiscard]] auto empty() const -> bool;
  operator bool() const;

  auto operator*() const -> Shared const&;
  auto operator->() const -> Shared const*;

  [[nodiscard]] auto equals(Directives const& other) const -> bool;
  [[nodiscard]] auto equals(String const& string) const -> bool;

  auto operator==(Directives const& other) const -> bool;
  auto operator==(String const& string) const -> bool;
  auto operator!=(Directives const& other) const -> bool;
  auto operator!=(String const& string) const -> bool;

  friend auto operator>>(DataStream& ds, Directives& directives) -> DataStream&;
  friend auto operator<<(DataStream& ds, Directives const& directives) -> DataStream&;

  std::shared_ptr<Shared const> m_shared;
};

class DirectivesGroup {
public:
  DirectivesGroup();
  DirectivesGroup(String const& directives);
  DirectivesGroup(String&& directives);

  [[nodiscard]] auto empty() const -> bool;
  operator bool() const;
  [[nodiscard]] auto compare(DirectivesGroup const& other) const -> bool;
  void append(Directives const& other);
  void clear();

  auto operator+=(Directives const& other) -> DirectivesGroup&;

  [[nodiscard]] auto toString() const -> String;
  void addToString(String& string) const;

  using DirectivesCallback = std::function<void(Directives::Entry const&, Directives const&)>;
  using AbortableDirectivesCallback = std::function<bool(Directives::Entry const&, Directives const&)>;

  void forEach(DirectivesCallback callback) const;
  [[nodiscard]] auto forEachAbortable(AbortableDirectivesCallback callback) const -> bool;

  [[nodiscard]] auto applyNewImage(const Image& image, ImageReferenceCallback refCallback = {}) const -> Image;
  void applyExistingImage(Image& image, ImageReferenceCallback refCallback = {}) const;

  [[nodiscard]] auto hash() const -> std::size_t;
  [[nodiscard]] auto list() const -> const List<Directives>&;

  friend auto operator==(DirectivesGroup const& a, DirectivesGroup const& b) -> bool;
  friend auto operator!=(DirectivesGroup const& a, DirectivesGroup const& b) -> bool;

  friend auto operator>>(DataStream& ds, DirectivesGroup& directives) -> DataStream&;
  friend auto operator<<(DataStream& ds, DirectivesGroup const& directives) -> DataStream&;

private:
  void buildString(String& string, const DirectivesGroup& directives) const;

  List<Directives> m_directives;
  std::size_t m_count = 0;
};

template <>
struct hash<DirectivesGroup> {
  auto operator()(DirectivesGroup const& s) const -> std::size_t;
};

using ImageDirectives = DirectivesGroup;

inline auto Directives::operator*() const -> Directives::Shared const& {
  return *m_shared;
}
inline auto Directives::operator->() const -> Directives::Shared const* {
  if (!m_shared)
    throw DirectivesException("Directives::operator-> nullptr");
  return m_shared.get();
}

}// namespace Star
