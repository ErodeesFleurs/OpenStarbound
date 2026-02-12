export module star.bi_map;

import std;
import star.hash;

export namespace star {

/**
 * @brief Bi-directional map of unique sets of elements.
 *
 * Provides quick map access from either the left or right element to the other side.
 * Every left side value must be unique, as must every right side value.
 *
 * @tparam left_t Type of the left-side elements.
 * @tparam right_t Type of the right-side elements.
 * @tparam left_map_t The underlying map type for left-to-right lookup (default: std::map).
 * @tparam right_map_t The underlying map type for right-to-left lookup (default: std::map).
 */
template <typename left_t, typename right_t, typename left_map_t = std::map<left_t, right_t const*>,
          typename right_map_t = std::map<right_t, left_t const*>>
class bi_map {
  public:
    using left = left_t;
    using right = right_t;
    using left_map = left_map_t;
    using right_map = right_map_t;

    using value_type = std::pair<left, right>;

    struct bi_map_iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<left const&, right const&>;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = value_type;

        constexpr bi_map_iterator() = default;
        explicit constexpr bi_map_iterator(typename left_map::const_iterator it)
            : m_iterator(std::move(it)) {}

        constexpr auto operator++() -> bi_map_iterator& {
            ++m_iterator;
            return *this;
        }

        constexpr auto operator++(int) -> bi_map_iterator {
            auto last = *this;
            ++m_iterator;
            return last;
        }

        constexpr auto operator==(bi_map_iterator const& rhs) const -> bool = default;

        constexpr auto operator*() const -> value_type {
            return {m_iterator->first, *m_iterator->second};
        }

        typename left_map::const_iterator m_iterator;
    };

    using iterator = bi_map_iterator;
    using const_iterator = iterator;

    template <typename collection_t> static auto from(collection_t const& c) -> bi_map {
        return bi_map(c.begin(), c.end());
    }

    bi_map() = default;

    bi_map(bi_map const& other) {
        for (auto const& [l, r] : other) {
            insert(l, r);
        }
    }

    bi_map(bi_map&&) noexcept = default;

    template <typename input_iterator_t> bi_map(input_iterator_t beg, input_iterator_t end) {
        while (beg != end) {
            insert(*beg);
            ++beg;
        }
    }

    bi_map(std::initializer_list<value_type> list) {
        for (auto const& v : list) {
            if (!insert(v.first, v.second)) {
                throw std::runtime_error("Duplicate pair in bi_map initializer_list construction");
            }
        }
    }

    [[nodiscard]] auto left_values() const {
        std::vector<left> values;
        values.reserve(size());
        for (auto const& [l, r] : m_left_map) {
            values.push_back(l);
        }
        return values;
    }

    [[nodiscard]] auto right_values() const {
        std::vector<right> values;
        values.reserve(size());
        for (auto const& [r, l] : m_right_map) {
            values.push_back(r);
        }
        return values;
    }

    [[nodiscard]] auto pairs() const {
        std::vector<value_type> res;
        res.reserve(size());
        for (auto const& p : *this) {
            res.push_back(p);
        }
        return res;
    }

    [[nodiscard]] auto has_left_value(left const& l) const -> bool {
        return m_left_map.contains(l);
    }
    [[nodiscard]] auto has_right_value(right const& r) const -> bool {
        return m_right_map.contains(r);
    }

    [[nodiscard]] auto get_right(left const& l) const -> right const& { return *m_left_map.at(l); }
    [[nodiscard]] auto get_left(right const& r) const -> left const& { return *m_right_map.at(r); }

    [[nodiscard]] auto value_right(left const& l, right const& def = right()) const -> right {
        return maybe_right(l).value_or(def);
    }

    [[nodiscard]] auto value_left(right const& r, left const& def = left()) const -> left {
        return maybe_left(r).value_or(def);
    }

    [[nodiscard]] auto maybe_right(left const& l) const -> std::optional<right> {
        if (auto it = m_left_map.find(l); it != m_left_map.end()) {
            return *it->second;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto maybe_left(right const& r) const -> std::optional<left> {
        if (auto it = m_right_map.find(r); it != m_right_map.end()) {
            return *it->second;
        }
        return std::nullopt;
    }

    auto take_right(left const& l) -> right {
        auto res = maybe_take_right(l);
        if (!res) {
            throw std::out_of_range("bi_map::take_right: key not found");
        }
        return std::move(*res);
    }

    auto take_left(right const& r) -> left {
        auto res = maybe_take_left(r);
        if (!res) {
            throw std::out_of_range("bi_map::take_left: key not found");
        }
        return std::move(*res);
    }

    auto maybe_take_right(left const& l) -> std::optional<right> {
        if (auto it = m_left_map.find(l); it != m_left_map.end()) {
            right r = *it->second;
            m_right_map.erase(*it->second);
            m_left_map.erase(it);
            return r;
        }
        return std::nullopt;
    }

    auto maybe_take_left(right const& r) -> std::optional<left> {
        if (auto it = m_right_map.find(r); it != m_right_map.end()) {
            left l = *it->second;
            m_left_map.erase(*it->second);
            m_right_map.erase(it);
            return l;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto right_ptr(this auto&& self, left const& l) -> auto const* {
        auto it = self.m_left_map.find(l);
        return it != self.m_left_map.end() ? it->second : nullptr;
    }

    [[nodiscard]] auto left_ptr(this auto&& self, right const& r) -> auto const* {
        auto it = self.m_right_map.find(r);
        return it != self.m_right_map.end() ? it->second : nullptr;
    }

    auto operator=(bi_map const& other) -> bi_map& {
        if (this != &other) {
            clear();
            for (auto const& [l, r] : other) {
                insert(l, r);
            }
        }
        return *this;
    }

    auto operator=(bi_map&&) noexcept -> bi_map& = default;

    auto insert(value_type const& val) -> std::pair<iterator, bool> {
        if (m_left_map.contains(val.first) || m_right_map.contains(val.second)) {
            return {iterator{m_left_map.find(val.first)}, false};
        }

        auto [it_l, inserted_l] = m_left_map.insert({val.first, nullptr});
        auto [it_r, inserted_r] = m_right_map.insert({val.second, nullptr});

        // Cross-link pointers to the keys in the other map
        it_l->second = &it_r->first;
        it_r->second = &it_l->first;

        return {iterator{it_l}, true};
    }

    auto insert(left const& l, right const& r) -> bool {
        return insert(std::make_pair(l, r)).second;
    }

    void add(left const& l, right const& r) {
        if (!insert(l, r)) {
            throw std::runtime_error("bi_map::add: duplicate key collision");
        }
    }

    void add(value_type const& value) { add(value.first, value.second); }

    void overwrite(left const& l, right const& r) {
        remove_left(l);
        remove_right(r);
        insert(l, r);
    }

    void overwrite(value_type const& value) { overwrite(value.first, value.second); }

    auto remove_left(left const& l) -> bool {
        if (auto it = m_left_map.find(l); it != m_left_map.end()) {
            m_right_map.erase(*it->second);
            m_left_map.erase(it);
            return true;
        }
        return false;
    }

    auto remove_right(right const& r) -> bool {
        if (auto it = m_right_map.find(r); it != m_right_map.end()) {
            m_left_map.erase(*it->second);
            m_right_map.erase(it);
            return true;
        }
        return false;
    }

    [[nodiscard]] auto begin() const -> const_iterator {
        return const_iterator{m_left_map.begin()};
    }
    [[nodiscard]] auto end() const -> const_iterator { return const_iterator{m_left_map.end()}; }

    [[nodiscard]] auto size() const -> std::size_t { return m_left_map.size(); }
    void clear() {
        m_left_map.clear();
        m_right_map.clear();
    }
    [[nodiscard]] auto empty() const -> bool { return m_left_map.empty(); }

    auto operator==(bi_map const& m) const -> bool {
        if (this == &m) {
            return true;
        }
        if (size() != m.size()) {
            return false;
        }
        for (auto const& [l, r] : *this) {
            if (auto p = m.right_ptr(l)) {
                if (*p != r) {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }

  private:
    left_map m_left_map;
    right_map m_right_map;
};

template <typename left_t, typename right_t, typename left_hash_t = star::hash<left_t>,
          typename right_hash_t = star::hash<right_t>>
using bi_hash_map = bi_map<left_t, right_t, std::unordered_map<left_t, right_t const*, left_hash_t>,
                           std::unordered_map<right_t, left_t const*, right_hash_t>>;

template <typename enum_t>
    requires std::is_enum_v<enum_t>
using enum_map = bi_map<enum_t, std::string, std::map<enum_t, std::string const*>,
                        std::unordered_map<std::string, enum_t const*, case_insensitive_string_hash,
                                           case_insensitive_string_compare>>;

}// namespace star
