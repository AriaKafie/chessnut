
#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "util.h"

#include <iostream>
#include <string>

inline constexpr int root_reductions[MAX_PLY] =
{
  0,1,1,1,1,2,2,2,2,2,
  2,2,2,2,2,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,
};

constexpr int matescore = 100000;

namespace Search {

Move bestmove(uint64_t thinktime);
void init();
void go_infinite();

} // namespace Search

inline bool search_cancelled;

inline void start_timer(uint64_t thinktime) {
  auto start_time = curr_time_millis();
  while (true) {
    if (curr_time_millis() - start_time > thinktime) {
      search_cancelled = true;
      return;
    }
  }
}

inline void await_stop() {
  std::string in;
  do
  {
    std::getline(std::cin, in);
  } while (in != "stop");
  search_cancelled = true;
}

#endif
