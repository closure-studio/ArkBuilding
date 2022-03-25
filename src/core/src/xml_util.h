#pragma once
#include "primitive_types.h"

namespace albc::xml
{
    // strips all xml tags from a string, UTF-8 encoded
    static string strip_xml_tags(const string &str, const char tag_start = '<', const char tag_end = '>')
    {
        string result = str;
        size_t start_pos = 0;
        while ((start_pos = result.find(tag_start, start_pos)) != string::npos)
        {
            size_t end_pos = result.find(tag_end, start_pos);
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