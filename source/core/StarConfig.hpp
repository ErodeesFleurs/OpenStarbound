#pragma once

#include "StarPch.hpp"
#include "StarObserverPtr.hpp"

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

using std::pair;

using std::tuple;
using std::tuple_element;
using std::get;
using std::tie;

using std::initializer_list;

using std::min;
using std::max;

using std::function;
using std::forward;
using std::mem_fn;
using std::ref;
using std::cref;

using std::prev;

using std::atomic;
using std::atomic_flag;

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

#ifndef NDEBUG
constexpr bool DebugEnabled = true;
#else
constexpr bool DebugEnabled = false;
#endif

// A version of string::npos that's used in general to mean "not a position"
// and is the largest value for size_t.
constexpr size_t NPos = static_cast<size_t>(-1);

using StreamOffset = int64_t;

}
