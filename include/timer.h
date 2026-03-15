#ifndef TIMER_H
#define TIMER_H

#include <cstdint>
#include <time.h>

// Returns current monotonic time in nanoseconds
inline uint64_t get_time_ns()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL
            + static_cast<uint64_t>(ts.tv_nsec);
}

inline double ns_to_ms(uint64_t ns)
{
    return static_cast<double>(ns) / 1.0e6;
}
#endif