//
// Created by Nonary on 2022/4/24.
//
#include "api_util.h"

namespace albc::api
{

}
AlbcException *ThrowApiException(const std::string &msg)
{
    auto *e = new AlbcException();
    std::string target_locale_err_msg = albc::util::ToTargetLocale(msg);
    e->what = new char[target_locale_err_msg.length() + 1];
    strcpy(e->what, target_locale_err_msg.c_str());
    return e;
}
void TranslateException(AlbcException **ep, std::string &out_msg)
{
    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception &e)
    {
        if (ep)
        {
            *ep = ThrowApiException(e.what());
        }
        out_msg = e.what();
    }
    catch (...)
    {
        if (ep)
        {
            *ep = ThrowApiException("Unknown exception");
        }
        out_msg = "Unknown exception";
    }
}
