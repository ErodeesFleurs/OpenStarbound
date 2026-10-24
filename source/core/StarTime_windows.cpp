#include <time.h>
import star.format;
import star.time;

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


}


}
