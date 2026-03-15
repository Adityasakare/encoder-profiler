#include "../include/resource_poller.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <string>

/* ======================================
    Constructor and Desctructor
    =====================================
*/

ResourcePoller::ResourcePoller() : running_(false)
{}

ResourcePoller::~ResourcePoller() 
{
    if(running_.load())
        stop();
}

/* ======================================
    Public APIs
    =====================================
*/

void ResourcePoller::start()
{
    running_.store(true);
    // launch poll_loop() on a new thread
    thread_ = std::thread(&ResourcePoller::poll_loop, this);
}

void ResourcePoller::stop()
{
    running_.store(false);
    if(thread_.joinable())
        thread_.join();
}

std::vector<ResourseSnapshot> ResourcePoller::get_snapshots() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return snapshots_;  // return a copy
}

/* ======================================
    Background Thread
    =====================================
*/

void ResourcePoller::poll_loop()
{
    // take first CPU snapshot before the loop
    CpuStat prev_cpu = read_cpu_stat();
    uint64_t prev_t = get_time_ns();

    while(running_.load())
    {
        // sleep 100ms 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // take new CPU snapshot
        CpuStat curr_cpu = read_cpu_stat();
        uint64_t curr_t = get_time_ns();

        // compute CPU%
        unsigned long ticks = (curr_cpu.utime + curr_cpu.stime) - 
                              (prev_cpu.utime + prev_cpu.stime);

        double cpu_time_ns = ticks * 10000000.0; // 1 tick = 10ms
        double wall_ns = static_cast<double>(curr_t - prev_t);
        double cpu_pct = (wall_ns > 0.0) ? (cpu_time_ns / wall_ns) * 100.0 : 0.0;

        // read RAM
        long rss = read_rss_kb();

        // store snapshot
        ResourseSnapshot snap;
        snap.timestamp_ns = curr_t;
        snap.cpu_precent = cpu_pct;
        snap.rss_kb = rss;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshots_.push_back(snap);
        }

        prev_cpu = curr_cpu;
        prev_t = curr_t;
    }
}

/* ======================================
    /proc readers
    =====================================
*/

CpuStat ResourcePoller::read_cpu_stat()
{
    CpuStat s;
    std::ifstream f("/proc/self/stat");
    if(!f.is_open())
    {
        std::cerr << "[poller] could not open /proc/self/stat\n";
        return s;
    }

    std::string token;
    for(int i=1; i <= 15; ++i)
    {
        f >> token;
        if(i == 14)
            s.utime = std::stoul(token);
        if(i == 15)
            s.stime = std::stoul(token);
    }
    return s;
}

long ResourcePoller::read_rss_kb()
{
    std::ifstream f("/proc/self/status");
    if (!f.is_open()) {
        std::cerr << "[poller] could not open /proc/self/status\n";
        return 0;
    }

    std::string line;
    while (std::getline(f, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream ss(line);
            std::string label;
            long kb = 0;
            ss >> label >> kb;   // "VmRSS:" then the number
            return kb;
        }
    }
    return 0;
}
