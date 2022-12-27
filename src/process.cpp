#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  pid_ = pid;
  command_ = LinuxParser::Command(pid_);
  std::string ram = LinuxParser::Ram(pid_);
  try {
    ram_ = std::stol(ram);
  } catch (...) {
    ram_ = 0;
  }
  uptime_ = LinuxParser::UpTime(pid_);
  user_ = LinuxParser::User(pid_);

  long processIdletime = LinuxParser::UpTime() - uptime_;
  long processActivetime = LinuxParser::ActiveJiffies(pid_);
  cpuUtilization_ = float(processActivetime) / float(processIdletime);
}

// Return this process's ID
int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpuUtilization_; }

// Return the command that generated this process
string Process::Command() const { return command_; }

// Return this process's memory utilization
string Process::Ram() const { return std::to_string(ram_); }

// Return the user (name) that generated this process
string Process::User() const { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() const { return uptime_; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}