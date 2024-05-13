#pragma once

#include <utility>
#include <vector>

#include "Func.h"

////////////////////////////////////////////////////////////////
// SIGNAL //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

template<typename... Args>
class Signal
{
public:
    Signal() = default;

    explicit operator bool() const
    {
        return IsEmpty();
    }

    void operator()(Args... args)
    {
        Emit(std::forward<Args>(args)...);
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return !m_Callbacks.empty();
    }

    template<typename Lambda>
    void Connect(Lambda&& lambda)
    {
        Callback callback{};
        callback.Connect(std::forward<Lambda>(lambda));
        m_Callbacks.push_back(std::move(callback));
    }

    void Emit(Args... args)
    {
        for (auto& callback : m_Callbacks)
        {
            callback.Invoke(std::forward<Args>(args)...);
        }
    }

    template<typename Lambda>
    void Disconnect(Lambda&& lambda)
    {
        Callback callback{};
        callback.Connect(std::forward<Lambda>(lambda));
        DISCARD_MAYBE_UNUSED(std::erase_if(m_Callbacks, [&](auto& other) {
            return callback == other;
        }));;
    }

private:
    ////////////////////////////////////////////////////////////
    // CALLBACK ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////

    using Callback = Func<void(Args...)>;
    std::vector<Callback> m_Callbacks{};
};