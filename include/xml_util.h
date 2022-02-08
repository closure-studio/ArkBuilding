#include "primitive_types.h"

namespace albc::xml{
    // strips all xml tags from a string
    string strip_xml_tags(const string& str, const string replacement = "", const char tag_start = '<', const char tag_end = '>'){
        string result;
        result.reserve(str.size());
        for (size_t pos = 0; pos < str.size(); ++pos){
            if (str[pos] == tag_start){
                while (str[++pos] != tag_end){
                    if (pos == str.size())
                        break;
                }

                if (!replacement.empty())
                    result += replacement;
            }
            else{
                result += str[pos];
            }
        }
        return result;
    }
}