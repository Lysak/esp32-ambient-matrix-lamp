#pragma once

#include <sys/time.h>
#include <time.h>

#include <cstdio>

inline void format_log_wall_time(char *buffer, size_t size) {
  struct timeval tv;
  gettimeofday(&tv, nullptr);

  struct tm timeinfo;
  localtime_r(&tv.tv_sec, &timeinfo);

  snprintf(buffer,
           size,
           "%02d:%02d:%02d.%06ld",
           timeinfo.tm_hour,
           timeinfo.tm_min,
           timeinfo.tm_sec,
           static_cast<long>(tv.tv_usec));
}
