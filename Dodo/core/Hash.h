#pragma once

namespace Dodo {

    class FNV1a
    {
    public:
        size_t operator()(const std::string& octets);
    };

}