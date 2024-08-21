#include "pch.h"
#include "File.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static void SkipBom(std::istream& is)
        {
            char bom[4] = { 0 }; // Include null terminator.
            is.seekg(0, std::ios::beg);
            is.read(bom, 3);
            // BOM not detected, nothing to skip.
            // Otherwise effectively skipped, because it has already been read.
            if (strcmp(bom, "\xEF\xBB\xBF") != 0)
            {
                is.seekg(0, std::ios::beg);
            }
        }

    }

    ////////////////////////////////////////////////////////////////
    // FILE ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    std::string File::ReadAndSkipBom(const std::filesystem::path& filePath)
    {
        std::ifstream is(filePath, std::ios::binary);
        if (!is)
        {
            return "";
        }

        Utils::SkipBom(is);
        std::string result((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        is.close();
        return result;
    }

}