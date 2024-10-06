#include "pch.h"
#include "file.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static void skip_bom(std::istream& is)
        {
            char bom[4]{}; // Include null terminator.
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

    std::string File::read_and_skip_bom(const std::filesystem::path& file_path)
    {
        std::ifstream is(file_path, std::ios::binary);
        if (!is)
        {
            return {};
        }

        Utils::skip_bom(is);
        std::string result((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        is.close();
        return result;
    }

}