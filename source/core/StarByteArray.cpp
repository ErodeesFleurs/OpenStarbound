import std;
import star.format;
import star.string;
import star.byte_array;
import star.memory;
import star.encode;

import star.algorithm;
namespace Star {

ByteArray ByteArray::fromCString(char const* str) {
  return ByteArray(str, std::strlen(str));
}

ByteArray ByteArray::fromCStringWithNull(char const* str) {
  std::size_t len = std::strlen(str);
  ByteArray ba(str, len + 1);
  ba[len] = 0;
  return ba;
}

ByteArray ByteArray::withReserve(std::size_t capacity) {
  ByteArray bytes;
  bytes.reserve(capacity);
  return bytes;
}

ByteArray::ByteArray() {
  m_data = nullptr;
  m_capacity = 0;
  m_size = 0;
}

ByteArray::ByteArray(std::size_t dataSize, char c)
  : ByteArray() {
  fill(dataSize, c);
}

ByteArray::ByteArray(const char* data, std::size_t dataSize)
  : ByteArray() {
  append(data, dataSize);
}

ByteArray::ByteArray(ByteArray const& b)
  : ByteArray() {
  operator=(b);
}

ByteArray::ByteArray(ByteArray&& b) noexcept
  : ByteArray() {
  operator=(std::move(b));
}

ByteArray::~ByteArray() {
  reset();
}

ByteArray& ByteArray::operator=(ByteArray const& b) {
  if (&b != this) {
    clear();
    append(b);
  }

  return *this;
}

ByteArray& ByteArray::operator=(ByteArray&& b) noexcept {
  if (&b != this) {
    reset();

    m_data = take(b.m_data);
    m_capacity = take(b.m_capacity);
    m_size = take(b.m_size);
  }

  return *this;
}

void ByteArray::reset() {
  if (m_data) {
    Star::free(m_data, m_capacity);
    m_data = nullptr;
    m_capacity = 0;
    m_size = 0;
  }
}

void ByteArray::reserve(std::size_t newCapacity) {
  if (newCapacity > m_capacity) {
    if (!m_data) {
      auto newMem = (char*)Star::malloc(newCapacity);
      if (!newMem)
        throw MemoryException::format("Could not set new ByteArray capacity {}\n", newCapacity);
      m_data = newMem;
      m_capacity = newCapacity;
    } else {
      newCapacity = std::max({m_capacity * 2, newCapacity, (std::size_t)8});
      auto newMem = (char*)Star::realloc(m_data, newCapacity);
      if (!newMem)
        throw MemoryException::format("Could not set new ByteArray capacity {}\n", newCapacity);
      m_data = newMem;
      m_capacity = newCapacity;
    }
  }
}

void ByteArray::resize(std::size_t size, char f) {
  if (m_size == size)
    return;

  std::size_t oldSize = m_size;
  resize(size);
  for (std::size_t i = oldSize; i < m_size; ++i)
    (*this)[i] = f;
}

void ByteArray::fill(std::size_t s, char c) {
  if (s != NPos)
    resize(s);

  if (m_size == 0)
    return;

  std::memset(m_data, c, m_size);
}

void ByteArray::fill(char c) {
  fill(NPos, c);
}

ByteArray ByteArray::sub(std::size_t b, std::size_t s) const {
  if (b == 0 && s >= m_size) {
    return ByteArray(*this);
  } else if (b >= m_size) {
    return ByteArray();
  } else {
    // The sub range has to be clamped against what is left *after* the offset;
    // min(m_size, b + s) let the copy read past the end of the buffer.
    return ByteArray(m_data + b, std::min(s, m_size - b));
  }
}

ByteArray ByteArray::left(std::size_t s) const {
  return sub(0, s);
}

ByteArray ByteArray::right(std::size_t s) const {
  if (s > m_size)
    s = 0;
  else
    s = m_size - s;

  return sub(s, m_size);
}

void ByteArray::trimLeft(std::size_t s) {
  if (s >= m_size) {
    clear();
  } else {
    std::memmove(m_data, m_data + s, m_size - s);
    resize(m_size - s);
  }
}

void ByteArray::trimRight(std::size_t s) {
  if (s >= m_size)
    clear();
  else
    resize(m_size - s);
}

std::size_t ByteArray::diffChar(const ByteArray& b) const {
  std::size_t s = std::min(m_size, b.size());
  char* ac = m_data;
  char* bc = b.m_data;
  std::size_t i;
  for (i = 0; i < s; ++i) {
    if (ac[i] != bc[i])
      break;
  }

  return i;
}

int ByteArray::compare(const ByteArray& b) const {
  if (m_size == 0 && b.m_size == 0)
    return 0;

  if (m_size == 0)
    return -1;

  if (b.m_size == 0)
    return 1;

  std::size_t d = diffChar(b);
  if (d == m_size) {
    if (d != b.m_size)
      return -1;
    else
      return 0;
  }

  if (d == b.m_size) {
    if (d != m_size)
      return 1;
    else
      return 0;
  }

  unsigned char c1 = (*this)[d];
  unsigned char c2 = b[d];

  if (c1 < c2) {
    return -1;
  } else if (c1 > c2) {
    return 1;
  } else {
    return 0;
  }
}

ByteArray ByteArray::andWith(ByteArray const& rhs, bool extend) {
  return combineWith([](char a, char b) { return a & b; }, rhs, extend);
}

ByteArray ByteArray::orWith(ByteArray const& rhs, bool extend) {
  return combineWith([](char a, char b) { return a | b; }, rhs, extend);
}

ByteArray ByteArray::xorWith(ByteArray const& rhs, bool extend) {
  return combineWith([](char a, char b) { return a ^ b; }, rhs, extend);
}

void ByteArray::insert(std::size_t pos, char byte) {
  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((pos <= m_size));
  }
  resize(m_size + 1);
  for (std::size_t i = m_size - 1; i > pos; --i)
    m_data[i] = m_data[i - 1];
  m_data[pos] = byte;
}

ByteArray::iterator ByteArray::insert(const_iterator pos, char byte) {
  std::size_t d = pos - begin();
  insert(d, byte);
  return begin() + d + 1;
}

void ByteArray::push_back(char byte) {
  resize(m_size + 1);
  m_data[m_size - 1] = byte;
}

bool ByteArray::operator<(const ByteArray& b) const {
  return compare(b) < 0;
}

bool ByteArray::operator==(const ByteArray& b) const {
  return compare(b) == 0;
}

bool ByteArray::operator!=(const ByteArray& b) const {
  return compare(b) != 0;
}

std::ostream& operator<<(std::ostream& os, const ByteArray& b) {
  os << "0x" << hexEncode(b);
  return os;
}

}
