#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SHADER //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Shader : public RefCounted
    {
    public:
        static Ref<Shader> Create(const std::string& assetPath);

        virtual ~Shader() = default;

        virtual void Reload() = 0;
        virtual void Reload_RenderThread() = 0;
    };

}