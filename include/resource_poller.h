#ifndef RESOURCE_POLLER_H
#define RESOURCE_POLLER_H

#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <cstdint>

#include "timer.h"

/* ======================================
    Snapshot stored every 100ms
    =====================================
*/
struct ResourseSnapshot
{
    uint64_t timestamp_ns;  // when this was taken
    double cpu_precent;     // CPU% over last 100ms interval
    long rss_kb;            // physical RAM in kilobytes
};

/* ========================================
    Internal CPU stat from /proc/self/stat
    =======================================
*/
struct CpuStat
{
    unsigned long utime;    // user ticks (field 14)
    unsigned long stime;    // kernel ticks (feild 15)

    CpuStat(): utime(0), stime(0) {}
};

/* ========================================
    ResourcePoller class
    =======================================
*/
class ResourcePoller
{
    public:
        ResourcePoller();
        ~ResourcePoller();

        void start();           // launch background thread
        void stop();            // signal + join

        // call after stop() - copies shnapshot out safely
        std::vector<ResourseSnapshot> get_snapshots() const;

    private:
        // function the background thread runs
        void poll_loop();

        // /proc readers
        static CpuStat read_cpu_stat();
        static long read_rss_kb();

        // thread + stop signal
        std::thread thread_;
        std::atomic<bool> running_;

        // snapshot storage - protected by mutex_
        std::vector<ResourseSnapshot> snapshots_;
        mutable std::mutex mutex_;

        // disable copy
        ResourcePoller(const ResourcePoller&) = delete;
        ResourcePoller& operator=(const ResourcePoller&) = delete;

};


#endif
