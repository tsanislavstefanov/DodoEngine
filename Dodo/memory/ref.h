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

        void Reference() const { m_RefCount.fetch_add(1, std::memory_order::relaxed); }
        void UnReference() const { m_RefCount.fetch_sub(1, std::memory_order::acq_rel); }
        inline uint64_t GetRefCount() const { return m_RefCount.load(); }

    private:
        mutable std::atomic<uint64_t> m_RefCount = 0;
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

        Ref() = default;
        Ref(std::nullptr_t) {}

        Ref(Type* instance)
            : m_Instance(instance)
        {
            Reference();
        }

        Ref(const Ref& other)
            : m_Instance(other.m_Instance)
        {
            Reference();
        }

        template<typename Other>
        Ref(const Ref<Other>& other)
            : m_Instance(static_cast<Type*>(other.m_Instance))
        {
            Reference();
        }

        template<typename Other>
        Ref(Ref<Other>&& other)
            : m_Instance(static_cast<Type*>(other.m_Instance))
        {
            other.m_Instance = nullptr;
        }

        ~Ref()
        {
            UnReference();
        }

        Ref& operator=(std::nullptr_t)
        {
            UnReference();
            m_Instance = nullptr;
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
            UnReference();
            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename Other>
        Ref& operator=(const Ref<Other>& other)
        {
            other.reference();
            UnReference();
            m_Instance = static_cast<Type*>(other.m_Instance);
            return *this;
        }

        template<typename Other>
        Ref& operator=(Ref<Other>&& other)
        {
            UnReference();
            m_Instance = other.m_Instance;
            other.m_Instance = nullptr;
            return *this;
        }

        inline operator bool() const
        {
            return m_Instance != nullptr;
        }

        inline Type& operator*()
        {
            return *m_Instance;
        }

        inline const Type& operator*() const
        {
            return *m_Instance;
        }

        inline Type* operator->()
        {
            return m_Instance;
        }

        inline const Type* operator->() const
        {
            return m_Instance;
        }

        inline Type* get()
        {
            return m_Instance;
        }

        inline const Type* get() const
        {
            return m_Instance;
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
        void Reference() const
        {
            if (m_Instance)
                m_Instance->Reference();
        }

        void UnReference() const
        {
            if (!m_Instance)
                return;

            m_Instance->UnReference();
            if (m_Instance->GetRefCount() <= 0)
                release();
        }

        void release() const
        {
            delete m_Instance;
            m_Instance = nullptr;
        }

        mutable Type* m_Instance = nullptr;

        template<typename Other>
        friend class Ref;
    };

}