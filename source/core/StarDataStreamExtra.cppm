module;

export module star.data_stream_extra;

import std;
import star.data_stream;
import star.poly;
import star.color;
import star.line;
import star.vector;
import star.array;
import star.matrix3;
import star.rect;

namespace star {

template <typename ElementT, std::size_t SizeN>
auto operator<<(data_stream& ds, star::array<ElementT, SizeN> const& array) -> data_stream& {
    for (auto const& element : array) {
        ds << element;
    }
    return ds;
}

template <typename ElementT, std::size_t SizeN>
auto operator>>(data_stream& ds, star::array<ElementT, SizeN>& array) -> data_stream& {
    for (auto& element : array) {
        ds >> element;
    }
    return ds;
}

template <typename T, std::size_t N>
auto operator<<(data_stream& ds, star::vector<T, N> const& vector) -> data_stream& {
    for (std::size_t i = 0; i < N; ++i) {
        ds << vector[i];
    }
    return ds;
}

template <typename T, std::size_t N>
auto operator>>(data_stream& ds, star::vector<T, N>& vector) -> data_stream& {
    for (std::size_t i = 0; i < N; ++i) {
        ds >> vector[i];
    }
    return ds;
}

inline auto operator<<(data_stream& ds, color const& color) -> data_stream& {
    ds << color.to_rgbaf();
    return ds;
}

inline auto operator>>(data_stream& ds, color& color) -> data_stream& {
    color = color::rgbaf(ds.read<vec_4f>());
    return ds;
}

template <typename First, typename Second>
auto operator<<(data_stream& ds, std::pair<First, Second> const& pair) -> data_stream& {
    ds << pair.first;
    ds << pair.second;
    return ds;
}

template <typename First, typename Second>
auto operator>>(data_stream& ds, std::pair<First, Second>& pair) -> data_stream& {
    ds >> pair.first;
    ds >> pair.second;
    return ds;
}

template <typename Element>
auto operator<<(data_stream& ds, std::shared_ptr<Element> const& ptr) -> data_stream& {
    ds.p_write(ptr);
    return ds;
}

template <typename Element>
auto operator>>(data_stream& ds, std::shared_ptr<Element>& ptr) -> data_stream& {
    ds.p_read(ptr);
    return ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
auto operator>>(data_stream& ds, std::flat_map<Key, Value, Compare, Allocator>& map)
  -> data_stream& {
    ds.read_map_container(map);
    return ds;
}

template <typename Key, typename Value, typename Compare, typename Allocator>
auto operator<<(data_stream& ds, std::flat_map<Key, Value, Compare, Allocator> const& map)
  -> data_stream& {
    ds.write_map_container(map);
    return ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
auto operator>>(data_stream& ds, std::flat_map<Key, Value, Hash, Equals, Allocator>& map)
  -> data_stream& {
    ds.read_map_container(map);
    return ds;
}

template <typename Key, typename Value, typename Hash, typename Equals, typename Allocator>
auto operator<<(data_stream& ds, std::flat_map<Key, Value, Hash, Equals, Allocator> const& map)
  -> data_stream& {
    ds.write_map_container(map);
    return ds;
}

template <typename Value, typename Compare, typename Allocator>
auto operator>>(data_stream& ds, std::flat_set<Value, Compare, Allocator>& set) -> data_stream& {
    ds.read_container(set);
    return ds;
}

template <typename Value, typename Compare, typename Allocator>
auto operator<<(data_stream& ds, std::flat_set<Value, Compare, Allocator> const& set)
  -> data_stream& {
    ds.write_container(set);
    return ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
auto operator>>(data_stream& ds, std::flat_set<Value, Equals, Allocator>& set) -> data_stream& {
    ds.read_container(set);
    return ds;
}

template <typename Value, typename Hash, typename Equals, typename Allocator>
auto operator<<(data_stream& ds, std::flat_set<Value, Equals, Allocator> const& set)
  -> data_stream& {
    ds.write_container(set);
    return ds;
}

template <typename DataT>
auto operator<<(data_stream& ds, const star::polygon<DataT>& poly) -> data_stream& {
    ds.write_container(poly.vertexes());
    return ds;
}

template <typename DataT>
auto operator>>(data_stream& ds, star::polygon<DataT>& poly) -> data_stream& {
    ds.read_container(poly.vertexes());
    return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator<<(data_stream& ds, star::box<DataT, Dimensions> const& box) -> data_stream& {
    ds.write(box.min());
    ds.write(box.max());
    return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator>>(data_stream& ds, star::box<DataT, Dimensions>& box) -> data_stream& {
    ds.read(box.min());
    ds.read(box.max());
    return ds;
}

template <typename DataT>
auto operator<<(data_stream& ds, star::matrix3<DataT> const& mat3) -> data_stream& {
    ds.write(mat3[0]);
    ds.write(mat3[1]);
    ds.write(mat3[2]);
    return ds;
}

template <typename DataT>
auto operator>>(data_stream& ds, star::matrix3<DataT>& mat3) -> data_stream& {
    ds.read(mat3[0]);
    ds.read(mat3[1]);
    ds.read(mat3[2]);
    return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator<<(data_stream& ds, const star::line<DataT, Dimensions>& line) -> data_stream& {
    ds.write(line.min());
    ds.write(line.max());
    return ds;
}

template <typename DataT, std::size_t Dimensions>
auto operator>>(data_stream& ds, star::line<DataT, Dimensions>& line) -> data_stream& {
    ds.read(line.min());
    ds.read(line.max());
    return ds;
}

template <typename... Ts>
auto operator<<(data_stream& ds, std::variant<Ts...> const& v) -> data_stream& {
    ds.write_vlq_u(v.index());
    std::visit([&](auto const& val) -> auto { ds << val; }, v);
    return ds;
}

template <typename... Ts> auto operator>>(data_stream& ds, std::variant<Ts...>& v) -> data_stream& {
    std::size_t const idx = ds.read_vlq_u();
    if (idx >= sizeof...(Ts)) {
        throw data_stream_exception("Variant index out of bounds");
    }

    [&]<std::size_t... Is>(std::index_sequence<Is...>) -> auto {
        ((idx == Is ? (v.template emplace<Is>(), ds >> std::get<Is>(v), true) : false) || ...);
    }(std::make_index_sequence<sizeof...(Ts)>{});

    return ds;
}

// std::optional operators

template <typename T>
auto operator<<(data_stream& ds, std::optional<T> const& opt) -> data_stream& {
    if (opt) {
        ds << true << *opt;
    } else {
        ds << false;
    }
    return ds;
}

template <typename T> auto operator>>(data_stream& ds, std::optional<T>& opt) -> data_stream& {
    if (ds.read<bool>()) {
        opt.emplace();
        ds >> *opt;
    } else {
        opt.reset();
    }
    return ds;
}

// std::tuple operators

template <typename... Ts>
auto operator<<(data_stream& ds, std::tuple<Ts...> const& t) -> data_stream& {
    std::apply([&](auto const&... args) -> auto { ((ds << args), ...); }, t);
    return ds;
}

template <typename... Ts> auto operator>>(data_stream& ds, std::tuple<Ts...>& t) -> data_stream& {
    std::apply([&](auto&... args) -> auto { ((ds >> args), ...); }, t);
    return ds;
}

}// namespace star
