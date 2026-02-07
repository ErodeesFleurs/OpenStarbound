#pragma once

#include "StarRandom.hpp"

import std;

namespace Star {

template <typename Item>
struct WeightedPool {
public:
  using ItemsType = std::pair<double, Item>;
  using ItemsList = List<ItemsType>;

  WeightedPool();

  template <typename Container>
  explicit WeightedPool(Container container);

  void add(double weight, Item item);
  void clear();

  auto items() const -> ItemsList const&;

  [[nodiscard]] auto size() const -> std::size_t;
  auto at(std::size_t index) const -> std::pair<double, Item> const&;
  [[nodiscard]] auto weight(std::size_t index) const -> double;
  auto item(std::size_t index) const -> Item const&;
  [[nodiscard]] auto empty() const -> bool;

  // Return item using the given randomness source
  auto select(RandomSource& rand) const -> Item;
  // Return item using the global randomness source
  auto select() const -> Item;
  // Return item using fast static randomness from the given seed
  auto select(std::uint64_t seed) const -> Item;

  // Return a list of n items which are selected uniquely (by index), where
  // n is the lesser of the desiredCount and the size of the pool.
  // This INFLUENCES PROBABILITIES so it should not be used where a
  // correct statistical distribution is required.
  auto selectUniques(std::size_t desiredCount) const -> List<Item>;
  auto selectUniques(std::size_t desiredCount, std::uint64_t seed) const -> List<Item>;

  auto selectIndex(RandomSource& rand) const -> std::size_t;
  [[nodiscard]] auto selectIndex() const -> std::size_t;
  [[nodiscard]] auto selectIndex(std::uint64_t seed) const -> std::size_t;

private:
  [[nodiscard]] auto selectIndex(double target) const -> std::size_t;

  ItemsList m_items;
  double m_totalWeight;
};

template <typename Item>
WeightedPool<Item>::WeightedPool()
    : m_totalWeight(0.0) {}

template <typename Item>
template <typename Container>
WeightedPool<Item>::WeightedPool(Container container)
    : WeightedPool() {
  for (auto const& pair : container)
    add(get<0>(pair), get<1>(pair));
}

template <typename Item>
void WeightedPool<Item>::add(double weight, Item item) {
  if (weight <= 0.0)
    return;

  m_items.append({weight, std::move(item)});
  m_totalWeight += weight;
}

template <typename Item>
void WeightedPool<Item>::clear() {
  m_items.clear();
  m_totalWeight = 0.0;
}

template <typename Item>
auto WeightedPool<Item>::items() const -> ItemsList const& {
  return m_items;
}

template <typename Item>
auto WeightedPool<Item>::size() const -> std::size_t {
  return m_items.count();
}

template <typename Item>
auto WeightedPool<Item>::at(std::size_t index) const -> std::pair<double, Item> const& {
  return m_items.at(index);
}

template <typename Item>
auto WeightedPool<Item>::weight(std::size_t index) const -> double {
  return at(index).first;
}

template <typename Item>
auto WeightedPool<Item>::item(std::size_t index) const -> Item const& {
  return at(index).second;
}

template <typename Item>
auto WeightedPool<Item>::empty() const -> bool {
  return m_items.empty();
}

template <typename Item>
auto WeightedPool<Item>::select(RandomSource& rand) const -> Item {
  if (m_items.empty())
    return Item();

  return m_items[selectIndex(rand)].second;
}

template <typename Item>
auto WeightedPool<Item>::select() const -> Item {
  if (m_items.empty())
    return Item();

  return m_items[selectIndex()].second;
}

template <typename Item>
auto WeightedPool<Item>::select(std::uint64_t seed) const -> Item {
  if (m_items.empty())
    return Item();

  return m_items[selectIndex(seed)].second;
}

template <typename Item>
auto WeightedPool<Item>::selectUniques(std::size_t desiredCount) const -> List<Item> {
  return selectUniques(desiredCount, Random::randu64());
}

template <typename Item>
auto WeightedPool<Item>::selectUniques(std::size_t desiredCount, std::uint64_t seed) const -> List<Item> {
  std::size_t targetCount = std::min(desiredCount, size());
  Set<std::size_t> indices;
  while (indices.size() < targetCount)
    indices.add(selectIndex(++seed));
  List<Item> result;
  for (std::size_t i : indices)
    result.append(m_items[i].second);
  return result;
}

template <typename Item>
auto WeightedPool<Item>::selectIndex(RandomSource& rand) const -> std::size_t {
  return selectIndex(rand.randd());
}

template <typename Item>
auto WeightedPool<Item>::selectIndex() const -> std::size_t {
  return selectIndex(Random::randd());
}

template <typename Item>
auto WeightedPool<Item>::selectIndex(std::uint64_t seed) const -> std::size_t {
  return selectIndex(staticRandomDouble(seed));
}

template <typename Item>
auto WeightedPool<Item>::selectIndex(double target) const -> std::size_t {
  if (m_items.empty())
    return std::numeric_limits<std::size_t>::max();

  // Test a randomly generated target against each weighted item in turn, and
  // see if that weighted item's weight value crosses the target.  This way, a
  // random item is picked from the list, but (roughly) weighted to be
  // proportional to its weight over the weight of all entries.
  //
  // TODO: This is currently O(n), but can easily be made O(log(n)) by using a
  // tree.  If this shows up in performance measurements, this is an obvious
  // improvement.

  double accumulatedWeight = 0.0f;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    accumulatedWeight += m_items[i].first / m_totalWeight;
    if (target <= accumulatedWeight)
      return i;
  }

  // If we haven't crossed the target, just assume floating point error has
  // caused us to not quite make it to the last item.
  return m_items.size() - 1;
}

}// namespace Star
