#pragma once

#include "primitive_types.h"

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

// convert from UTF-8 to OS charset
std::string ToTargetLocale(const std::string_view &src, const char* from_code = nullptr, const char* to_code = nullptr);

bool CheckTargetLocale(const char* locale);
}