#include <time.h>
#include <windows.h>
import star.format;
import star.thread;
import star.time;
import star.lexical_cast;

import std;
import star.string;

namespace Star {
namespace Time {

String printDateAndTime(std::int64_t epochTicks, String format) {
  // playing fast and loose with the standard here...
  std::time_t requestedTime = epochTicks / epochTickFrequency();
  struct std::tm* ptm;
  ptm = std::localtime(&requestedTime);

  return format.replaceTags(StringMap<String>{
      {"year", strf("{:04d}", ptm->tm_year + 1900)},
      {"month", strf("{:02d}", ptm->tm_mon + 1)},
      {"day", strf("{:02d}", ptm->tm_mday)},
      {"hours", strf("{:02d}", ptm->tm_hour)},
      {"minutes", strf("{:02d}", ptm->tm_min)},
      {"seconds", strf("{:02d}", ptm->tm_sec)},
      {"millis", strf("{:03d}", (epochTicks % epochTickFrequency()) / (epochTickFrequency() / 1000))},
    });
}

std::int64_t epochTicks() {
  FILETIME ft_now;
  GetSystemTimeAsFileTime(&ft_now);
  LONGLONG now = (LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL);
  now -= 116444736000000000LL;
  return now;
}

std::int64_t epochTickFrequency() {
  return 10000000LL;
}

}

struct MonotonicClock {
  MonotonicClock() {
    QueryPerformanceFrequency(&freq);
  };

  std::int64_t ticks() const {
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
  }

  std::int64_t frequency() const {
    return freq.QuadPart;
  }

  LARGE_INTEGER freq;
};

static MonotonicClock g_monotonicClock;

namespace Time {

std::int64_t monotonicTicks() {
  return g_monotonicClock.ticks();
}

std::int64_t monotonicTickFrequency() {
  return g_monotonicClock.frequency();
}

}


}
