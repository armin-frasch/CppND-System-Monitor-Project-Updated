#include "processor.h"

#include "linux_parser.h"

// Return the aggregated CPU utilization
float Processor::Utilization() {
  long total_new = LinuxParser::Jiffies();
  long active_new = LinuxParser::ActiveJiffies();
  long idle_new = LinuxParser::IdleJiffies();

  long total_old = totalTime_;
  long idle_old = idleTime_;
  idleTime_ = idle_new;
  activeTime_ = active_new;
  totalTime_ = total_new;

  float totalDelta = float(total_new) - float(total_old);
  float idleDelta = float(idle_new) - float(idle_old);
  if (totalDelta != 0) {
    return (totalDelta - idleDelta) / totalDelta;
  } else {
    return 0;
  }
}