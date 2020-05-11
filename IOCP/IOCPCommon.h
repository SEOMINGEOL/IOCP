#pragma once

#ifndef __IOCP_COMMON_H__
#define __IOCP_COMMON_H__

#define MAX_BUF_SIZE 1024
#define SERVER_PORT 3500

#include <iostream>
#include <string>
#include <WinSock2.h>

enum {
    Normal = 0,
    Waring,
    Error
};

enum {
    WinSock = 0,
    INVALID,
    BIND,
    LISTEN,
    ACCEPT
};

namespace Log_Form
{
    static std::string Format(std::string log_data, int error_code)
    {
        std::string log;
        log = log_data + "(Error Code : " + std::to_string(error_code) + ")";

        return log;
    }
}

static void Log(int level, std::string log_data)
{
    std::string log;
    switch (level)
    {
    case Normal:
        log = "(Normal)Log : ";
        break;
    case Waring:
        log = "(Waring)Log: ";
        break;
    case Error:
        log = "(Error)Log : ";
        break;
    }

    log += log_data;
    std::cout << log << std::endl;
}


static void Log_printf(int level, const char* log_data, ...)
{
    va_list va;
    char buf[MAX_BUF_SIZE];

    va_start(va, log_data);
    vsprintf_s(buf, log_data, va);
    va_end(va);

    std::cout << buf << std::endl;
}


#endif // !__IOCP_COMMON__
