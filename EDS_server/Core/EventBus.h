#pragma once
#include <boost/signals2.hpp>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <utility>

class EventBus {
public:
    template<typename Event>
    boost::signals2::connection subscribe(
        typename boost::signals2::signal<void(const Event&)>::slot_type slot)
    {
        auto sig = fnGetSignal<Event>();
        return sig->connect(std::move(slot));
    }

    template<typename Event>
    void publish(const Event& event)
    {
        auto sig = fnTryGetSignal<Event>();
        if (sig) {
            (*sig)(event);
        }
    }

private:
    struct SignalHolderBase {
        virtual ~SignalHolderBase() = default;
    };

    template<typename Event>
    struct SignalHolder final : SignalHolderBase {
        using signal_t = boost::signals2::signal<void(const Event&)>;
        std::shared_ptr<signal_t> signal{ std::make_shared<signal_t>() };
    };

private:
    template<typename Event>
    std::shared_ptr<boost::signals2::signal<void(const Event&)>> fnGetSignal()
    {
        const std::type_index key(typeid(Event));

        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_signals.find(key);
        if (it != m_signals.end()) {
            auto holder = std::static_pointer_cast<SignalHolder<Event>>(it->second);
            return holder->signal;
        }

        auto holder = std::make_shared<SignalHolder<Event>>();
        auto sig = holder->signal;
        m_signals.emplace(key, std::move(holder));
        return sig;
    }

    template<typename Event>
    std::shared_ptr<boost::signals2::signal<void(const Event&)>> fnTryGetSignal()
    {
        const std::type_index key(typeid(Event));

        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_signals.find(key);
        if (it == m_signals.end()) {
            return {};
        }

        auto holder = std::static_pointer_cast<SignalHolder<Event>>(it->second);
        return holder->signal;
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<SignalHolderBase>> m_signals;
    std::mutex m_mutex;
};
