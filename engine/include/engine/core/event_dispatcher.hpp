#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <vector>

// ── EventDispatcher<EventT> ───────────────────────────────────────────────────
// Synchronous, type-safe event dispatcher for intra-thread event delivery.
//
// Threading model (see docs/03-initial-architecture.md — Threading model):
//   - publish() is intended to be called from the ENGINE thread.
//   - subscribe() and unsubscribe() may be called from any thread; the
//     shared_mutex allows concurrent reads during publish() while serialising
//     structural changes.
//
// publish() semantics:
//   - Copies the subscriber list under a shared lock before invoking handlers.
//     The lock is released before any handler is called, so:
//       * Handlers that call subscribe() or unsubscribe() will not deadlock.
//       * Handlers added or removed during a publish() call do not participate
//         in that dispatch round.
//
// One EventDispatcher instance per event type. Instances are owned and managed
// by an EventBusRegistry (to be implemented as part of the engine context).
//
// Usage:
//   EventDispatcher<SignalAspectChanged> dispatcher;
//   auto token = dispatcher.subscribe([](const SignalAspectChanged& e) { ... });
//   dispatcher.publish(SignalAspectChanged{...});
//   dispatcher.unsubscribe(token);

namespace engine::core
{

template<typename EventT>
class EventDispatcher
{
public:
    using Handler = std::function<void(const EventT&)>;
    using SubscriptionToken = std::uint64_t;

    EventDispatcher() = default;

    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    EventDispatcher(EventDispatcher&&) = delete;
    EventDispatcher& operator=(EventDispatcher&&) = delete;

    // Register a handler. Returns an opaque token used to unsubscribe.
    // Safe to call from any thread.
    SubscriptionToken subscribe(Handler handler)
    {
        std::unique_lock lock{mutex_};
        const SubscriptionToken token = next_token_++;
        subscriptions_.push_back({token, std::move(handler)});
        return token;
    }

    // Unregister the handler identified by token. No-op if the token is unknown.
    // Safe to call from any thread, including from within a handler.
    void unsubscribe(SubscriptionToken token)
    {
        std::unique_lock lock{mutex_};
        subscriptions_.erase(
            std::remove_if(subscriptions_.begin(), subscriptions_.end(),
                           [token](const Subscription& s) { return s.token == token; }),
            subscriptions_.end());
    }

    // Deliver the event to all currently registered handlers synchronously.
    // The subscriber list is copied before dispatch; changes made by handlers
    // take effect from the next publish() call.
    void publish(const EventT& event)
    {
        // Snapshot under shared lock — allows concurrent reads, blocks only
        // subscribe/unsubscribe writers.
        std::vector<Subscription> snapshot;
        {
            std::shared_lock lock{mutex_};
            snapshot = subscriptions_;
        }
        for (const auto& s : snapshot)
        {
            s.handler(event);
        }
    }

    // Number of currently registered handlers.
    std::size_t subscriber_count() const
    {
        std::shared_lock lock{mutex_};
        return subscriptions_.size();
    }

private:
    struct Subscription
    {
        SubscriptionToken token;
        Handler handler;
    };

    mutable std::shared_mutex mutex_;
    std::vector<Subscription> subscriptions_;
    SubscriptionToken next_token_{0};
};

}  // namespace engine::core
