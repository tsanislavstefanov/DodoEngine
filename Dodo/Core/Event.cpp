#include "pch.h"
#include "Event.h"

namespace Dodo {

    uint64_t Event::GenerateNewTypeId() const
    {
        static uint64_t typeId = 0;
        return typeId++;
    }

}