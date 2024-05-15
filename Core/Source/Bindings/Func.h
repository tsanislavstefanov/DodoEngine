#pragma once

#include <type_traits>
#include <utility>

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

        operator bool() const
        {
            return m_Invocation.Callback != nullptr;
        }

        bool operator==(const Func& other) const
        {
            return m_Invocation.Data == other.m_Invocation.Data;
        }

        bool operator!=(const Func& other) const
        {
            return !((*this) == other);
        }

        Result operator()(Args... args) const
        {
            return Invoke(std::forward<Args>(args)...);
        }

        template<typename Lambda>
        void Connect(Lambda&& lambda)
        {
            new (&m_Invocation.Data) Lambda(std::forward<Lambda>(lambda));
            m_Invocation.Callback = [](const Storage& data, Args&&... args) -> Result {
                auto lambda = reinterpret_cast<const Lambda*>(&data);
                return (*lambda)(std::forward<Args>(args)...);
            };
        }

        Result Invoke(Args... args) const
        {
            return m_Invocation.Callback(m_Invocation.Data, std::forward<Args>(args)...);
        }

        void Disconnect()
        {
            m_Invocation = {};
        }

    private:
        ////////////////////////////////////////////////////////////
        // STORAGE /////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        using Storage = std::aligned_storage<sizeof(void*), alignof(void*)>;

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
            Stub Callback = nullptr;
        };

        Invocation m_Invocation{};
    };

}