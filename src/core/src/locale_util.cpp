#include "locale_util.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <clocale>
#endif

#include <mutex>
#include <iostream>

// convert from UTF-8 to OS charset
const string& albc::toOSCharset(const string &src)
{
    static std::once_flag init_locale_flag;
    std::call_once(init_locale_flag, []() 
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#else
        setlocale(LC_ALL, "en_US.UTF-8");
#endif
    });

    return src;
}
