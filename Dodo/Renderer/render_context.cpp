#include "pch.h"
#include "render_context.h"

namespace Dodo {

    uint32_t RenderContext::get_adapter_count() const {
        return _adapters.size();
    }

    const RenderContext::Adapter& RenderContext::get_adapter(size_t index) const {
        DODO_ASSERT((index >= 0) && (index < _adapters.size()), "Requested adapter index out of range!");
        return _adapters.at(index);
    }

}