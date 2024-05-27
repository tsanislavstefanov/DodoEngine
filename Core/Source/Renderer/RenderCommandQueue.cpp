#include "pch.h"
#include "RenderCommandQueue.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND CHUNK ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static constexpr size_t MaxBufferSize = 10 * 1024;

    RenderCommandChunk::RenderCommandChunk()
    {
        m_Data = new uint8_t[MaxBufferSize];
        Reset();
    }

    RenderCommandChunk::~RenderCommandChunk()
    {
        delete[] m_Data;
    }

    void* RenderCommandChunk::Allocate(RenderCommand cmd, size_t size)
    {
        const size_t requiredSize = sizeof(RenderCommand) + sizeof(size_t) + size;
        if (m_CurrentPosition + requiredSize > m_Data + MaxBufferSize)
        {
            return nullptr;
        }

        *reinterpret_cast<RenderCommand*>(m_CurrentPosition) = cmd;
        m_CurrentPosition += sizeof(RenderCommand);

        *reinterpret_cast<size_t*>(m_CurrentPosition) = size;
        m_CurrentPosition += sizeof(size_t);

        void* memory = m_CurrentPosition;
        m_CurrentPosition += size;

        m_CommandCount++;
        return memory;
    }

    void RenderCommandChunk::Execute()
    {
        uint8_t* memory = m_Data;
        for (uint32_t i = 0; i < m_CommandCount; i++)
        {
            RenderCommand cmd = *reinterpret_cast<RenderCommand*>(memory);
            memory += sizeof(RenderCommand);

            const auto size = *reinterpret_cast<size_t*>(memory);
            memory += sizeof(size_t);
            cmd(memory);
            memory += size;
        }

        Reset();
    }

    void RenderCommandChunk::Reset()
    {
        m_CurrentPosition = m_Data;
        m_CommandCount = 0;
    }

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND BUFFER ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    RenderCommandQueue::RenderCommandQueue()
    {
    }

    void RenderCommandQueue::Execute()
    {
        for (auto& chunk : m_Chunks)
        {
            chunk.Execute();
        }
    }

    RenderCommandChunk& RenderCommandQueue::GetCurrentChunk()
    {
        if (m_Chunks.empty())
        {
            return m_Chunks.emplace_back();
        }

        return m_Chunks.back();
    }

    void* RenderCommandQueue::Allocate(RenderCommand cmd, size_t size)
    {
        RenderCommandChunk& chunk = GetCurrentChunk();
        void* memory = chunk.Allocate(cmd, size);
        if (memory)
        {
            return memory;
        }

        return m_Chunks.emplace_back().Allocate(cmd, size);
    }

}