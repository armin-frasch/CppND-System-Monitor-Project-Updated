#include "format.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

std::string Format::ElapsedTime(long s) {
  /*
  std::ostringstream hours, minutes, secs; hours <<
  std::setfill('0') << std::setw(2) << (seconds / 3600); seconds = seconds %
  3600; minutes << std::setfill('0') << std::setw(2) << seconds / 60; secs <<
  std::setfill('0') << std::setw(2) << seconds % 60; return hours.str() + ":" +
  minutes.str() + ":" + secs.str();

  old implementation. Due to Udacitys Review Comment changed to the more
  expressive solution below. Copied line by line from the Review suggestion.
  */

  // return std::chrono::format("%T", seconds); // in C++20
  std::chrono::seconds seconds{s};

  std::chrono::hours hours =
      std::chrono::duration_cast<std::chrono::hours>(seconds);

  seconds -= std::chrono::duration_cast<std::chrono::seconds>(hours);

  std::chrono::minutes minutes =
      std::chrono::duration_cast<std::chrono::minutes>(seconds);

  seconds -= std::chrono::duration_cast<std::chrono::seconds>(minutes);

  std::stringstream ss{};

  ss << std::setw(2) << std::setfill('0') << hours.count()     // HH
     << std::setw(1) << ":"                                    // :
     << std::setw(2) << std::setfill('0') << minutes.count()   // MM
     << std::setw(1) << ":"                                    // :
     << std::setw(2) << std::setfill('0') << seconds.count();  // SS

  return ss.str();
}