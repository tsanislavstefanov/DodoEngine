#pragma once

#include <utility>

#include "Core/Core.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FUNC ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

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
            Connect(std::forward<Lambda>(callback));
        }

        operator bool() const
        {
            return m_Invocation.Invoker != nullptr;
        }

        Result operator()(Args... args) const
        {
            return Invoke(std::forward<Args>(args)...);
        }

        template<typename Lambda>
        void Connect(Lambda&& callback)
        {
            DODO_ASSERT(sizeof(Lambda) <= DefaultStackSize, "Callback too complex!");
            new (&m_Invocation.Data) Lambda(std::forward<Lambda>(callback));
            m_Invocation.Invoker = [](const Storage& data, Args&&... args) -> Result {
                auto callback = reinterpret_cast<const Lambda*>(&data);
                return (*callback)(std::forward<Args>(args)...);
            };
        }

        Result Invoke(Args... args) const
        {
            return m_Invocation.Invoker(m_Invocation.Data, std::forward<Args>(args)...);
        }

        void Disconnect()
        {
            m_Invocation = {};
        }

    private:
        static constexpr size_t DefaultStackAlignment = alignof(void*);
        static constexpr size_t DefaultStackSize = sizeof(void*);

        ////////////////////////////////////////////////////////////
        // STORAGE /////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct Storage
        {
            alignas(DefaultStackAlignment) uint8_t Stack[DefaultStackSize]{};
        };

        ////////////////////////////////////////////////////////////
        // INVOCATION //////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct Invocation
        {
            Storage Data{};

            ////////////////////////////////////////////////////////
            // STUB ////////////////////////////////////////////////
            ////////////////////////////////////////////////////////

            using Stub = Result(*)(const Storage&, Args&&...);
            Stub Invoker = nullptr;
        };

        Invocation m_Invocation{};
    };

}