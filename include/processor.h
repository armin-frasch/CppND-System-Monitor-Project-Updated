#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long totalTime_;
  long activeTime_;
  long idleTime_;
};

#endif