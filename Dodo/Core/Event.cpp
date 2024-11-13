#include "pch.h"
#include "event.h"

namespace Dodo {

    uint64_t Event::generate_new_type_id() {
        static uint64_t type_id = 0;
        return type_id++;
    }

}