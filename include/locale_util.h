#pragma once

#include "primitive_types.h"

#ifdef _WIN32
#include "winnls.h"
#endif

namespace albc
{
    // convert from UTF-8 to OS charset
    inline string toOSCharset(const string &src)
    {
#ifdef _WIN32
        // conv utf8 to gbk
        int len = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, nullptr, 0);
        auto *wszGBK = new wchar_t[len + 1];
        memset(wszGBK, 0, len * 2 + 2);
        ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, wszGBK, len);
        len = ::WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, nullptr, 0, nullptr, nullptr);
        char *szGBK = new char[len + 1];
        memset(szGBK, 0, len + 1);
        ::WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, nullptr, nullptr);
        string strTemp(szGBK);
        delete[] szGBK;
        delete[] wszGBK;
        return strTemp;
#else
        return src;
#endif
    }
}