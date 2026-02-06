#pragma once

import std;

namespace Star {
    template <typename T> using Ptr = std::shared_ptr<T>;
    template <typename T> using ConstPtr = std::shared_ptr<const T>;
    template <typename T> using WeakPtr = std::weak_ptr<T>;
    template <typename T> using ConstWeakPtr = std::weak_ptr<const T>;
    template <typename T> using UPtr = std::unique_ptr<T>;
    template <typename T> using ConstUPtr = std::unique_ptr<const T>;
}
