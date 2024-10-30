#pragma once

namespace Dodo {

    class File
    {
    public:
        static std::string ReadAndSkipBOM(const std::filesystem::path& filePath);
    };

}