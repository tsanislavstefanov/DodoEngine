#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // THREAD //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Thread : public RefCounted
    {
    public:
        static Ref<Thread> Create(std::string name);

        Thread(std::string name)
            : m_Name(std::move(name))
        {}

        template<typename Job, typename... Args>
        void Dispatch(Job&& job, Args&&... args)
        {
            m_Thread = std::thread(job, std::forward<Args>(args)...);
            SetName();
        }

        virtual void Join() = 0;

    protected:
        virtual void SetName() = 0;

        std::string m_Name;
        std::thread m_Thread{};
    };

}