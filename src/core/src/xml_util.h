#pragma once
#include "primitive_types.h"

namespace albc::xml
{
    // strips all xml tags from a string
    [[maybe_unused]]
    static string strip_xml_tags(const string &str)
    {
        string result = str;
        size_t start_pos = 0;
        while ((start_pos = result.find('<', start_pos)) != string::npos)
        {
            size_t end_pos = result.find('>', start_pos);
            if (end_pos != string::npos)
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