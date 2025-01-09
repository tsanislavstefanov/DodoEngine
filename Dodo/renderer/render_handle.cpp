#include "pch.h"
#include "render_handle.h"

namespace Dodo {

    RenderHandle::operator bool() const {
        return handle != 0;
    }

}
