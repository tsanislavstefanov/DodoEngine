#pragma once

#include <atomic>
#include <cstdint>
#include <utility>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // REF COUNTED /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RefCounted
    {
    public:
        RefCounted() = default;

        virtual ~RefCounted() = default;

        [[nodiscard]] uint64_t GetRefCount() const
        {
            return m_RefCount.load();
        }

        void Reference()
        {
            ++m_RefCount;
        }

        void UnReference()
        {
            --m_RefCount;
        }

    private:
        std::atomic<uint64_t> m_RefCount = 0;
    };

    ////////////////////////////////////////////////////////////////
    // REF /////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    template<typename Type>
    class Ref
    {
    public:
        template<typename... Args>
        static Ref<Type> Create(Args&&... args)
        {
            return Ref<Type>(new Type(std::forward<Args>(args)...));
        }

        Ref()
            : m_Instance(nullptr)
        {}

        Ref(std::nullptr_t)
            : m_Instance(nullptr)
        {}

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

        Ref& operator=(const Ref& other)
        {
            // To avoid self-assignment.
            if (this == &other)
            {
                return *this;
            }

            other.Reference();
            UnReference();
            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename Other>
        Ref& operator=(const Ref<Other>& other)
        {
            other.Reference();
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

        operator bool() const
        {
            return m_Instance != nullptr;
        }

        Type& operator*()
        {
            return *m_Instance;
        }

        const Type& operator*() const
        {
            return *m_Instance;
        }

        Type* operator->()
        {
            return m_Instance;
        }

        const Type* operator->() const
        {
            return m_Instance;
        }

        Type* GetRaw()
        {
            return m_Instance;
        }

        const Type* GetRaw() const
        {
            return m_Instance;
        }

        template<typename Other>
        Ref<Other> CastTo()
        {
            return Ref<Other>(*this);
        }

        template<typename Other>
        Ref<Other> CastTo() const
        {
            return Ref<Other>(*this);
        }

        void Release()
        {
            delete m_Instance;
            m_Instance = nullptr;
        }

    private:
        void Reference()
        {
            if (m_Instance)
            {
                m_Instance->Reference();
            }
        }

        void UnReference()
        {
            if (!m_Instance)
            {
                return;
            }

            m_Instance->UnReference();
            if (m_Instance->GetRefCount() <= 0)
            {
                Release();
            }
        }

        Type* m_Instance;

        template<typename Other>
        friend class Ref;
    };

}