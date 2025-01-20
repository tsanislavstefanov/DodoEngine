#pragma once

#include <cstdint>
#include <vector>

namespace Dodo {

    template<typename Handle, typename Resource>
    class RenderHandlePool {
    public:
        Handle create(Resource&& resource);
        Resource* get_or_null(Handle handle) const;
        void destroy(Handle handle);

    private:
        void _expand(size_t new_size);
        std::pair<uint32_t, uint32_t> _unpack(Handle handle) const;
        bool _is_valid(uint32_t index, uint32_t version) const;

        std::vector<uint32_t> _versions = {};
        std::vector<Resource> _data = {};
        size_t _size = 0;
        std::vector<uint32_t> _free_list = {};
    };

    template<typename Handle, typename Resource>
    inline Handle RenderHandlePool<Handle, Resource>::create(Resource&& resource) {
        uint32_t index = 0;
        if (!_free_list.empty()) {
            index = _free_list.back();
            _free_list.pop_back();
        }
        else {
            index = _size++;
            if (_versions.size() < _size) {
                _expand(static_cast<size_t>(_size * 1.5f));
            }
        }

        _data.at(index) = std::move(resource);
        const uint32_t version = _versions.at(index);
        return Handle((static_cast<uint64_t>(version) << 32) | (static_cast<uint64_t>(index) + 1));
    }

    template<typename Handle, typename Resource>
    inline Resource* RenderHandlePool<Handle, Resource>::get_or_null(Handle handle) const {
        auto [index, version] = _unpack(handle);
        if (!_is_valid(index, version)) {
            return nullptr;
        }

        return &_data.at(index);
    }

    template<typename Handle, typename Resource>
    inline void RenderHandlePool<Handle, Resource>::destroy(Handle handle) {
        auto [index, version] = _unpack(handle);
        if (!_is_valid(index, version)) {
            return;
        }

        _versions.at(index)++;
        _data.at(index).~Resource();
        _free_list.push_back(index);
    }

    template<typename Handle, typename Resource>
    inline void RenderHandlePool<Handle, Resource>::_expand(size_t new_size) {
        _versions.resize(new_size);
        _data.resize(new_size);
    }

    template<typename Handle, typename Resource>
    inline std::pair<uint32_t, uint32_t> RenderHandlePool<Handle, Resource>::_unpack(Handle handle) const {
        const auto index = static_cast<uint32_t>(handle.get_id() & UINT32_MAX);
        const auto version = static_cast<uint32_t>(handle.get_id() >> 32);
        return std::pair<uint32_t, uint32_t>(index - 1, version);
    }

    template<typename Handle, typename Resource>
    inline bool RenderHandlePool<Handle, Resource>::_is_valid(uint32_t index, uint32_t version) const {
        return (_size > index) && (_versions.at(index) == version);
    }

    class RenderHandle {
    public:
        RenderHandle() = default;
        RenderHandle(uint64_t id)
            : _id(id) {}

        inline operator bool() const {
            return !is_null();
        }

        inline operator uint64_t() const {
            return get_id();
        }

        inline bool is_null() const {
            return _id == 0;
        }

        inline uint64_t get_id() const {
            return _id;
        }

    protected:
        uint64_t _id{0};
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