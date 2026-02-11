#pragma once

import std;

namespace Star {

// Multidimensional array class that wraps a vector as a simple contiguous N
// dimensional array.  Values are stored so that the highest dimension is the
// dimension with stride 0, and the lowest dimension has the largest stride.
//
// Due to usage of std::vector, ElementT = bool means that the user must use
// set() and get() rather than operator()
template <typename ElementT, std::size_t RankN>
    requires(RankN > 0)
class MultiArray {
  public:
	using Storage = std::vector<ElementT>;
	using Element = ElementT;
	static constexpr std::size_t Rank = RankN;

	using IndexArray = std::array<std::size_t, Rank>;
	using SizeArray = std::array<std::size_t, Rank>;

	using iterator = typename Storage::iterator;
	using const_iterator = typename Storage::const_iterator;
	using value_type = Element;

	constexpr MultiArray() = default;

	template <typename... T>
	    requires(sizeof...(T) == Rank) && (std::convertible_to<T, std::size_t> && ...)
	explicit constexpr MultiArray(T... dims) {
		set_size(SizeArray{static_cast<std::size_t>(dims)...});
	}

	explicit constexpr MultiArray(SizeArray const& shape) {
		set_size(shape);
	}

	explicit constexpr MultiArray(SizeArray const& shape, Element const& c) {
		set_size(shape, c);
	}

	[[nodiscard]] constexpr auto size() const -> SizeArray const& { return m_shape; }
	[[nodiscard]] constexpr auto size(std::size_t dimension) const -> std::size_t { return m_shape[dimension]; }

	constexpr void clear() {
		m_data.clear();
		m_shape.fill(0);
	}

	template <typename... Indices>
	    requires(sizeof...(Indices) == Rank) && (std::convertible_to<Indices, std::size_t> && ...)
	[[nodiscard]] constexpr auto operator[](this auto&& self, Indices... indices) -> auto&& {
		return std::forward<decltype(self)>(self).m_data[self.storageIndex({static_cast<std::size_t>(indices)...})];
	}

	[[nodiscard]] constexpr auto operator[](this auto&& self, IndexArray const& index) -> auto&& {
		return std::forward<decltype(self)>(self).m_data[self.storageIndex(index)];
	}

	template <typename... Indices>
	    requires(sizeof...(Indices) == Rank) && (std::convertible_to<Indices, std::size_t> && ...)
	[[nodiscard]] constexpr auto at(this auto&& self, Indices... indices) -> auto&& {
		IndexArray idx{static_cast<std::size_t>(indices)...};
		for (std::size_t i = 0; i < Rank; ++i) {
			if (idx[i] >= self.m_shape[i]) {
				std::unexpected("MultiArray::at() index out of bounds");
			}
		}
		return std::forward<decltype(self)>(self).m_data[self.storageIndex(idx)];
	}

	constexpr void fill(Element const& element) {
		std::ranges::fill(m_data, element);
	}

	constexpr void set_size(SizeArray const& shape) {
		m_shape = shape;
		std::size_t total = 1;
		for (auto s : m_shape) {
			total *= s;
		}
		m_data.resize(total);
	}

	constexpr void set_size(SizeArray const& shape, Element const& c) {
		m_shape = shape;
		std::size_t total = 1;
		for (auto s : m_shape) {
			total *= s;
		}
		m_data.resize(total, c);
	}

	[[nodiscard]] constexpr auto as_mdspan(this auto&& self) {
		auto make_extents = []<std::size_t... Is>(const SizeArray& s, std::index_sequence<Is...>) -> auto {
			return std::dextents<std::size_t, Rank>{s[Is]...};
		};
		auto extents = make_extents(self.m_shape, std::make_index_sequence<Rank>{});
		return std::mdspan(self.m_data.data(), extents);
	}

	[[nodiscard]] constexpr auto count() const -> std::size_t { return m_data.size(); }

	template <typename Self>
	[[nodiscard]] constexpr auto data(this Self&& self) -> auto* { return self.m_data.data(); }

	[[nodiscard]] constexpr auto storage_index(IndexArray const& index) const -> std::size_t {
		std::size_t loc = index[0];
		for (std::size_t i = 1; i < Rank; ++i) {
			loc = loc * m_shape[i] + index[i];
		}
		return loc;
	}

	template <typename OpType>
	constexpr void for_each(OpType&& op) {
		for (std::size_t i = 0; i < m_data.size(); ++i) {
			op(m_data[i]);
		}
	}

	auto operator==(MultiArray const&) const -> bool = default;

  private:
	Storage m_data;
	SizeArray m_shape;
};

using MultiArray2I = MultiArray<std::int32_t, 2>;
using MultiArray2S = MultiArray<std::size_t, 2>;
using MultiArray2U = MultiArray<std::uint32_t, 2>;
using MultiArray2F = MultiArray<std::float_t, 2>;
using MultiArray2D = MultiArray<std::double_t, 2>;

using MultiArray3I = MultiArray<std::int32_t, 3>;
using MultiArray3S = MultiArray<std::size_t, 3>;
using MultiArray3U = MultiArray<std::uint32_t, 3>;
using MultiArray3F = MultiArray<std::float_t, 3>;
using MultiArray3D = MultiArray<std::double_t, 3>;

using MultiArray4I = MultiArray<std::int32_t, 4>;
using MultiArray4S = MultiArray<std::size_t, 4>;
using MultiArray4U = MultiArray<std::uint32_t, 4>;
using MultiArray4F = MultiArray<std::float_t, 4>;
using MultiArray4D = MultiArray<std::double_t, 4>;

}// namespace Star
