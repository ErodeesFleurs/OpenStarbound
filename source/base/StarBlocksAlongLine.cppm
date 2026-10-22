module;
import std;
import star.format;

import star.vector;

import star.math_common;
namespace Star {

// Iterate over integral cells based on Bresenham's line drawing algorithm.
// Returns false immediately when the callback returns false for any cell,
// returns true after iterating through every cell otherwise.
template <typename Scalar>
[[nodiscard]] std::expected<bool, StarException> forBlocksAlongLine(Vector<Scalar, 2> origin, Vector<Scalar, 2> const& dxdy, std::function<std::expected<bool, StarException>(int, int)> callback) {
  if (!callback)
    return std::unexpected(StarException("Empty block traversal callback"));
  Vector<Scalar, 2> remote = origin + dxdy;

  double dx = dxdy[0];
  if (dx < 0)
    dx *= -1;

  double dy = dxdy[1];
  if (dy < 0)
    dy *= -1;

  double oxfloor = std::floor(origin[0]);
  double oyfloor = std::floor(origin[1]);
  double rxfloor = std::floor(remote[0]);
  double ryfloor = std::floor(remote[1]);

  if (dx == 0) {
    if (oyfloor < ryfloor) {
      for (int i = oyfloor; i <= ryfloor; ++i) {
        auto visited = callback(oxfloor, i);
        if (!visited) return std::unexpected(std::move(visited.error()));
        if (!*visited)
          return false;
      }
    } else {
      for (int i = oyfloor; i >= ryfloor; --i) {
        auto visited = callback(oxfloor, i);
        if (!visited) return std::unexpected(std::move(visited.error()));
        if (!*visited)
          return false;
      }
    }
    return true;

  } else if (dy == 0) {
    if (oxfloor < rxfloor) {
      for (int i = oxfloor; i <= rxfloor; ++i) {
        auto visited = callback(i, oyfloor);
        if (!visited) return std::unexpected(std::move(visited.error()));
        if (!*visited)
          return false;
      }
    } else {
      for (int i = oxfloor; i >= rxfloor; --i) {
        auto visited = callback(i, oyfloor);
        if (!visited) return std::unexpected(std::move(visited.error()));
        if (!*visited)
          return false;
      }
    }
    return true;

  } else {
    int x = oxfloor;
    int y = oyfloor;

    int n = 1;
    int x_inc, y_inc;
    double error;

    if (dxdy[0] > 0) {
      x_inc = 1;
      n += int(rxfloor) - x;
      error = (oxfloor + 1 - origin[0]) * dy;
    } else {
      x_inc = -1;
      n += x - int(rxfloor);
      error = (origin[0] - oxfloor) * dy;
    }

    if (dxdy[1] > 0) {
      y_inc = 1;
      n += int(ryfloor) - y;
      error -= (oyfloor + 1 - origin[1]) * dx;
    } else {
      y_inc = -1;
      n += y - int(ryfloor);
      error -= (origin[1] - oyfloor) * dx;
    }

    for (; n > 0; --n) {
      auto visited = callback(x, y);
      if (!visited) return std::unexpected(std::move(visited.error()));
      if (!*visited)
        return false;

      if (error > 0) {
        y += y_inc;
        error -= dx;
      } else if (error < 0) {
        x += x_inc;
        error += dy;
      } else {
        --n;
        y += y_inc;
        x += x_inc;
        error += dy;
        error -= dx;
      }
    }
    return true;
  }
}

}

export module star.blocks_along_line;

export namespace Star {
  using ::Star::forBlocksAlongLine;
}
