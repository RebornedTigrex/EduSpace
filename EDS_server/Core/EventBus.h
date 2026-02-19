#pragma once

#include <boost/signals2.hpp>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <mutex>

class EventBus {
public:
    template<typename Event>
    boost::signals2::connection subscribe(
        typename boost::signals2::signal<void(const Event&)>::slot_type slot)
    {
        auto sig = fnGetSignal<Event>();     //Не реф
        return sig->connect(std::move(slot));
    }

    template<typename Event>
    void publish(const Event& event)
    {
        std::shared_ptr<boost::signals2::signal<void(const Event&)>> sig;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_signals.find(std::type_index(typeid(Event)));
            if (it != m_signals.end()) {
                auto holder = std::static_pointer_cast<SignalHolder<Event>>(it->second);
                sig = holder->signal;
            }
        }
        if (sig) {
            (*sig)(event);
        }
    }

private:
    struct SignalHolderBase { virtual ~SignalHolderBase() = default; };

    template<typename Event>
    struct SignalHolder : SignalHolderBase {
        std::shared_ptr<boost::signals2::signal<void(const Event&)>> signal =
            std::shared_ptr<boost::signals2::signal<void(const Event&)>>(
                new boost::signals2::signal<void(const Event&)>());
    };

    template<typename Event>
    std::shared_ptr<boost::signals2::signal<void(const Event&)>> fnGetSignal()
    {
        const std::type_index key(typeid(Event));

        auto it = m_signals.find(key);
        if (it != m_signals.end()) {
            return std::static_pointer_cast<SignalHolder<Event>>(it->second)->signal;
        }

        auto holder = std::shared_ptr<SignalHolder<Event>>(new SignalHolder<Event>());
        m_signals[key] = holder;
        return holder->signal;
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<SignalHolderBase>> m_signals;
    std::mutex m_mutex;
};
