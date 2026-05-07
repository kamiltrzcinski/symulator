#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>

// ── EventQueue<T> ─────────────────────────────────────────────────────────────
// Thread-safe FIFO queue designed for crossing a single thread boundary.
//
// Threading model (see docs/03-initial-architecture.md — Threading model):
//   - Any number of producer threads may call push() concurrently.
//   - Exactly one consumer thread calls wait_and_pop() or try_pop().
//   - Multiple concurrent consumers are NOT supported.
//
// Lifecycle:
//   - close() signals shutdown; wait_and_pop() returns std::nullopt when the
//     queue is both closed and drained. Subsequent push() calls throw.
//
// Designed use cases:
//   ENGINE  -> EventQueue<DomainEvent>    -> DISPATCHER
//   DISPATCHER -> EventQueue<EventLogEntry> -> DB_WRITER

namespace engine::core
{

template<typename T>
class EventQueue
{
public:
    EventQueue() = default;

    // Non-copyable, non-movable — owned by exactly one producer/consumer pair.
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;
    EventQueue(EventQueue&&) = delete;
    EventQueue& operator=(EventQueue&&) = delete;

    // Push an item onto the back of the queue. Never blocks.
    // Thread-safe for concurrent producers.
    // Throws std::runtime_error if the queue has been closed.
    void push(T item)
    {
        {
            std::scoped_lock lock{mutex_};
            if (closed_)
            {
                throw std::runtime_error("EventQueue::push: queue is closed");
            }
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    // Non-blocking pop from the front. Returns std::nullopt if the queue is empty.
    std::optional<T> try_pop()
    {
        std::scoped_lock lock{mutex_};
        if (queue_.empty())
        {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    // Blocking pop. Waits until an item is available or the queue is closed.
    // Returns std::nullopt when the queue is both closed and fully drained —
    // the consumer should treat std::nullopt as a shutdown signal.
    std::optional<T> wait_and_pop()
    {
        std::unique_lock lock{mutex_};
        cv_.wait(lock, [this] { return !queue_.empty() || closed_; });
        if (queue_.empty())
        {
            return std::nullopt;  // Closed and drained — signal consumer to exit.
        }
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    // Signal that no more items will be pushed.
    // Unblocks all threads waiting in wait_and_pop(). Remaining items are still
    // consumed normally; wait_and_pop() returns std::nullopt only after the queue
    // is empty.
    void close()
    {
        {
            std::scoped_lock lock{mutex_};
            closed_ = true;
        }
        cv_.notify_all();
    }

    // Returns true if close() has been called.
    bool is_closed() const
    {
        std::scoped_lock lock{mutex_};
        return closed_;
    }

    // Returns the number of items currently in the queue.
    // May be stale by the time the caller uses the value.
    std::size_t size() const
    {
        std::scoped_lock lock{mutex_};
        return queue_.size();
    }

    bool empty() const
    {
        std::scoped_lock lock{mutex_};
        return queue_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
    bool closed_{false};
};

}  // namespace engine::core
