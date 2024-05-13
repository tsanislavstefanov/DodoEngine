#pragma once

#include "Window.h"
#include "Diagnostics/Stopwatch.h"
#include "Renderer/RenderSettings.h"

////////////////////////////////////////////////////////////////
// COMMAND LINE ARGS ///////////////////////////////////////////
////////////////////////////////////////////////////////////////

class CommandLineArgs
{
public:
    CommandLineArgs() = default;

    CommandLineArgs(int count, char** values)
        : m_Count (count )
        , m_Values(values)
    {}

    const char* operator[](size_t index) const
    {
        ASSERT((index < m_Count) && (index >= 0), "Index out of range!");
        return m_Values[index];
    }

    [[nodiscard]] int GetCount() const
    {
        return m_Count;
    }

    [[nodiscard]] char** GetValues() const
    {
        return m_Values;
    }

private:
    int    m_Count  = 0;
    char** m_Values = nullptr;
};

////////////////////////////////////////////////////////////////
// APPLICATION SPECS ///////////////////////////////////////////
////////////////////////////////////////////////////////////////

struct ApplicationSpecs
{
    CommandLineArgs  CmdLineArgs{};
    uint32_t         Width         = 0;
    uint32_t         Height        = 0;
    std::string      Title{};
    bool             ShowFrameRate = false;
    bool             EnableImGui   = false;
    RenderSettings   RenderSettings{};
};

////////////////////////////////////////////////////////////////
// APPLICATION /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class Application
{
public:
    static Application& GetCurrent()
    {
        return *s_App;
    }

    explicit Application(ApplicationSpecs specs);

    virtual ~Application() = default;

    [[nodiscard]] const ApplicationSpecs& GetSpecs() const
    {
        return m_Specs;
    }

    Ref<Window> GetWindow()
    {
        return m_Window;
    }

    [[nodiscard]] Ref<Window> GetWindow() const
    {
        return m_Window;
    }

    void Run();

protected:
    virtual void OnInit() {}

    virtual void OnUpdate() {}

    virtual void OnBeginRender() {}

    virtual void OnRender() {}

    virtual void OnEndRender() {}

    virtual void OnDispose() {}

private:
    static Application* s_App;

    void Init();

    void Dispose();

    ApplicationSpecs m_Specs;
    Ref<Window>      m_Window    = nullptr;
    bool             m_IsRunning = true;
    Stopwatch        m_FrameRateWatch{};
    uint32_t         m_FrameRate = 0;
};