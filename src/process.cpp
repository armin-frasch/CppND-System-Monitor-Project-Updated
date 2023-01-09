#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using namespace std;

Process::Process(int pid) {
  pid_ = pid;
  command_ = LinuxParser::Command(pid_);
  if (command_.length() > MAX_COMMAND_LENGTH) {
    command_ = command_.substr(0, MAX_COMMAND_LENGTH) + "...";
  }

  string ram = LinuxParser::Ram(pid_);
  try {
    ram_ = stol(ram);
  } catch (...) {
    ram_ = 0;
  }

  uptime_ = LinuxParser::UpTime() - LinuxParser::UpTime(pid_);
  user_ = LinuxParser::User(pid_);

  double processActivetime = LinuxParser::ActiveJiffies(pid_);
  if (UpTime() != 0) {
    cpuUtilization_ = processActivetime / UpTime();
  }
}

// Return this process's ID
int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpuUtilization_; }

// Return the command that generated this process
string Process::Command() const { return command_; }

// Return this process's memory utilization
string Process::Ram() const { return to_string(ram_); }

// Return the user (name) that generated this process
string Process::User() const { return user_; }

// Return the age of this process (in seconds)
// Due to the latest Udacity Review, the Process Constructor changed. The
// Process::Uptime returns the time from System Update minus Process Starttime.
// Link to Udacitys reference Doc:
// https://man7.org/linux/man-pages/man5/proc.5.html
long int Process::UpTime() const { return uptime_; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}