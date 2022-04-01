#pragma once
#include "albc/albc_api_common.h"
#include "primitive_types.h"
#include <exception>

#define CALBC_HANDLE_IMPL(name) struct name

static AlbcException *ThrowApiException(const string &msg, int code = -1)
{
    auto *e = new AlbcException();
    e->what = new char[msg.size() + 1];
    msg.copy(const_cast<char *>(e->what), msg.size() + 1);
    e->code = code;
    return e;
}

[[maybe_unused]] static void TranslateException(AlbcException **ep, string &out_msg)
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
        out_msg.assign(e.what());
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

#define ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(receiver, ...)                                                          \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        std::string msg;                                                                                               \
        TranslateException(receiver, msg);                                                                             \
        LOG_E(__VA_ARGS__, msg);                                                                                       \
        if (receiver == nullptr)                                                                                       \
        {                                                                                                              \
            LOG_W("An exception was suppressed: ", msg);                                                               \
        }                                                                                                              \
    }
