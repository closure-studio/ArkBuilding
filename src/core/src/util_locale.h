#pragma once

#include "albc_types.h"

#include <mutex>
#include <cstring>

namespace albc::util
{
constexpr const char* kExecutableLocale = "UTF-8";
constexpr const char* kDefaultLocale = kExecutableLocale;
class GlobalLocale
{
    inline static std::string locale_;
    inline static std::mutex mutex_;

  public:
    static std::string GetLocale()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return !locale_.empty() ? locale_ : kDefaultLocale;
    }

    static void SetLocale(const std::string& locale)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        locale_ = !locale.empty() ? locale : kDefaultLocale;
    }
};

std::string ToTargetLocale(const std::string_view &src, const char* from_code = kDefaultLocale, const char* to_code = kDefaultLocale);

bool CheckTargetLocale(const char* locale);
}