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
            Connect(std::forward<Lambda>(callback));
        }

        bool operator==(const Func other) const
        {
            return m_Invokation.Storage == other.m_Invokation.Storage;
        }

        bool operator!=(const Func other) const
        {
            return !((*this) == other);
        }

        operator bool() const
        {
            return m_Invokation.Stub != nullptr;
        }

        Result operator()(Args&&... args) const
        {
            return Invoke(std::forward<Args>(args)...);
        }

        template<typename Lambda>
        void Connect(Lambda&& callback)
        {
            DODO_ASSERT(sizeof(Lambda) <= AlignedStorage::MaxStackSize, "Callback too complex!");
            new (&m_Invokation.Storage) Lambda(std::forward<Lambda>(callback));
            m_Invokation.Stub = [](const AlignedStorage& storage, Args&&... args) -> Result {
                auto callback = reinterpret_cast<const Lambda*>(&storage);
                return (*callback)(std::forward<Args>(args)...);
            };
        }

        Result Invoke(Args&&... args) const
        {
            return m_Invokation.Stub(m_Invokation.Storage, std::forward<Args>(args)...);
        }

        void Disconnect()
        {
            m_Invokation = {};
        }

    private:
        struct AlignedStorage
        {
            static constexpr size_t MaxStackSize = sizeof(void*);
            alignas(alignof(void*)) uint8_t Stack[MaxStackSize]{};
        };

        struct Invokation
        {
            AlignedStorage Storage{};
            Result(*Stub)(const AlignedStorage&, Args&&...) = nullptr;
        };

        Invokation m_Invokation{};
    };

}