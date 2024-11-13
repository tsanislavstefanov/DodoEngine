#pragma once

namespace Dodo {

    class RenderCommandQueue {
    public:
        RenderCommandQueue();

        template<typename T>
        void enqueue(T* instance, void(T::*member)());
        void flush();

    private:
        struct Command {
            virtual ~Command() = default;
            virtual void execute() = 0;
        };

        template<typename T>
        class RenderCommand : public Command {
        public:
            RenderCommand(T* instance, void(T::*member)())
                : instance{instance}, member{member} {}

            void execute() override { (instance->*member)(); }

        private:
            T* instance = nullptr;
            void(T::*member)() = nullptr;
        };

        void ensure_capacity(size_t size);

        std::vector<uint8_t> queue{};
        size_t offset = 0;
        uint32_t command_count = 0;
        std::vector<size_t> command_offsets{};
    };

    template<typename T>
    inline void RenderCommandQueue::enqueue(T* instance, void(T::* member)()) {
        const auto command_size = sizeof(RenderCommand<T>);
        ensure_capacity(command_size);

        new (queue.data() + offset) RenderCommand<T>(instance, member);
        offset += command_size;
        command_count++;
        command_offsets.push_back(command_size);
    }

}