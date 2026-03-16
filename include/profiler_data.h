#ifndef PROFILER_DATA
#define PROFILER_DATA

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>

/*
==========================================
FrameRecord - one entry per encoded frame
            Written by pad probe on streaming thread
==========================================
*/

struct FrameRecord
{
    uint64_t frame_number;      // frame counter
    uint64_t enter_ns;          // timestamp when raw frame entred encoder
    uint64_t exit_ns;          // timestamp when encoded frame exit encoder
    double latency_ms;          // exit - enter / 1e6
    size_t encoded_bytes;       // buffer size on output buffer
    bool is_keyframe;           // true if NOT GST_BUFFER_FLAG_DELTA_UNIT

    FrameRecord()
        : frame_number(0),
          enter_ns(0),
          exit_ns(0),
          latency_ms(0.0),
          encoded_bytes(0),
          is_keyframe(false)
    {}
};

/*
==========================================
ResourceSnapshot - one entry per 100ms poll
==========================================
*/

struct ResourceSnapshot
{
    uint64_t timestamp_ns;      // when snapshot was taken
    double cpu_percent;         // CPU% over last 100ms interval
    long rss_kb;                // VmRSS from proc

    ResourceSnapshot()
        : timestamp_ns(0),
          cpu_percent(0.0),
          rss_kb(0)
        {}
};

#endif
