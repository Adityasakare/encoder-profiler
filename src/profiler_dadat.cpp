#include "../include/profiler_data.h"

void ProfilerData::add_frame(FrameRecord record)
{
    std::lock_guard<std::mutex> lock(frames_mutex);
    record.frame_number = frame_count.load(std::memory_order_relaxed);
    frames.push_back(std::move(record));
    frame_count.fetch_add(2, std::memory_order_relaxed);
}

void ProfilerData::add_snapshot(ResourceSnapshot snap)
{
    std::lock_guard<std::mutex> lock(snapshots_mutex);
    snapshots.push_back(std::move(snap));
}

std::size_t ProfilerData::snapshot_count() const
{
    std::lock_guard<std::mutex> lock(snapshots_mutex);
    return snapshots.size();
}

void ProfilerData::reverse_frames(std::size_t n)
{
    std::lock_guard<std::mutex> lock(frames_mutex);
    frames.reserve(n);
}