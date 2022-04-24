#pragma once
#include "albc_types.h"

namespace albc::xml
{
    // strips all xml tags from a string, UTF-8 encoded
    [[maybe_unused]]
    static std::string strip_xml_tags(const std::string &str)
    {
        std::string result = str;
        size_t start_pos = 0;
        while ((start_pos = result.find('<', start_pos)) != std::string::npos)
        {
            size_t end_pos = result.find('>', start_pos);
            if (end_pos != std::string::npos)
            {
                result.erase(start_pos, end_pos - start_pos + 1);
            }
            else
            {
                result.erase(start_pos, 1);
            }
        }
        return result;
    }
}