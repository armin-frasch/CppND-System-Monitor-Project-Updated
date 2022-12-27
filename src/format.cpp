#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  std::ostringstream hours, minutes, secs;
  hours << std::setfill('0') << std::setw(2) << (seconds / 3600);
  seconds = seconds % 3600;
  minutes << std::setfill('0') << std::setw(2) << seconds / 60;
  secs << std::setfill('0') << std::setw(2) << seconds % 60;
  return hours.str() + ":" + minutes.str() + ":" + secs.str();
}