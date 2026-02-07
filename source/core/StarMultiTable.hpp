#pragma once

#include "StarMultiArrayInterpolator.hpp"

import std;

namespace Star {

// Provides a method for storing, retrieving, and interpolating uneven
// n-variate data.  Access times involve a binary search over the domain of
// each dimension, so is O(log(n)*m) where n is the size of the largest
// dimension, and m is the table_rank.
template <typename ElementT, typename PositionT, size_t RankN>
class MultiTable {
public:
  using Element = ElementT;
  using Position = PositionT;
  static size_t const Rank = RankN;

  using MultiArray = Star::MultiArray<ElementT, RankN>;

  using Interpolator2 = Star::MultiArrayInterpolator2<MultiArray, Position>;
  using Interpolator4 = Star::MultiArrayInterpolator4<MultiArray, Position>;
  using PiecewiseInterpolator = Star::MultiArrayPiecewiseInterpolator<MultiArray, Position>;

  using PositionArray = Array<Position, Rank>;
  using WeightArray2 = Array<Position, 2>;
  using WeightArray4 = Array<Position, 4>;
  using SizeArray = typename MultiArray::SizeArray;
  using IndexArray = typename MultiArray::IndexArray;
  using Range = List<Position>;
  using RangeArray = Array<Range, Rank>;

  using WeightFunction2 = std::function<WeightArray2(Position)>;
  using WeightFunction4 = std::function<WeightArray4(Position)>;
  using InterpolateFunction = std::function<Element(PositionArray const&)>;

  MultiTable() = default;

  // Set input ranges on a particular dimension.  Will resize underlying storage
  // to fit range.
  void setRange(std::size_t dim, Range const& range) {
    SizeArray sizes = m_array.size();
    sizes[dim] = range.size();
    m_array.resize(sizes);

    m_ranges[dim] = range;
  }

  void setRanges(RangeArray const& ranges) {
    SizeArray arraySize;

    for (size_t dim = 0; dim < Rank; ++dim) {
      arraySize[dim] = ranges[dim].size();
      m_ranges[dim] = ranges[dim];
    }

    m_array.resize(arraySize);
  }

  // Set array element based on index.
  void set(IndexArray const& index, Element const& element) {
    m_array.set(index, element);
  }

  // Get array element based on index.
  auto get(IndexArray const& index) const -> Element const& {
    return m_array(index);
  }

  auto array() const -> MultiArray const& {
    return m_array;
  }

  auto array() -> MultiArray& {
    return m_array;
  }

  [[nodiscard]] auto interpolationMode() const -> InterpolationMode {
    return m_interpolationMode;
  }

  void setInterpolationMode(InterpolationMode interpolationMode) {
    m_interpolationMode = interpolationMode;
  }

  [[nodiscard]] auto boundMode() const -> BoundMode {
    return m_boundMode;
  }

  void setBoundMode(BoundMode boundMode) {
    m_boundMode = boundMode;
  }

  auto interpolate(PositionArray const& coord) const -> Element {
    if (m_interpolationMode == InterpolationMode::HalfStep) {
      PiecewiseInterpolator piecewiseInterpolator(StepWeightOperator<Position>(), m_boundMode);
      return piecewiseInterpolator.interpolate(m_array, toIndexSpace(coord));

    } else if (m_interpolationMode == InterpolationMode::Linear) {
      Interpolator2 interpolator2(LinearWeightOperator<Position>(), m_boundMode);
      return interpolator2.interpolate(m_array, toIndexSpace(coord));

    } else if (m_interpolationMode == InterpolationMode::Cubic) {
      // MultiTable uses CubicWeights with linear extrapolation (not
      // configurable atm)
      Interpolator4 interpolator4(Cubic4WeightOperator<Position>(true), m_boundMode);
      return interpolator4.interpolate(m_array, toIndexSpace(coord));

    } else {
      throw MathException("Unsupported interpolation type in MultiTable::interpolate");
    }
  }

  // Synonym for inteprolate
  auto operator()(PositionArray const& coord) const -> Element {
    return interpolate(coord);
  }

  // op should take a PositionArray parameter and return an element.
  template <typename OpType>
  void eval(OpType op) {
    m_array.forEach(EvalWrapper<OpType>(op, *this));
  }

private:
  template <typename Coordinate>
  inline auto toIndexSpace(Coordinate const& coord) const -> PositionArray {
    PositionArray indexCoord;
    for (size_t i = 0; i < Rank; ++i)
      indexCoord[i] = inverseLinearInterpolateLower(m_ranges[i].begin(), m_ranges[i].end(), coord[i]);
    return indexCoord;
  }

  template <typename OpType>
  struct EvalWrapper {
    EvalWrapper(OpType& o, MultiTable const& t)
        : op(o), table(t) {}

    template <typename IndexArray>
    void operator()(IndexArray const& indexArray, Element& element) {
      PositionArray rangeArray;
      for (size_t i = 0; i < Rank; ++i)
        rangeArray[i] = table.m_ranges[i][indexArray[i]];

      element = op(rangeArray);
    }

    OpType& op;
    MultiTable const& table;
  };

  RangeArray m_ranges;
  MultiArray m_array;
  InterpolationMode m_interpolationMode{};
  BoundMode m_boundMode{};
};

using MultiTable2F = MultiTable<float, float, 2>;
using MultiTable2D = MultiTable<double, double, 2>;

using MultiTable3F = MultiTable<float, float, 3>;
using MultiTable3D = MultiTable<double, double, 3>;

using MultiTable4F = MultiTable<float, float, 4>;
using MultiTable4D = MultiTable<double, double, 4>;

}// namespace Star
