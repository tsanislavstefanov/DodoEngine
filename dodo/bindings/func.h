#pragma once

#include <utility>

#include "core/core.h"

namespace Dodo {

    template<typename>
    class Func {};

    template<typename Result, typename... Args>
    class Func<Result(Args...)>
    {
    public:
        Func() = default;

        template<typename Lambda>
        Func(Lambda&& callback)
        {
            connect(std::forward<Lambda>(callback));
        }

        bool operator==(const Func other) const
        {
            return invokation_.data == other.invokation_.data;
        }

        bool operator!=(const Func other) const
        {
            return !((*this) == other);
        }

        operator bool() const
        {
            return invokation_.stub_callback != nullptr;
        }

        Result operator()(Args&&... args) const
        {
            return invoke(std::forward<Args>(args)...);
        }

        template<typename Lambda>
        void connect(Lambda&& callback)
        {
            DODO_ASSERT(sizeof(Lambda) <= Storage::max_stack_size, "Callback too complex!");
            new (&invokation_.data) Lambda(std::forward<Lambda>(callback));
            invokation_.stub_callback = [](const Storage& data, Args&&... args) -> Result {
                auto callback = reinterpret_cast<const Lambda*>(&data);
                return (*callback)(std::forward<Args>(args)...);
            };
        }

        Result invoke(Args&&... args) const
        {
            return invokation_.stub_callback(invokation_.data, std::forward<Args>(args)...);
        }

        void disconnect()
        {
            invokation_ = {};
        }

    private:
        struct Storage
        {
            static constexpr size_t max_stack_size = sizeof(void*);
            alignas(alignof(void*)) uint8_t stack[max_stack_size]{};
        };

        struct Invocation
        {
            Storage data{};
            Result(*stub_callback)(const Storage&, Args&&...) = nullptr;
        };

        Invocation invokation_{};
    };

}