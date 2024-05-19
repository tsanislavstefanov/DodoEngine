#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    using RenderCommand = void(*)(void*);

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND BATCH ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandBatch
    {
    public:
        RenderCommandBatch();

        ~RenderCommandBatch();

        void* Allocate(RenderCommand cmd, size_t size);

        void  Execute();
    private:
        static constexpr size_t MaxBufferSize = 10 * 1024; // 10KB.

        void  Reset();

        uint8_t* m_Buffer          = nullptr;
        uint8_t* m_CurrentPosition = nullptr;
        uint64_t m_CommandCount    = 0;
    };

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND BUFFER ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandBuffer
    {
    public:
        RenderCommandBuffer() = default;

        void* Allocate(RenderCommand cmd, size_t size);

        void  Execute();
    private:
        RenderCommandBatch& GetCurrentBatch();

        std::vector<RenderCommandBatch> m_Batches{};
    };

}