#ifndef HUES_COMMON_H_
#define HUES_COMMON_H_

#include <time.h>

#include <iostream>

#include <config.h>

// Shorthand to disable a class's copy constructor and assignment operator.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(TypeName const&) = delete;    \
    TypeName& operator=(TypeName const&) = delete;

// Ugly logging to stdout.
#define _DO_LOG(type, message) { \
  std::string fn = __transform_pretty_function(__PRETTY_FUNCTION__); \
  std::string time = __format_log_time(); \
  std::cout << "[" type " " << time << " " << fn << " " \
      << __SRCFILE__ << ":" << __LINE__ << "] " << (message) << std::endl; \
}
#define ERR(message) _DO_LOG("E", message)
#define LOG(message) _DO_LOG("I", message)
#ifdef _DEBUG
#define DEBUG(message) _DO_LOG("D", message)
#else // DEBUG
#define DEBUG(message) {}
#endif // DEBUG

inline std::string __transform_pretty_function(const std::string& fn) {
  size_t colons = fn.find("::");
  size_t begin = fn.substr(0, colons).rfind(" ") + 1;
  size_t end = fn.rfind("(") - begin;
  return fn.substr(begin, end) + "()";
}
inline std::string __format_log_time() {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[16];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime (buffer, 16, "%y%m%d %H:%M:%S" , timeinfo);
  return std::string(buffer);
}

#endif // HUES_COMMON_H_
