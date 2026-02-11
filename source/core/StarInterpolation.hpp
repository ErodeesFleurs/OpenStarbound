#pragma once

#include "StarMathCommon.hpp"

import std;

namespace Star {

enum class BoundMode : std::uint8_t {
	Clamp,
	Extrapolate,
	Wrap
};

enum class InterpolationMode : std::uint8_t {
	HalfStep,
	Linear,
	Cubic
};

// Base interpolation functions

template <typename T1, typename T2>
[[nodiscard]] constexpr auto angle_lerp(T1 offset, T2 f0, T2 f1) -> T2 {
	return f0 + angle_diff(f0, f1) * offset;
}

template <typename T1, typename T2>
[[nodiscard]] constexpr auto sin_ease(T1 offset, T2 f0, T2 f1) -> T2 {
	constexpr auto pi = std::numbers::pi_v<std::common_type_t<T1, float>>;
	T1 w = (std::sin(offset * pi - pi / 2) + 1) / 2;
	return f0 * (1 - w) + f1 * w;
}

template <typename T1, typename T2>
[[nodiscard]] constexpr auto lerp(T1 offset, T2 f0, T2 f1) -> T2 {
	if constexpr (std::floating_point<T2> && std::convertible_to<T1, T2>) {
		return std::lerp(f0, f1, static_cast<T2>(offset));
	} else {
		return f0 * (1 - offset) + f1 * offset;
	}
}

template <typename T1, typename T2>
[[nodiscard]] constexpr auto lerp_with_limit(std::optional<T2> const& limit, T1 offset, T2 f0, T2 f1) -> T2 {
	if (limit && std::abs(f1 - f0) > *limit) {
		return f1;
	}
	return lerp(offset, f0, f1);
}

// Weight Operators

template <std::floating_point WeightT>
struct LinearWeightOperator {
	using WeightVec = std::array<WeightT, 2>;
	[[nodiscard]] constexpr auto operator()(WeightT x) const -> WeightVec {
		return {1 - x, x};
	}
};

template <std::floating_point WeightT>
struct StepWeightOperator {
	WeightT threshold = 0.5;
	[[nodiscard]] constexpr auto operator()(WeightT x) const -> std::array<WeightT, 2> {
		return (x < threshold) ? std::array<WeightT, 2>{1, 0} : std::array<WeightT, 2>{0, 1};
	}
};

template <std::floating_point WeightT>
struct SinWeightOperator {
	[[nodiscard]] constexpr auto operator()(WeightT x) const -> std::array<WeightT, 2> {
		constexpr auto pi = std::numbers::pi_v<WeightT>;
		WeightT w = (std::sin(x * pi - pi / 2) + 1) / 2;
		return {1 - w, w};
	}
};

// Bound Logic

template <typename Loctype, typename IndexType>
struct Bound2 {
	IndexType i0, i1;
	Loctype offset;
};

template <std::floating_point LocType, std::integral IndexType>
[[nodiscard]] constexpr auto get_bound2(LocType loc, IndexType extent, BoundMode bmode) -> Bound2<LocType, IndexType> {
	if (extent <= 1) {
		return {0, 0, 0};
	}

	LocType offset = 0;
	if (bmode == BoundMode::Wrap) {
		loc = pfmod<LocType>(loc, static_cast<LocType>(extent));
	} else {
		LocType newLoc = std::clamp<LocType>(loc, 0, static_cast<LocType>(extent - 1));
		if (bmode == BoundMode::Extrapolate) {
			offset += loc - newLoc;
		}
		loc = newLoc;
	}

	auto i0 = static_cast<IndexType>(loc);
	IndexType i1 = (i0 >= extent - 1) ? (bmode == BoundMode::Wrap ? 0 : i0) : i0 + 1;
	if (i0 == extent - 1 && bmode != BoundMode::Wrap) {
		i0--;
	}

	return {i0, i1, offset + (loc - static_cast<LocType>(i0))};
}

// List Interpolation

template <typename Container, typename Pos, typename WeightOp>
[[nodiscard]] constexpr auto listInterpolate2(
  Container const& cont, Pos x, WeightOp&& weightOp, BoundMode bmode = BoundMode::Clamp) -> typename Container::value_type {
	if (cont.empty()) {
		return {};
	}
	if (cont.size() == 1) {
		return cont[0];
	}

	auto bound = getBound2(x, cont.size(), bmode);
	auto weights = weightOp(bound.offset);
	return cont[bound.i0] * weights[0] + cont[bound.i1] * weights[1];
}

// Inverse Interpolation

template <typename Iterator, typename Pos, typename Comp = std::less<>, typename PosGetter = std::identity>
[[nodiscard]] constexpr auto inverseLinearInterpolateLower(
  Iterator begin, Iterator end, Pos t, Comp comp = {}, PosGetter posGetter = {}) -> Pos {
	if (begin == end || std::next(begin) == end) {
		return Pos{};
	}

	auto r = std::ranges::subrange(std::next(begin), std::prev(end));
	auto i = std::ranges::lower_bound(r, t, comp, posGetter);

	auto prev_i = std::prev(i);
	Pos min = std::invoke(posGetter, *prev_i);
	Pos max = std::invoke(posGetter, *i);
	Pos ipos = static_cast<Pos>(std::distance(begin, prev_i));

	Pos dist = max - min;
	return (dist == 0) ? ipos : ipos + (t - min) / dist;
}

}// namespace Star
