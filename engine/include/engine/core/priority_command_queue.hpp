#pragma once

#include "engine/core/types.hpp"

#include <array>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>

// ── PriorityCommandQueue<T> ───────────────────────────────────────────────────
// Thread-safe multi-producer, single-consumer command queue with four FIFO
// priority buckets ordered by CommandPriority.
//
// Threading model (see docs/ARCHITECTURE.md — Threading model):
//   - WORK_POOL threads push commands after ownership validation.
//   - ENGINE thread pops commands once per tick up to MAX_CMDS_PER_TICK.
//
// Priority order (highest first):
//   EMERGENCY(0) > SAFETY(1) > NORMAL(2) > BACKGROUND(3)
//
// Guarantees:
//   - try_pop / wait_and_pop always return from the highest-priority non-empty
//     bucket, preserving FIFO order within each priority level.
//   - A command pushed at EMERGENCY will be dequeued before any SAFETY command
//     already in the queue, regardless of push order.

namespace engine::core
{

template<typename T>
class PriorityCommandQueue
{
public:
    static constexpr std::size_t kBucketCount =
        static_cast<std::size_t>(CommandPriority::BACKGROUND) + 1;  // 4

    PriorityCommandQueue() = default;

    PriorityCommandQueue(const PriorityCommandQueue&) = delete;
    PriorityCommandQueue& operator=(const PriorityCommandQueue&) = delete;
    PriorityCommandQueue(PriorityCommandQueue&&) = delete;
    PriorityCommandQueue& operator=(PriorityCommandQueue&&) = delete;

    // Push a command into the bucket for the given priority. Never blocks.
    // Thread-safe for concurrent producers.
    // Throws std::runtime_error if the queue has been closed.
    void push(T item, CommandPriority priority)
    {
        const auto idx = static_cast<std::size_t>(priority);
        {
            std::scoped_lock lock{mutex_};
            if (closed_)
            {
                throw std::runtime_error("PriorityCommandQueue::push: queue is closed");
            }
            buckets_[idx].push(std::move(item));
        }
        cv_.notify_one();
    }

    // Non-blocking pop from the highest-priority non-empty bucket.
    // Returns std::nullopt if all buckets are empty.
    std::optional<T> try_pop()
    {
        std::scoped_lock lock{mutex_};
        return pop_highest_locked();
    }

    // Blocking pop. Waits until any bucket is non-empty or the queue is closed.
    // Returns std::nullopt when closed and all buckets are drained.
    std::optional<T> wait_and_pop()
    {
        std::unique_lock lock{mutex_};
        cv_.wait(lock, [this] { return has_items_locked() || closed_; });
        return pop_highest_locked();
    }

    // Signal shutdown. Unblocks threads waiting in wait_and_pop().
    // Remaining items in buckets are still poppable after close() is called;
    // std::nullopt is returned only after all buckets are empty.
    void close()
    {
        {
            std::scoped_lock lock{mutex_};
            closed_ = true;
        }
        cv_.notify_all();
    }

    bool is_closed() const
    {
        std::scoped_lock lock{mutex_};
        return closed_;
    }

    // Total number of items across all priority buckets.
    std::size_t size() const
    {
        std::scoped_lock lock{mutex_};
        std::size_t total = 0;
        for (const auto& bucket : buckets_)
        {
            total += bucket.size();
        }
        return total;
    }

    bool empty() const
    {
        std::scoped_lock lock{mutex_};
        return !has_items_locked();
    }

private:
    bool has_items_locked() const
    {
        for (const auto& bucket : buckets_)
        {
            if (!bucket.empty())
            {
                return true;
            }
        }
        return false;
    }

    // Caller must hold mutex_. Returns std::nullopt if all buckets are empty.
    std::optional<T> pop_highest_locked()
    {
        for (auto& bucket : buckets_)
        {
            if (!bucket.empty())
            {
                T item = std::move(bucket.front());
                bucket.pop();
                return item;
            }
        }
        return std::nullopt;
    }

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::array<std::queue<T>, kBucketCount> buckets_;
    bool closed_{false};
};

}  // namespace engine::core
