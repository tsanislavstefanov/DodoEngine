#include "pch.h"
#include "RenderCommandBuffer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND BATCH ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    RenderCommandBatch::RenderCommandBatch()
    {
        m_Buffer = new uint8_t[MaxBufferSize];
        Reset();
    }

    RenderCommandBatch::~RenderCommandBatch()
    {
        delete[] m_Buffer;
    }

    void* RenderCommandBatch::Allocate(RenderCommand cmd, size_t size)
    {
        const size_t requiredSize = sizeof(cmd) + sizeof(size) + size;
        if (m_CurrentPosition + requiredSize > m_Buffer + MaxBufferSize)
        {
            return nullptr;
        }

        // RenderCommand is a wrapper that will call the submitted command.
        // Will always be either 4 or 8 bytes.
        *reinterpret_cast<RenderCommand*>(m_CurrentPosition) = cmd;
        m_CurrentPosition += sizeof(cmd);

        *reinterpret_cast<size_t*>(m_CurrentPosition) = size;
        m_CurrentPosition += sizeof(size);

        // The memory block allocated for the submitted command.
        void* memory = m_CurrentPosition;
        m_CurrentPosition += size;

        m_CommandCount++;
        return memory;
    }

    void  RenderCommandBatch::Execute()
    {
        uint8_t* memory = m_Buffer;
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

    void  RenderCommandBatch::Reset()
    {
        m_CurrentPosition = m_Buffer;
        m_CommandCount    = 0;
    }

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND BUFFER ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void* RenderCommandBuffer::Allocate(RenderCommand cmd, size_t size)
    {
        RenderCommandBatch& batch = GetCurrentBatch();
        void* memory = batch.Allocate(cmd, size);
        if (memory)
        {
            return memory;
        }

        return m_Batches.emplace_back().Allocate(cmd, size);
    }

    void  RenderCommandBuffer::Execute()
    {
        for (auto& batch : m_Batches)
        {
            batch.Execute();
        }
    }

    RenderCommandBatch& RenderCommandBuffer::GetCurrentBatch()
    {
        if (m_Batches.empty())
        {
            return m_Batches.emplace_back();
        }

        return m_Batches.back();
    }
}