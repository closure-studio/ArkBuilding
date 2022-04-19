#include "locale_util.h"
#include "log_util.h"

namespace albc::util
{
#ifdef ALBC_HAVE_ICONV
#include <iconv.h>
#endif

// iconv wrapper
std:: string ToTargetLocale(const std::string_view &src, const char *from_code, const char *to_code)
{
    std::string to_code_str(to_code ? to_code : GlobalLocale::GetLocale());
    if (src.empty())
        return "";

    if (!from_code)
        from_code = kExecutableLocale;

    if (to_code_str == from_code)
        return src.data();

    try
    {
#ifdef ALBC_HAVE_ICONV
        iconv_t cd = iconv_open(to_code_str.c_str(), from_code);
        if (cd == (iconv_t)-1)
        {
            LOG_E("iconv_open failed: ", strerror(errno), " from_code: ", from_code, " to_code: ", to_code_str);
            return src.data();
        }

        char *in_buf = const_cast<char *>(src.data());
        size_t src_size = src.size();
        size_t dst_size = src_size * 4;
        auto dst_ptr = std::make_unique<char[]>(dst_size);
        char *out_buf = dst_ptr.get();
        size_t dst_left = dst_size;

        if (iconv(cd, &in_buf, &src_size, &out_buf, &dst_left) == (size_t)-1)
        {
            LOG_E("iconv failed: ", strerror(errno), " from_code: ", from_code, " to_code: ", to_code_str);
            iconv_close(cd);
            return src.data();
        }

        iconv_close(cd);
        return {dst_ptr.get(), dst_size - dst_left};
#else
        static std::once_flag once;
        std::call_once(once, []() { LOG_W("iconv is not available"); });
        return src.data();
#endif
    }
    catch (std::exception &e)
    {
        LOG_E("Failed to convert string: ", e.what(), " from_code: ", from_code, " to_code: ", to_code_str);
        return src.data();
    }
}

bool CheckTargetLocale(const char *locale)
{
#ifdef ALBC_HAVE_ICONV
    if (iconv_t cd = iconv_open(locale, locale))
    {
        iconv_close(cd);
        return true;
    }
    else
    {
        LOG_E("iconv_open failed: ", strerror(errno), " locale: ", locale);
        return false;
    }
#else
    static std::once_flag once;
    std::call_once(once, []() { LOG_W("iconv is not available"); });
    return true;
#endif
}
} // namespace albc
