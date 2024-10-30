#pragma once

#include "Event.h"
#include "Bindings/Func.h"
#include "Drivers/Vulkan/VulkanUtils.h"

namespace Dodo {

    class WindowResizeEvent : public RegisterEventOfType<WindowResizeEvent>
    {
    public:
        const uint32_t Width, Height;

        WindowResizeEvent(uint32_t width, uint32_t height)
            : Width{width},
              Height{height}
        {
        }
    };

    class WindowCloseEvent : public RegisterEventOfType<WindowCloseEvent>
    {
    };



    struct WindowSpecifications
    {
        uint32_t Width = 1280, Height = 720;
        std::string Title = "Dodo Engine";
        bool Maximized = false;
    };

    class Window : public RefCounted
    {
    public:
        static Ref<Window> Create(WindowSpecifications&& specs);

        Window(WindowSpecifications&& specs)
            : m_Specifications(std::move(specs))
        {}

        virtual ~Window() = default;

        virtual void ProcessEvents() = 0;
        virtual void* GetNativeHandle() const = 0;
        inline uint32_t GetWidth() const { return m_Specifications.Width; }
        inline uint32_t GetHeight() const { return m_Specifications.Height; }
        inline bool HasFocus() const { return m_HasFocus; }

        void SetEventCallback(EventCallback&& callback)
        {
            m_EventCallback = std::move(callback);
        }

#ifdef DODO_USE_VULKAN
        virtual std::vector<std::string> GetRequiredVulkanInstanceExtensions() const = 0;
        virtual VkSurfaceKHR CreateVulkanSurface(VkInstance instance) const = 0;
#endif

    protected:
        WindowSpecifications m_Specifications{};
        bool m_HasFocus = true;
        EventCallback m_EventCallback{};
    };

}