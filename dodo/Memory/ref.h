#pragma once

#include <atomic>
#include <cstdint>
#include <utility>

namespace Dodo {

    class RefCounted
    {
    public:
        RefCounted() = default;
        virtual ~RefCounted() = default;

        void reference() const
        {
            ++ref_count_;
        }

        void unreference() const
        {
            --ref_count_;
        }

        uint64_t ref_count() const
        {
            return ref_count_.load();
        }

    private:
        mutable std::atomic<uint64_t> ref_count_ = 0;
    };

    template<typename Type>
    class Ref
    {
    public:
        template<typename... Args>
        static Ref<Type> create(Args&&... args)
        {
            return Ref<Type>(new Type(std::forward<Args>(args)...));
        }

        Ref()
            : instance_(nullptr)
        {}

        Ref(std::nullptr_t)
            : instance_(nullptr)
        {}

        Ref(Type* instance)
            : instance_(instance)
        {
            reference();
        }

        Ref(const Ref& other)
            : instance_(other.instance_)
        {
            reference();
        }

        template<typename Other>
        Ref(const Ref<Other>& other)
            : instance_(static_cast<Type*>(other.instance_))
        {
            reference();
        }

        template<typename Other>
        Ref(Ref<Other>&& other)
            : instance_(static_cast<Type*>(other.instance_))
        {
            other.instance_ = nullptr;
        }

        ~Ref()
        {
            unreference();
        }

        Ref& operator=(std::nullptr_t)
        {
            unreference();
            instance_ = nullptr;
            return *this;
        }

        Ref& operator=(const Ref<Type>& other)
        {
            // Avoid self-assignment.
            if (this == &other)
            {
                return *this;
            }

            other.reference();
            unreference();
            instance_ = other.instance_;
            return *this;
        }

        template<typename Other>
        Ref& operator=(const Ref<Other>& other)
        {
            other.reference();
            unreference();
            instance_ = static_cast<Type*>(other.instance_);
            return *this;
        }

        template<typename Other>
        Ref& operator=(Ref<Other>&& other)
        {
            unreference();
            instance_ = other.instance_;
            other.instance_ = nullptr;
            return *this;
        }

        inline operator bool() const
        {
            return instance_ != nullptr;
        }

        inline Type& operator*()
        {
            return *instance_;
        }

        inline const Type& operator*() const
        {
            return *instance_;
        }

        inline Type* operator->()
        {
            return instance_;
        }

        inline const Type* operator->() const
        {
            return instance_;
        }

        inline Type* get()
        {
            return instance_;
        }

        inline const Type* get() const
        {
            return instance_;
        }

        template<typename Other>
        inline Ref<Other> cast_to()
        {
            return Ref<Other>(*this);
        }

        template<typename Other>
        inline const Ref<Other> cast_to() const
        {
            return Ref<Other>(*this);
        }

    private:
        void reference() const
        {
            if (instance_)
                instance_->reference();
        }

        void unreference() const
        {
            if (!instance_)
                return;

            instance_->unreference();
            if (instance_->ref_count() <= 0)
                release();
        }

        void release() const
        {
            delete instance_;
            instance_ = nullptr;
        }

        mutable Type* instance_;

        template<typename Other>
        friend class Ref;
    };

}