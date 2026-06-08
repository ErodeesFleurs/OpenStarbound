#pragma once

#include "StarPch.hpp"

namespace Star {

// Some really common std namespace includes

using std::size_t;

using std::swap;
using std::move;

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;
using std::make_unique;
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;
using std::enable_shared_from_this;

using std::pair;
using std::make_pair;

using std::tuple;
using std::make_tuple;
using std::tuple_element;
using std::get;
using std::tie;
using std::ignore;

using std::initializer_list;

using std::min;
using std::max;

using std::function;
using std::forward;
using std::mem_fn;
using std::ref;
using std::cref;
using namespace std::string_literals;

using std::prev;
// using std::next;

using std::atomic;
using std::atomic_flag;
using std::atomic_load;
using std::atomic_store;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

#ifndef NDEBUG
#define STAR_DEBUG 1
constexpr bool DebugEnabled = true;
#else
constexpr bool DebugEnabled = false;
#endif

// A version of string::npos that's used in general to mean "not a position"
// and is the largest value for size_t.
constexpr size_t NPos = static_cast<size_t>(-1);

using StreamOffset = int64_t;

#define STAR_QUOTE(name) #name
#define STAR_STR(macro) STAR_QUOTE(macro)

}
