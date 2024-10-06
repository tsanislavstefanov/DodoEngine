#pragma once

namespace Dodo {

    enum class ThreadAffinity
    {
        render_thread,
        auto_count,
        none
    };

    class Thread : public RefCounted
    {
    public:
        static Ref<Thread> create(
                const std::string& name,
                ThreadAffinity affinity = ThreadAffinity::none);

        Thread() = default;
        virtual ~Thread() = default;

        template<typename Func, typename... Args>
        void dispatch(Func&& func, Args&&... args)
        {
            thread_ = std::thread(std::forward<Func>(func), std::forward<Args>(args)...);
        }

        void join();

    protected:
        std::thread thread_{};
    };

}