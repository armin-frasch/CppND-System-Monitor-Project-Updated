#include "linux_parser.h"

#include <ctype.h>
#include <dirent.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <vector>

template <typename TValue>
TValue parseFile(std::string key, std::string path) {
  TValue value;
  std::string foundKey;
  std::string line;

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
std::string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value;

  std::ifstream filestream(LinuxParser::kOSPath);
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
std::string LinuxParser::Kernel() {
  std::string os, version, kernel;
  std::string line;
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();

  return kernel;
}

// BONUS: Update this to use filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  const std::filesystem::path dir{LinuxParser::kProcDirectory};

  for (auto const& dir_entry : std::filesystem::directory_iterator{dir}) {
    if (dir_entry.is_directory()) {
      std::string filename{dir_entry.path().filename()};
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        try {
          int pid = stoi(filename);
          pids.emplace_back(pid);
        } catch (...) {
          // do nothing
        }
      }
    }
  }

  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float memTotal =
      parseFile<float>(filterMemTotal, LinuxParser::kProcDirectory +
                                           LinuxParser::kMeminfoFilename);
  float memFree =
      parseFile<float>(filterMemAvailable, LinuxParser::kProcDirectory +
                                               LinuxParser::kMeminfoFilename);

  return (memTotal - memFree) / memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  long systemUptime = parseFile<long>(LinuxParser::kProcDirectory +
                                      LinuxParser::kUptimeFilename);

  return systemUptime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::string line, value;
  std::vector<std::string> jiffies;

  std::ifstream stream(LinuxParser::kProcDirectory + std::to_string(pid) +
                       LinuxParser::kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      jiffies.emplace_back(value);
    }
  }
  stream.close();

  long activeTime;
  try {
    if (jiffies.size() >= 16) {
      activeTime = std::stol(jiffies[13]) + std::stol(jiffies[14]) +
                   std::stol(jiffies[15]) + std::stol(jiffies[16]);
    }
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
    if (jiffies.size() >= CPUStates::kSteal_) {
      activeTime = std::stol(jiffies[CPUStates::kUser_]) +
                   std::stol(jiffies[CPUStates::kNice_]) +
                   std::stol(jiffies[CPUStates::kSystem_]) +
                   std::stol(jiffies[CPUStates::kIRQ_]) +
                   std::stol(jiffies[CPUStates::kSoftIRQ_]) +
                   std::stol(jiffies[CPUStates::kSteal_]);
    }
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
    if (jiffies.size() >= CPUStates::kIOwait_) {
      idleTime = std::stol(jiffies[CPUStates::kIdle_]) +
                 std::stol(jiffies[CPUStates::kIOwait_]);
    }
  } catch (...) {
    idleTime = 0;
  }

  return idleTime / sysconf(_SC_CLK_TCK);
}

// Read and return CPU utilization
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::string line, cpu, value;
  std::vector<std::string> jiffies;

  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> value) {
      jiffies.emplace_back(value);
    }
  }
  stream.close();

  return jiffies;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int processes;

  processes = parseFile<int>(filterProcesses, LinuxParser::kProcDirectory +
                                                  LinuxParser::kStatFilename);

  return processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int processes;

  processes =
      parseFile<int>(filterRunningProcesses,
                     LinuxParser::kProcDirectory + LinuxParser::kStatFilename);

  return processes;
}

// Read and return the command associated with a process
std::string LinuxParser::Command(int pid) {
  std::string command =
      parseFile<std::string>(LinuxParser::kProcDirectory + std::to_string(pid) +
                             LinuxParser::kCmdlineFilename);

  return command;
}

// Read and return the memory used by a process
std::string LinuxParser::Ram(int pid) {
  // The Keyword is changed from VmSize to VmRSS due to Udacitys review
  // suggestion. The background is the fact, that VmSize gives the whole virtual
  // memory which might exceed the actual available physical memory size. The
  // physical memory is given by VmRSS. Given link by Reviewer:
  // https://man7.org/linux/man-pages/man5/proc.5.html
  std::string memory = parseFile<std::string>(
      filterProcMem, LinuxParser::kProcDirectory + std::to_string(pid) +
                         LinuxParser::kStatusFilename);
  int scaledMem = 0;
  try {
    scaledMem = stoi(memory);
    scaledMem /= 1024;
  } catch (...) {
    // scaledMem is initialized to zero, do nothing.
  }
  return std::to_string(scaledMem);
}

// Read and return the user ID associated with a process
std::string LinuxParser::Uid(int pid) {
  std::string uid = parseFile<std::string>(
      filterUID, LinuxParser::kProcDirectory + std::to_string(pid) +
                     LinuxParser::kStatusFilename);

  return uid;
}

// Read and return the user associated with a process
std::string LinuxParser::User(int pid) {
  std::string uid = Uid(pid);
  std::string id, not_used, foundName, line;
  std::string name;

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
  std::string line, value;
  std::vector<std::string> values;
  long starttime;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.emplace_back(value);
    }
  }
  stream.close();

  try {
    if (values.size() >= 21) {
      starttime = std::stol(values[21]) / sysconf(_SC_CLK_TCK);
    }
  } catch (...) {
    starttime = 0;
  }
  return starttime;
}
