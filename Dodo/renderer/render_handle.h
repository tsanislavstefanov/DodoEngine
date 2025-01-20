#pragma once

#include <cstdint>
#include <vector>

namespace Dodo {

    template<typename Handle, typename T>
    class RenderHandleOwner {
    public:
        Handle create(T&& p_data) {
            uint32_t index = 0;
            if (!_free_list.empty()) {
                index = _free_list.back();
                _free_list.pop_back();
            }
            else {
                index = _size++;
                if (_size > _generations.size()) {
                    _resize(static_cast<size_t>(_size * 1.5f));
                }
            }

            _data.at(index) = std::move(p_data);
            const uint32_t generation = _generations.at(index);
            return Handle((static_cast<uint64_t>(generation) << 32) | static_cast<uint64_t>(index + 1));
        }

        T* get_or_null(Handle p_handle) const {
            auto [index, generation] = _unpack(p_handle);
            if (!_is_valid(index, generation)) {
                return nullptr;
            }

            return &_data.at(index);
        }

        void destroy(Handle p_handle) {
            auto [index, generation] = _unpack(p_handle);
            if (!_is_valid(index, generation))  {
                return;
            }

            _generations.at(index)++;
            _data.at(index).~T();
            _free_list.push_back(index);
        }

        void clear() {
            _generations.clear();
            _data.clear();
            _size = 0;
            _free_list.clear();
        }

    private:
        void _resize(size_t p_new_size) {
            _generations.resize(p_new_size);
            _data.resize(p_new_size);
        }

        std::pair<uint32_t, uint32_t> _unpack(Handle p_handle) const {
            const auto index = static_cast<uint32_t>(p_handle.get_id() & UINT32_MAX);
            const auto generation = static_cast<uint32_t>(p_handle.get_id() >> 32);
            return std::pair<uint32_t, uint32_t>(index - 1, generation);
        }

        bool _is_valid(uint32_t p_index, uint32_t p_generation) const {
            return (_size > p_index) && (_generations.at(p_index) == p_generation);
        }

        std::vector<uint32_t> _generations = {};
        std::vector<T> _data = {};
        size_t _size = 0;
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