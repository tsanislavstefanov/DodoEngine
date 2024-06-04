#pragma once

#include <utility>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FUNC ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    template<typename, size_t>
    class Func {};

    template<size_t StackSize, typename Result, typename... Args>
    class Func<Result(Args...), StackSize>
    {
    public:
        Func() = default;

        template<typename Lambda>
        Func(Lambda&& lambda)
        {
            Connect(std::forward<Lambda>(lambda));
        }

        operator bool() const
        {
            return m_Invocation.Callback != nullptr;
        }

        Result operator()(Args... args) const
        {
            return Invoke(std::forward<Args>(args)...);
        }

        template<typename Lambda>
        void Connect(Lambda&& lambda)
        {
            DODO_ASSERT(sizeof(Lambda) <= StackSize, "Function too complex!");
            new (&m_Invocation.Data) Lambda(std::forward<Lambda>(lambda));
            m_Invocation.Callback = [](const Storage& data, Args&&... args) -> Result
            {
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
        // ALIGNED STORAGE /////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        template<size_t Align = alignof(void*)>
        struct AlignedStorage
        {
            alignas(Align) uint8_t Stack[StackSize]{};
        };

        ////////////////////////////////////////////////////////////
        // INVOCATION //////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct Invocation
        {
            AlignedStorage Data{};

            ////////////////////////////////////////////////////////
            // STUB ////////////////////////////////////////////////
            ////////////////////////////////////////////////////////

            using Stub = Result(*)(const Storage&, Args&&...);
            Stub Callback = nullptr;
        };

        Invocation m_Invocation{};
    };

    ////////////////////////////////////////////////////////////////
    // SMALL FUNC //////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    template<typename Result, typename... Args>
    using SmallFunc = Func<Result(Args...), sizeof(void*)>;

}