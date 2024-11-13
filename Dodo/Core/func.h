#pragma once

#include <utility>

#include "core/core.h"

namespace Dodo {

    template<typename>
    class Func {};

    template<typename Result, typename... Args>
    class Func<Result(Args...)> {
    public:
        Func() = default;

        template<typename Lambda>
        Func(Lambda&& callback) {
            connect(std::forward<Lambda>(callback));
        }

        bool operator==(const Func other) const {
            return (invokation.storage == other.invokation.storage) &&
                (invokation.stub == other.invokation.stub);
        }

        bool operator!=(const Func other) const {
            return !((*this) == other);
        }

        operator bool() const {
            return invokation.stub != nullptr;
        }

        Result operator()(Args&&... args) const {
            return invoke(std::forward<Args>(args)...);
        }

        template<typename Lambda>
        void connect(Lambda&& callback) {
            DODO_VERIFY(sizeof(Lambda) <= AlignedStorage::max_stack_size);
            new (&invokation.storage) Lambda(std::forward<Lambda>(callback));
            invokation.stub = [](const AlignedStorage& storage, Args&&... args) -> Result {
                auto callback = reinterpret_cast<const Lambda*>(&storage);
                return (*callback)(std::forward<Args>(args)...);
            };
        }

        Result invoke(Args&&... args) const {
            return invokation.stub(invokation.storage, std::forward<Args>(args)...);
        }

        void disconnect()
        {
            invokation = {};
        }

    private:
        struct AlignedStorage
        {
            static constexpr size_t max_stack_size = sizeof(void*);
            alignas(alignof(void*)) uint8_t data[max_stack_size]{};
        };

        struct Invokation
        {
            AlignedStorage storage{};
            Result(*stub)(const AlignedStorage&, Args&&...) = nullptr;
        };

        Invokation invokation{};
    };

}