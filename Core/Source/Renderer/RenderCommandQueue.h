#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    using RenderCommand = void(*)(void*);

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND CHUNK ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandChunk
    {
    public:
        RenderCommandChunk();

        ~RenderCommandChunk();

        void* Allocate(RenderCommand cmd, size_t size);
        void  Execute();
    private:
        void  Reset();

        uint8_t* m_Data = nullptr;
        uint8_t* m_CurrentPosition = nullptr;
        uint64_t m_CommandCount = 0;
    };

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND QUEUE ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue() = default;

        template<typename Job>
        void Submit(Job&& job)
        {
            RenderCommand cmd = [](void* memory)
            {
                auto job = (Job*)memory;
                (*job)();

                job->~Job();
            };

            new (Allocate(cmd, sizeof(Job))) Job(std::forward<Job>(job));
        }

        void  Execute();
    private:
        RenderCommandChunk& GetCurrentChunk();
        void* Allocate(RenderCommand cmd, size_t size);

        std::deque<RenderCommandChunk> m_Chunks{};
    };

}