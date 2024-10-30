#include "pch.h"
#include "File.h"

namespace Dodo {

    namespace Utils {

        static void SkipBOM(std::istream& is)
        {
            char bom[4]{}; // Include null terminator.
            is.seekg(0, std::ios::beg);
            is.read(bom, 3);
            // BOM not detected, nothing to skip.
            // Otherwise effectively skipped, because it has already been read.
            if (strcmp(bom, "\xEF\xBB\xBF") != 0)
                is.seekg(0, std::ios::beg);
        }

    }

    std::string File::ReadAndSkipBOM(const std::filesystem::path& filePath)
    {
        std::ifstream is(filePath, std::ios::binary);
        if (!is)
        {
            DODO_LOG_ERROR("Failed to read file: {0}.", filePath.string());
            return {};
        }

        Utils::SkipBOM(is);
        std::string result((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        is.close();
        return result;
    }

}