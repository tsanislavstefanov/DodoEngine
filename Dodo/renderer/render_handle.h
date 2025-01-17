#pragma once

#include <cstdint>
#include <limits>
#include <vector>

namespace Dodo {

    template<typename Handle, typename T>
    class RenderHandleOwner {
    public:
        Handle create(const T& value) {
            uint32_t index = _size;
            if (!_free_list.empty()) {
                index = _free_list.back();
                _free_list.pop_back();
            }

            if (_size <= index) {
                _size++;
                _generations.resize(_size + 1);
                _data.resize(_size + 1);
            }

            _data.at(index) = std::move(value);
            const uint32_t generation = _generations.at(index);
            return (static_cast<uint64_t>(generation) << 32) | index;
        }

        T* get_or_null(Handle handle) const {
            const auto generation = static_cast<uint32_t>(handle.get_id() >> 32);
            const auto index = static_cast<uint32_t>(handle.get_id() & std::numeric_limits<uint32_t>::max());
            if ((_size <= index) || (generation != _generations.at(index)))  {
                return nullptr;
            }

            return &_data.at(index);
        }

        void destroy(Handle handle) {
            const auto generation = static_cast<uint32_t>(handle.get_id() >> 32);
            const auto index = static_cast<uint32_t>(handle.get_id() & std::numeric_limits<uint32_t>::max());
            if ((_size <= index) || (generation != _generations.at(index)))  {
                return;
            }

            _generations.at(index)++;
            _data.at(index).~T();
            _free_list.push_back(index);
        }

    private:
        std::vector<uint32_t> _generations = {};
        std::vector<T> _data = {};
        uint32_t _size = 0;
        std::vector<uint32_t> _free_list = {};
    };

    class RenderHandle {
    public:
        RenderHandle() = default;
        RenderHandle(uint64_t id) : _id(id) {}

        inline bool is_null() const {
            return _id == 0;
        }

        inline uint64_t get_id() const {
            return _id;
        }

    protected:
        uint64_t _id = 0;
    };

#define DODO_DEFINE_RENDER_HANDLE(NAME) \
    class NAME##Handle : public RenderHandle { \
    public: \
        NAME##Handle() = default; \
        NAME##Handle(uint64_t id) : RenderHandle(id) {} \
        NAME##Handle(const NAME##Handle& other) : RenderHandle(other.get_id()) {} \
        inline NAME##Handle& operator=(const NAME##Handle& other) { \
            _id = other.get_id(); \
            return *this; \
        } \
        inline bool operator==(const NAME##Handle& other) const { \
            return _id == other.get_id(); \
        } \
        inline bool operator!=(const NAME##Handle& other) const { \
            return !((*this) == other); \
        } \
    }

}
