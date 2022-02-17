#include "locale_util.h"

#ifdef _WIN32
#include <Windows.h>
#endif

// convert from UTF-8 to OS charset
string albc::toOSCharset(const string &src)
{
#ifdef _WIN32
    static bool has_set_cp = false;
    if (!has_set_cp)
    {
        // Set console code page to UTF-8 so console known how to interpret string data
        SetConsoleOutputCP(CP_UTF8);
        has_set_cp = true;
    }
#endif
    
    return src;
}
