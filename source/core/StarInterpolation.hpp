#pragma once

#include "StarAlgorithm.hpp"
#include "StarArray.hpp"
#include "StarMathCommon.hpp"

import std;

namespace Star {

enum class BoundMode {
  Clamp,
  Extrapolate,
  Wrap
};

enum class InterpolationMode {
  HalfStep,
  Linear,
  Cubic
};

template <typename T1, typename T2>
auto angleLerp(T1 const& offset, T2 const& f0, T2 const& f1) -> T2 {
  return f0 + angleDiff(f0, f1) * offset;
}

template <typename T1, typename T2>
auto sinEase(T1 const& offset, T2 const& f0, T2 const& f1) -> T2 {
  T1 w = (std::sin(offset * Constants::pi - Constants::pi / 2) + 1) / 2;
  return f0 * (1 - w) + f1 * w;
}

template <typename T1, typename T2>
auto lerp(T1 const& offset, T2 const& f0, T2 const& f1) -> T2 {
  return f0 * (1 - offset) + f1 * (offset);
}

template <typename T1, typename T2>
auto lerpWithLimit(std::optional<T2> const& limit, T1 const& offset, T2 const& f0, T2 const& f1) -> T2 {
  if (limit && std::abs(f1 - f0) > *limit)
    return f1;
  return std::lerp(offset, f0, f1);
}

template <typename T1, typename T2>
auto step(T1 threshold, T1 x, T2 a, T2 b) -> T2 {
  if (x < threshold)
    return a;
  else
    return b;
}

template <typename T1, typename T2>
auto halfStep(T1 x, T2 a, T2 b) -> T2 {
  if (x < 0.5)
    return a;
  else
    return b;
}

template <typename T1, typename T2>
auto cubic4(T1 const& x, T2 const& f0, T2 const& f1, T2 const& f2, T2 const& f3) -> T2 {
  // (-1/2 * f0 +  3/2 * f1 + -3/2 * f2 +  1/2 * f3) * x * x * x +
  // (   1 * f0 + -5/2 * f1 +    2 * f2 + -1/2 * f3) * x * x +
  // (-1/2 * f0 +    0 * f1 +  1/2 * f2 +    0 * f3) * x +
  // (   0 * f0 +    1 * f1 +    0 * f2 +    0 * f3) * 1.0
  return f1 + (f2 - f0 + (f0 * 2.0 - f1 * 5.0 + f2 * 4.0 - f3 + ((f1 - f2) * 3.0 + f3 - f0) * x) * x) * x * 0.5;
}

template <typename T1, typename T2>
auto catmulRom4(T1 const& x, T2 const& f0, T2 const& f1, T2 const& f2, T2 const& f3) -> T2 {
  return ((f1 * 2) + (-f0 + f2) * x + (f0 * 2 - f1 * 5 + f2 * 4 - f3) * x * x
          + (-f0 + f1 * 3 - f2 * 3 + f3) * x * x * x)
    * 0.5;
}

template <typename T1, typename T2>
auto hermite2(T1 const& x, T2 const& a, T2 const& b) -> T2 {
  return a + (b - a) * x * x * (3 - 2 * x);
}

template <typename T1, typename T2>
auto quintic2(T1 const& x, T2 const& a, T2 const& b) -> T2 {
  return a + (b - a) * x * x * x * (x * (x * 6 - 15) + 10);
}

template <typename WeightT>
struct LinearWeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 2>;

  auto operator()(Weight x) const -> WeightVec {
    return {1 - x, x};
  }
};

template <typename WeightT>
struct StepWeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 2>;

  StepWeightOperator(Weight threshold = 0.5) : threshold(threshold) {}

  auto operator()(Weight x) const -> WeightVec {
    if (x < threshold)
      return {1, 0};
    else
      return {0, 1};
  }

  Weight threshold;
};

template <typename WeightT>
struct SinWeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 2>;

  auto operator()(Weight x) const -> WeightVec {
    Weight w = (sin(x * Constants::pi - Constants::pi / 2) + 1) / 2;
    return {1 - w, w};
  }
};

template <typename WeightT>
struct Hermite2WeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 2>;

  auto operator()(Weight x) const -> WeightVec {
    Weight w = x * x * (3 - 2 * x);
    return {1 - w, w};
  }
};

template <typename WeightT>
struct Quintic2WeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 2>;

  auto operator()(Weight x) const -> WeightVec {
    Weight w = x * x * x * (x * (x * 6 - 15) + 10);
    return {1 - w, w};
  }
};

// Setting 'LinearExtrapolate' flag to true changes the weights to be linear
// when x is outside of the range [0.0, 1.0]
template <typename WeightT>
struct Cubic4WeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 4>;

  Cubic4WeightOperator(bool le = false) : linearExtrapolate(le) {}

  auto operator()(Weight x) const -> WeightVec {
    if (linearExtrapolate && x > 1) {
      return {0, 0, 2 - x, x - 1};
    } else if (linearExtrapolate && x < 0) {
      return {-x, 1 + x, 0, 0};
    } else {
      // (-1/2 * f0 +  3/2 * f1 + -3/2 * f2 +  1/2 * f3) * x*x*x +
      // (   1 * f0 + -5/2 * f1 +    2 * f2 + -1/2 * f3) * x*x +
      // (-1/2 * f0 +    0 * f1 +  1/2 * f2 +    0 * f3) * x +
      // (   0 * f0 +    1 * f1 +    0 * f2 +    0 * f3) * 1.0

      Weight x2 = x * x;
      Weight x3 = x2 * x;
      return WeightVec(-0.5 * x3 + 1 * x2 - 0.5 * x,
                       1.5 * x3 + -2.5 * x2 + 1.0,
                       -1.5 * x3 + 2.0 * x2 + 0.5 * x,
                       0.5 * x3 - 0.5 * x2);
    }
  }
  bool linearExtrapolate;
};

// Setting 'LinearExtrapolate' flag to true changes the weights to be linear
// when x is outside of the range [0.0, 1.0]
template <typename WeightT>
struct Catmul4WeightOperator {
  using Weight = WeightT;
  using WeightVec = Array<Weight, 4>;

  Catmul4WeightOperator(bool le = false) : linearExtrapolate(le) {}

  auto operator()(Weight x) const -> WeightVec {
    if (linearExtrapolate && x > 1) {
      return {0, 0, 2 - x, x - 1};
    } else if (linearExtrapolate && x < 0) {
      return {-x, 1 + x, 0, 0};
    } else {
      Weight x2 = x * x;
      Weight x3 = x * x * x;
      return {(-x3 + x2 * 2 - x) / 2, (x3 * 3 - x2 * 5 + 2) / 2, (-x3 * 3 + x2 * 4 + x) / 2, (x3 - x2) / 2};
    }
  }

  bool linearExtrapolate;
};

template <typename Loctype, typename IndexType>
struct Bound2 {
  IndexType i0;
  IndexType i1;
  Loctype offset;
};

// loc should be in "index space", meaning that 0 points exactly to the first
// element and extent - 1
// points exactly to the last element.
template <typename LocType, typename IndexType>
auto getBound2(LocType loc, IndexType extent, BoundMode bmode) -> Bound2<LocType, IndexType> {
  Bound2<LocType, IndexType> bound;
  if (extent <= 1) {
    bound.i0 = bound.i1 = bound.offset = 0;
    return bound;
  }

  bound.offset = 0;
  if (bmode == BoundMode::Wrap) {
    loc = pfmod<LocType>(loc, extent);
  } else {
    LocType newLoc = clamp<LocType>(loc, 0, extent - 1);
    if (bmode == BoundMode::Extrapolate)
      bound.offset += loc - newLoc;

    loc = newLoc;
  }

  bound.i0 = IndexType(loc);

  if (bound.i0 == extent - 1) {
    if (bmode == BoundMode::Wrap) {
      bound.i1 = 0;
    } else {
      bound.i1 = bound.i0;
      bound.i0 -= 1;
    }
  } else {
    bound.i1 = bound.i0 + 1;
  }

  bound.offset += loc - bound.i0;

  return bound;
}

template <typename Loctype, typename IndexType>
struct Bound4 {
  Bound4() = default;
  IndexType i0;
  IndexType i1;
  IndexType i2;
  IndexType i3;
  Loctype offset;
};

// loc should be in "index space", meaning that 0 points exactly to the first
// element and extent - 1
// points exactly to the last element.
template <typename LocType, typename IndexType>
auto getBound4(LocType loc, IndexType extent, BoundMode bmode) -> Bound4<LocType, IndexType> {
  Bound4<LocType, IndexType> bound;
  if (extent <= 1) {
    bound.i0 = bound.i1 = bound.i2 = bound.i3 = bound.offset = 0;
    return bound;
  }

  bound.offset = 0;
  if (bmode == BoundMode::Wrap) {
    loc = pfmod<LocType>(loc, extent);
  } else {
    LocType newLoc = clamp<LocType>(loc, 0, extent - 1);
    if (bmode == BoundMode::Extrapolate)
      bound.offset += loc - newLoc;

    loc = newLoc;
  }

  bound.i1 = IndexType(loc);

  if (bound.i1 == extent - 1) {
    if (bmode == BoundMode::Wrap) {
      bound.i0 = bound.i1 - 1;
      bound.i2 = 0;
      bound.i3 = 1;
    } else {
      bound.i1 = bound.i1 - 2;

      bound.i0 = bound.i1 - 1;
      bound.i2 = bound.i1 + 1;
      bound.i3 = bound.i2 + 1;
    }
  } else if (bound.i1 == extent - 2) {
    if (bmode == BoundMode::Wrap) {
      bound.i0 = bound.i1 - 1;
      bound.i2 = bound.i1 + 1;
      bound.i3 = 0;
    } else {
      bound.i1 = bound.i1 - 1;

      bound.i0 = bound.i1 - 1;
      bound.i2 = bound.i1 + 1;
      bound.i3 = bound.i2 + 1;
    }
  } else if (bound.i1 == 0) {
    if (bmode == BoundMode::Wrap) {
      bound.i0 = extent - 1;
      bound.i2 = bound.i1 + 1;
      bound.i3 = bound.i2 + 1;
    } else {
      bound.i1 = bound.i1 + 1;

      bound.i0 = bound.i1 - 1;
      bound.i2 = bound.i1 + 1;
      bound.i3 = bound.i2 + 1;
    }
  } else {
    bound.i0 = bound.i1 - 1;
    bound.i2 = bound.i1 + 1;
    bound.i3 = bound.i1 + 2;
  }

  bound.offset += loc - bound.i1;

  return bound;
}

template <typename Container, typename Pos, typename WeightOp>
auto listInterpolate2(
  Container const& cont, Pos x, WeightOp weightOp, BoundMode bmode = BoundMode::Clamp) -> typename Container::value_type {
  if (cont.size() == 0) {
    return typename Container::value_type();
  } else if (cont.size() == 1) {
    return cont[0];
  } else {
    auto bound = getBound2(x, cont.size(), bmode);
    auto weights = weightOp(bound.offset);
    return cont[bound.i0] * weights[0] + cont[bound.i1] * weights[1];
  }
}

template <typename Container, typename Pos, typename WeightOp>
auto listInterpolate4(
  Container const& cont, Pos x, WeightOp weightOp, BoundMode bmode = BoundMode::Clamp) -> typename Container::value_type {
  if (cont.size() == 0) {
    return typename Container::value_type();
  } else if (cont.size() == 1) {
    return cont[0];
  } else {
    auto bound = getBound4(x, cont.size(), bmode);
    auto weights = weightOp(bound.offset);
    return cont[bound.i0] * weights[0] + cont[bound.i1] * weights[1] + cont[bound.i2] * weights[2]
      + cont[bound.i3] * weights[3];
  }
}

// Returns an index value (not integer) that represents the value that, if
// passed in as an index to a simple linear interpolation of the given
// container, would yield the given value.  (In other words, this goes from
// function space to index space on a list of points). Useful for doing
// interpolation on functions that are unevenly spaced.  Given container must
// be sorted.  If there is an ambiguity on points due to repeat points, will
// choose the lower-most of the points.
template <typename Iterator, typename Pos, typename Comp, typename PosGetter>
auto inverseLinearInterpolateLower(Iterator begin, Iterator end, Pos t, Comp&& comp, PosGetter&& posGetter) -> Pos {
  // Container must be at least size 2 for this to make sense.
  if (begin == end || std::next(begin) == end)
    return Pos();

  Iterator i = std::lower_bound(std::next(begin), std::prev(end), t, std::forward<Comp>(comp));

  --i;
  Pos min = posGetter(*i);
  Pos max = posGetter(*(++i));
  Pos ipos = Pos(std::distance(begin, --i));

  Pos dist = max - min;
  if (dist == 0)
    return ipos;
  else
    return ipos + (t - min) / dist;
}

template <typename Iterator, typename Pos>
auto inverseLinearInterpolateLower(Iterator begin, Iterator end, Pos t) -> Pos {
  return inverseLinearInterpolateLower(begin, end, t, std::less<Pos>(), identity());
}

// Same as inverseLinearInterpolateLower, except chooses the upper most of the
// points in the ambiguous case.
template <typename Iterator, typename Pos, typename Comp, typename PosGetter>
auto inverseLinearInterpolateUpper(Iterator begin, Iterator end, Pos t, Comp&& comp, PosGetter&& posGetter) -> Pos {
  // Container must be at least size 2 for this to make sense.
  if (begin == end || std::next(begin) == end)
    return Pos();

  Iterator i = std::upper_bound(std::next(begin), std::prev(end), t, std::forward<Comp>(comp));

  --i;
  Pos min = posGetter(*i);
  Pos max = posGetter(*(++i));
  Pos ipos = Pos(std::distance(begin, --i));

  Pos dist = max - min;
  if (dist == 0)
    return ipos + 1;
  else
    return ipos + (t - min) / dist;
}

template <typename Iterator, typename Pos>
auto inverseLinearInterpolateUpper(Iterator begin, Iterator end, Pos t) -> Pos {
  return inverseLinearInterpolateUpper(begin, end, t, std::less<Pos>(), identity());
}

template <typename XContainer, typename YContainer, typename PositionType, typename WeightOp>
auto parametricInterpolate2(XContainer const& xvals,
                            YContainer const& yvals,
                            PositionType const& position,
                            WeightOp weightOp,
                            BoundMode bmode) -> typename YContainer::value_type {

  if (yvals.size() == 1)
    return yvals[0];

  PositionType ipos = inverseLinearInterpolateLower(xvals.begin(), xvals.end(), position);

  return listInterpolate2(yvals, ipos, weightOp, bmode);
}

template <typename XContainer, typename YContainer, typename PositionType, typename WeightOp>
auto parametricInterpolate4(XContainer const& xvals,
                            YContainer const& yvals,
                            PositionType const& position,
                            WeightOp weightOp,
                            BoundMode bmode) -> typename YContainer::value_type {

  if (yvals.size() == 1)
    return yvals[0];

  PositionType ipos = inverseLinearInterpolateLower(xvals.begin(), xvals.end(), position);

  return listInterpolate4(yvals, ipos, weightOp, bmode);
}

}// namespace Star
