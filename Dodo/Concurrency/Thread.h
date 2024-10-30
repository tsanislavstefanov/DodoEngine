#pragma once

namespace Dodo {

    enum class ThreadAffinity
    {
        RenderThread,
        AutoCount,
        None
    };

    class Thread : public RefCounted
    {
    public:
        static Ref<Thread> Create(const std::string& name, ThreadAffinity affinity = ThreadAffinity::None);

        Thread() = default;
        virtual ~Thread() = default;

        template<typename Func, typename... Args>
        void Dispatch(Func&& func, Args&&... args);
        void Join();

    protected:
        std::thread m_Thread{};
    };

    template<typename Func, typename... Args>
    inline void Thread::Dispatch(Func&& func, Args&&... args)
    {
        m_Thread = std::thread(std::forward<Func>(func), std::forward<Args>(args)...);
    }

}