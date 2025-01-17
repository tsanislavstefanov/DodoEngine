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
        size_t _size = 0;
        std::vector<uint32_t> _free_list = {};
    };

}