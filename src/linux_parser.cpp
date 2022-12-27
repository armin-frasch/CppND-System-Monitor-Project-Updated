#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <typename TValue>
TValue parseFile(std::string key, std::string path) {
  TValue value;
  std::string foundKey;
  string line;

  if (!(path.empty())) {
    std::ifstream stream(path);
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        std::istringstream linestream(line);
        linestream >> foundKey;
        if (foundKey == key) {
          linestream >> value;
          break;
        }
      }
    }
    stream.close();
  }
  return value;
}

template <typename TValue>
TValue parseFile(std::string path) {
  TValue value;
  std::string line;

  if (!(path.empty())) {
    std::ifstream stream(path);
    if (stream.is_open()) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      linestream >> value;
    }
    stream.close();
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;

  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  filestream.close();

  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();

  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);

  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float memTotal = parseFile<float>(
      "MemTotal:", LinuxParser::kProcDirectory + LinuxParser::kMeminfoFilename);
  float memFree =
      parseFile<float>("MemAvailable:", LinuxParser::kProcDirectory +
                                            LinuxParser::kMeminfoFilename);

  return (memTotal - memFree) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  long systemUptime = parseFile<long>(LinuxParser::kProcDirectory + LinuxParser::kUptimeFilename);

  return systemUptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, value;
  vector<string> jiffies;

  std::ifstream stream(LinuxParser::kProcDirectory + std::to_string(pid) +
                       LinuxParser::kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      jiffies.push_back(value);
    }
  }
  stream.close();

  long activeTime;
  try {
    activeTime = std::stol(jiffies[13]) + std::stol(jiffies[14]) +
                 std::stol(jiffies[15]) + std::stol(jiffies[16]);
  } catch (...) {
    activeTime = 0;
  }

  return activeTime / sysconf(_SC_CLK_TCK);
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();

  long activeTime;
  try {
    activeTime = std::stol(jiffies[CPUStates::kUser_]) +
                 std::stol(jiffies[CPUStates::kNice_]) +
                 std::stol(jiffies[CPUStates::kSystem_]) +
                 std::stol(jiffies[CPUStates::kIRQ_]) +
                 std::stol(jiffies[CPUStates::kSoftIRQ_]) +
                 std::stol(jiffies[CPUStates::kSteal_]);
  } catch (...) {
    activeTime = 0;
  }

  return activeTime / sysconf(_SC_CLK_TCK);
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();

  long idleTime;
  try {
    idleTime = std::stol(jiffies[CPUStates::kIdle_]) +
               std::stol(jiffies[CPUStates::kIOwait_]);
  } catch (...) {
    idleTime = 0;
  }

  return idleTime / sysconf(_SC_CLK_TCK);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, cpu, value;
  vector<string> jiffies;

  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> value) {
      jiffies.push_back(value);
    }
  }
  stream.close();

  return jiffies;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int processes;

  processes = parseFile<int>(
      "processes", LinuxParser::kProcDirectory + LinuxParser::kStatFilename);

  return processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int processes;

  processes = parseFile<int>("procs_running", LinuxParser::kProcDirectory +
                                                  LinuxParser::kStatFilename);

  return processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string command  = parseFile<string>(LinuxParser::kProcDirectory + std::to_string(pid) + LinuxParser::kCmdlineFilename);

  return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  long mem = parseFile<long>("VmSize:", LinuxParser::kProcDirectory +
                                            std::to_string(pid) +
                                            LinuxParser::kStatusFilename);
  mem /= 1000;
  return std::to_string(mem);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string uid = parseFile<string>("Uid:", LinuxParser::kProcDirectory +
                                             std::to_string(pid) +
                                             LinuxParser::kStatusFilename);

  return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string id, not_used, foundName, line;
  string name;

  std::ifstream stream(LinuxParser::kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      linestream >> foundName >> not_used >> id;
      if (id == uid) {
        name = foundName;
        break;
      }
    }
  }
  stream.close();

  return name;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values;
  long starttime = 0;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
  }
  stream.close();

  try {
    starttime = std::stol(values[21]) / sysconf(_SC_CLK_TCK);
  } catch (...) {
    starttime = 0;
  }
  return starttime;
}
