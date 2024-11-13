#pragma once

#include "core/thread_pool.h"

namespace Dodo {

    class RenderEngine {
    public:
        RenderEngine();

        void sync_and_draw();
    private:
        static void render_thread_loop(RenderEngine* render_engine);

        void init();

        bool exit = false;
    };

}