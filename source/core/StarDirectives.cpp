#include "StarDirectives.hpp"

#include "StarImage.hpp"
#include "StarImageProcessing.hpp"

import std;

namespace Star {

Directives::Entry::Entry(ImageOperation&& newOperation, std::size_t strBegin, std::size_t strLength) {
  operation = std::move(newOperation);
  begin = strBegin;
  length = strLength;
}

Directives::Entry::Entry(ImageOperation const& newOperation, std::size_t strBegin, std::size_t strLength) {
  operation = newOperation;
  begin = strBegin;
  length = strLength;
}

Directives::Entry::Entry(Entry const& other) {
  operation = other.operation;
  begin = other.begin;
  length = other.length;
}

auto Directives::Entry::loadOperation(Shared const& parent) const -> ImageOperation const& {
  if (operation.is<NullImageOperation>()) {
    try {
      operation = imageOperationFromString(string(parent));
    } catch (StarException const& e) { operation = ErrorImageOperation{std::exception_ptr()}; }
  }
  return operation;
}

auto Directives::Entry::string(Shared const& parent) const -> StringView {
  StringView result = parent.string;
  result = result.utf8().substr(begin, length);
  return result;
}

auto Directives::Shared::empty() const -> bool {
  return entries.empty();
}

Directives::Shared::Shared() = default;

Directives::Shared::Shared(List<Entry>&& givenEntries, String&& givenString) {
  entries = std::move(givenEntries);
  string = std::move(givenString);

  if (string.empty()) {
    hash = 0;
  } else {
    std::string_view sv(string.utf8Ptr(), string.utf8Size());
    hash = std::hash<std::string_view>{}(sv);
  }
}

Directives::Directives() = default;

Directives::Directives(String const& directives) {
  parse(String(directives));
}

Directives::Directives(String&& directives) {
  parse(std::move(directives));
}

Directives::Directives(const char* directives) {
  parse(directives);
}

Directives::Directives(Directives&& directives) noexcept {
  *this = std::move(directives);
}

Directives::Directives(Directives const& directives) {
  *this = directives;
}

Directives::~Directives() = default;

auto Directives::operator=(String const& s) -> Directives& {
  if (m_shared && m_shared->string == s)
    return *this;

  parse(String(s));
  return *this;
}

auto Directives::operator=(String&& s) -> Directives& {
  if (m_shared && m_shared->string == s) {
    s.clear();
    return *this;
  }

  parse(std::move(s));
  return *this;
}

auto Directives::operator=(const char* s) -> Directives& {
  if (m_shared && m_shared->string.utf8().compare(s) == 0)
    return *this;

  parse(s);
  return *this;
}

auto Directives::operator=(Directives&& other) noexcept -> Directives& {
  m_shared = std::move(other.m_shared);
  return *this;
}

auto Directives::operator=(Directives const& other) -> Directives& = default;

void Directives::loadOperations() const {
  if (!m_shared)
    return;

  MutexLocker locker(m_shared->mutex, false);
  if (m_shared.use_count() != 1)
    locker.lock();
  for (auto& entry : m_shared->entries)
    entry.loadOperation(*m_shared);
}

void Directives::parse(String&& directives) {
  if (directives.empty()) {
    m_shared.reset();
    return;
  }

  List<Entry> entries;
  StringView view(directives);
  view.forEachSplitView("?", [&](StringView split, std::size_t beg, std::size_t end) -> void {
    if (!split.empty()) {
      ImageOperation operation = NullImageOperation();
      if (beg == 0) {
        try {
          operation = imageOperationFromString(split);
        } catch (StarException const& e) { operation = ErrorImageOperation{std::exception_ptr()}; }
      }
      entries.emplace_back(std::move(operation), beg, end);
    }
  });

  if (entries.empty()) {
    m_shared.reset();
    return;
  }

  m_shared = std::make_shared<Shared const>(std::move(entries), std::move(directives));
  if (view.utf8().size() < 1000) {// Pre-load short enough directives
    for (auto& entry : m_shared->entries)
      entry.loadOperation(*m_shared);
  }
}

auto Directives::prefix() const -> StringView {
  if (!m_shared)
    return "";
  else if (m_shared->empty())
    return m_shared->string;
  else if (m_shared->string.utf8().at(0) == '?')
    return "";
  else
    return m_shared->entries.front().string(*m_shared);
}

auto Directives::string() const -> String {
  if (!m_shared)
    return "";
  else
    return m_shared->string;
}

auto Directives::stringPtr() const -> String const* {
  if (!m_shared)
    return nullptr;
  else
    return &m_shared->string;
}

auto Directives::buildString() const -> String {
  if (m_shared) {
    String built;
    for (auto& entry : m_shared->entries) {
      if (entry.begin > 0)
        built += "?";
      built += entry.string(*m_shared);
    }

    return built;
  }

  return {};
}

auto Directives::addToString(String& out) const -> String& {
  if (!empty())
    out += m_shared->string;
  return out;
}

auto Directives::hash() const -> std::size_t {
  return m_shared ? m_shared->hash : 0;
}

auto Directives::size() const -> std::size_t {
  return m_shared ? m_shared->entries.size() : 0;
}

auto Directives::empty() const -> bool {
  return !m_shared || m_shared->empty();
}

Directives::operator bool() const {
  return !empty();
}

auto Directives::equals(Directives const& other) const -> bool {
  return hash() == other.hash();
}

auto Directives::equals(String const& string) const -> bool {
  auto directiveString = stringPtr();
  return directiveString ? string == *directiveString : string.empty();
}

auto Directives::operator==(Directives const& other) const -> bool {
  return equals(other);
}

auto Directives::operator==(String const& string) const -> bool {
  return equals(string);
}

auto Directives::operator!=(Directives const& other) const -> bool {
  return !equals(other);
}

auto Directives::operator!=(String const& string) const -> bool {
  return !equals(string);
}

auto operator>>(DataStream& ds, Directives& directives) -> DataStream& {
  String string;
  ds.read(string);

  directives.parse(std::move(string));

  return ds;
}

auto operator<<(DataStream& ds, Directives const& directives) -> DataStream& {
  if (directives)
    ds.write(directives->string);
  else
    ds.write(String());

  return ds;
}

auto operator==(Directives const& d1, Directives const& d2) -> bool {
  return d1.equals(d2);
}

auto operator==(String const& string, Directives const& directives) -> bool {
  return directives.equals(string);
}

auto operator==(Directives const& directives, String const& string) -> bool {
  return directives.equals(string);
}

DirectivesGroup::DirectivesGroup() = default;
DirectivesGroup::DirectivesGroup(String const& directives) {
  if (directives.empty())
    return;

  Directives parsed(directives);
  if (parsed) {
    m_directives.emplace_back(std::move(parsed));
    m_count = m_directives.back().size();
  }
}
DirectivesGroup::DirectivesGroup(String&& directives) {
  if (directives.empty()) {
    directives.clear();
    return;
  }

  Directives parsed(std::move(directives));
  if (parsed) {
    m_directives.emplace_back(std::move(parsed));
    m_count = m_directives.back().size();
  }
}

auto DirectivesGroup::empty() const -> bool {
  return m_count == 0;
}

DirectivesGroup::operator bool() const {
  return empty();
}

auto DirectivesGroup::compare(DirectivesGroup const& other) const -> bool {
  if (m_count != other.m_count)
    return false;

  if (empty())
    return true;

  return hash() == other.hash();
}

void DirectivesGroup::append(Directives const& directives) {
  m_directives.emplace_back(directives);
  m_count += m_directives.back().size();
}

void DirectivesGroup::clear() {
  m_directives.clear();
  m_count = 0;
}

auto DirectivesGroup::operator+=(Directives const& other) -> DirectivesGroup& {
  append(other);
  return *this;
}

auto DirectivesGroup::toString() const -> String {
  String string;
  addToString(string);
  return string;
}

void DirectivesGroup::addToString(String& string) const {
  for (auto& entry : m_directives) {
    if (entry && !entry->string.empty()) {
      if (!string.empty() && string.utf8().back() != '?' && entry->string.utf8()[0] != '?')
        string.append('?');
      string += entry->string;
    }
  }
}

void DirectivesGroup::forEach(DirectivesCallback callback) const {
  for (auto& directives : m_directives) {
    if (directives) {
      for (auto& entry : directives->entries)
        callback(entry, directives);
    }
  }
}

auto DirectivesGroup::forEachAbortable(AbortableDirectivesCallback callback) const -> bool {
  for (auto& directives : m_directives) {
    if (directives) {
      for (auto& entry : directives->entries) {
        if (!callback(entry, directives))
          return false;
      }
    }
  }

  return true;
}

auto DirectivesGroup::applyNewImage(Image const& image, ImageReferenceCallback refCallback) const -> Image {
  Image result = image;
  applyExistingImage(result, refCallback);
  return result;
}

void DirectivesGroup::applyExistingImage(Image& image, ImageReferenceCallback refCallback) const {
  bool first = true;
  forEach([&](Directives::Entry const& entry, Directives const& directives) -> void {
    ImageOperation const& operation = entry.loadOperation(*directives);
    if (auto error = operation.ptr<ErrorImageOperation>())
      if (auto string = error->cause.ptr<std::string>())
        throw DirectivesException::format("ImageOperation parse error: {}", *string);
      else
        std::rethrow_exception(error->cause.get<std::exception_ptr>());
    else if (!first && entry.begin != 0 && operation.is<NullImageOperation>())
      throw DirectivesException::format("Invalid image operation: {}", entry.string(*directives));
    else
      processImageOperation(operation, image, refCallback);
    first = false;
  });
}

auto DirectivesGroup::hash() const -> std::size_t {
  std::size_t seed = 233;
  for (auto const& directives : m_directives) {
    hashCombine(seed, directives.hash());
  }
  return seed;
}

auto DirectivesGroup::list() const -> const List<Directives>& {
  return m_directives;
}

auto operator==(DirectivesGroup const& a, DirectivesGroup const& b) -> bool {
  return a.compare(b);
}

auto operator!=(DirectivesGroup const& a, DirectivesGroup const& b) -> bool {
  return !a.compare(b);
}

auto operator>>(DataStream& ds, DirectivesGroup& directivesGroup) -> DataStream& {
  String string;
  ds.read(string);

  directivesGroup = DirectivesGroup(std::move(string));

  return ds;
}

auto operator<<(DataStream& ds, DirectivesGroup const& directivesGroup) -> DataStream& {
  ds.write(directivesGroup.toString());

  return ds;
}

auto hash<DirectivesGroup>::operator()(DirectivesGroup const& s) const -> std::size_t {
  return s.hash();
}

}// namespace Star
