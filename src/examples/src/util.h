//
// Created by Nonary on 2022/4/24.
//
#pragma once
#include "albc/albc.h"
#include <stdexcept>
#include <string>

struct AlbcExceptionDelegate
{
    AlbcException *e = nullptr;
    AlbcException** operator()() { return &e; }

    ~AlbcExceptionDelegate() noexcept(false)
    {
        if (!e)
            return;

        try
        {
            std::string msg(e->what);
            albc::FreeException(e);
            throw std::runtime_error(msg);
        }
        catch (...)
        {
            if (!std::uncaught_exceptions())
                std::rethrow_exception(std::current_exception());
        }
    }
    // 语言丁真，鉴定为：坏文明
    // 推荐使用C风格的异常处理（详见C接口样例）
    // 此处仅为了省事以及示例代码的简洁
};

#define THROW_ON_ALBC_ERROR AlbcExceptionDelegate()()
