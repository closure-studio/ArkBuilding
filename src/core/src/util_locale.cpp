#include "util_locale.h"
#include "util_log.h"

#ifdef ALBC_HAVE_ICU
#if __has_include(<icu.h>)
#   include <icu.h>
#else   
#   include <unicode/ucnv.h>
#endif
#endif

namespace albc::util
{

std:: string ToTargetLocale(const std::string_view &src, const char *from_code, const char *to_code)
{
    if (!from_code || !to_code)
        return {};

    if (strcmp(from_code, to_code) == 0)
        return src.data();

    try
    {
#ifdef ALBC_HAVE_ICU
        UErrorCode err = U_ZERO_ERROR;
        auto in_len = static_cast<int32_t>(src.size() + 1);
        auto out_buf_size = static_cast<int32_t>(in_len * 4 + 1);
        auto out_buf = std::make_unique<char[]>( out_buf_size );

        ucnv_convert(to_code, from_code, out_buf.get(), out_buf_size, src.data(), in_len, &err);
        if (U_FAILURE(err))
        {
            throw std::runtime_error("Failed to convert string to target locale");
        }
        return { out_buf.get() };
#else
        (void)src; (void)from_code; (void)to_code;
        static std::once_flag once;
        std::call_once(once, []() { std::cout << "ALBC|ICU is not available" << std::endl; });
        return src.data();
#endif
    }
    catch (std::exception &e)
    {
        LOG_E("Failed to convert string: ", e.what(), " from_code: ", from_code, " to_code: ", to_code);
        return src.data();
    }
}

bool CheckTargetLocale(const char *locale)
{
#ifdef ALBC_HAVE_ICU
    UErrorCode err = U_ZERO_ERROR;
    auto res = ucnv_open(locale, &err);
    if (U_FAILURE(err))
    {
        return false;
    }
    ucnv_close(res);
    return true;
#else
    (void)locale;
    static std::once_flag once;
    std::call_once(once, []() { std::cout << "ALBC|ICU is not available" << std::endl; });
    return true;
#endif
}
} // namespace albc
