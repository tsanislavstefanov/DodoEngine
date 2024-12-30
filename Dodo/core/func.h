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
        Func(Lambda&& lambda) {
            connect(std::forward<Lambda>(lambda));
        }

        bool operator==(const Func other) const {
            return _storage == other._storage;
        }

        bool operator!=(const Func other) const {
            return !((*this) == other);
        }

        operator bool() const {
            return _fn != nullptr;
        }

        Result operator()(Args... args) const {
            return invoke(args...);
        }

        template<typename Lambda>
        void connect(Lambda&& lambda) {
            new (&_storage.data) Lambda(std::forward<Lambda>(lambda));
            _fn = [](const AlignedStorage& storage, Args&&... args) -> Result {
                auto callback = reinterpret_cast<const Lambda*>(&storage.data);
                return (*callback)(std::forward<Args>(args)...);
            };
        }

        Result invoke(Args... args) const {
            return _fn(_storage, args...);
        }

        void disconnect() {
            _storage = {};
            _fn = nullptr;
        }

    private:
        struct AlignedStorage {
            static constexpr size_t max_stack_size = sizeof(void*);
            alignas(alignof(void*)) uint8_t data[max_stack_size]{};
        };

        AlignedStorage _storage{};
        using Stub = Result(*)(const AlignedStorage&, Args&&...);
        Stub _fn = nullptr;
    };

}