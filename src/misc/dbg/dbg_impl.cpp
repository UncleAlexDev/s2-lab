//
// Created by LAV on 02.04.2026.
//

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include <fmt/core.h>
#include <fmt/chrono.h>

#include "dbg_impl.h"

static std::mutex _mutex;

//---------------------------------------------------------
void DBG::trim(std::string &str)
{
    const std::string whitespaces = " \t\n\r\f\v";
    const size_t first = str.find_first_not_of(whitespaces);
    if (first == std::string::npos) {
        str.clear();
        return;
    }
    str.erase(0, first);

    const size_t last = str.find_last_not_of(whitespaces);
    if (last != std::string::npos) {
        str.erase(last + 1);
        return;
    }
    str.clear();
}

//---------------------------------------------------------
void DBG::funcname_alignment(std::string &funcname)
{
    std::string_view func_name(funcname);
    if (func_name.find("<lambda()>") != std::string_view::npos) {
        // main(int, char**)::<lambda()>
        // std::shared_ptr<TcpServer>&)::<lambda()>
        const auto pos = func_name.rfind("::");
        if (pos != std::string_view::npos) {
            const auto pos1 = func_name.find('(');
            if (pos1 != std::string_view::npos) {
                const auto str1 = func_name.substr(0, pos1 + 1);
                const auto str2 = func_name.substr(pos - 1);
                char buf[128];
                auto sz = std::snprintf(buf, sizeof(buf), "%s%s", std::string(str1).c_str(), std::string(str2).c_str());
                if(sz > 0) {
                    funcname = std::string(buf, sz);
                }
            }
        }
        return;
    }

    auto pos = func_name.rfind('(');
    if (pos != std::string_view::npos) {
        func_name.remove_suffix(func_name.size() - pos);
    }

    pos = func_name.rfind("::");
    if (pos != std::string_view::npos) {
        const auto pos1 = func_name.rfind(' ', pos + 1);
        if (pos1 != std::string_view::npos) {
            func_name.remove_prefix(pos1 + 1);
        } else {
            // func_name.remove_prefix(pos + 2);
        }
    }
    pos = func_name.rfind(' ');
    if (pos != std::string_view::npos) {
        func_name.remove_prefix(pos + 1);
    }
    funcname = func_name;
}

//---------------------------------------------------------
void DBG::dbg_print_nl()
{
    auto lock = std::lock_guard(_mutex);
    std::cout << std::endl << std::flush;
}

//---------------------------------------------------------
void DBG::dbg_print(const int level, const char *funcnm, std::string_view message)
{
    // Для локального времени -  fmt::localtime
    // auto now_t = std::chrono::system_clock::to_time_t(now);
    // std::string msg = fmt::format("{:%F %T}", *std::localtime(&now_t));

    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::floor<std::chrono::milliseconds>(now);
    std::time_t timet = std::chrono::system_clock::to_time_t(milliseconds);
    std::string datetime = fmt::format("{:%Y-%m-%d %H:%M:%S}.{:03}",
        *std::localtime(&timet), milliseconds.time_since_epoch().count() % 1000);

    std::string funcname(funcnm);
    funcname_alignment(funcname);

    if (!message.empty() && message[0] == '\n') {
        std::cout << std::endl << std::flush;
        message.remove_prefix(1);
    }

    auto lock = std::lock_guard(_mutex);

    /////std::string msg;
    /////char buf[1024];
    /////char timebuf[64];
    /////std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);
    /////if (DBG::print_thread_id == 1) {
    /////    auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
    /////    msg = std::snprintf(buf, sizeof(buf), "%s.%03ld [%d] 0x%08x %-*s {}", timebuf, ms_part.count(), level,
    /////        tid, FUNCNAMEWIDTH, funcname.c_str(), message);
    /////} else {
    /////    msg = std::snprintf(buf, sizeof(buf), "%s.%03ld [%d] %-*s {}", timebuf, ms_part.count(), level,
    /////        FUNCNAMEWIDTH, funcname.c_str(), message);
    /////}

    std::string msg;
    if (DBG::print_thread_id == 1) {
        auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
        msg = fmt::format("{} [{}] [0x{:08x}] {:<{}.{}} {}",
            datetime, level, tid, fmt::format("{}()", funcname),
            FUNCNAMEWIDTH, FUNCNAMEWIDTH,
            message);
    } else {
        msg = fmt::format("{} [{}] {:<{}.{}} {}",
            datetime, level, fmt::format("{}()", funcname),
            FUNCNAMEWIDTH, FUNCNAMEWIDTH,
            message);
    }

    std::cout << msg << std::endl << std::flush;
}

// //---------------------------------------------------------
// void DBG::dbg_print_raw(const std::string &message)
// {
//     std::string_view msg(message);
//     if (!msg.empty() && msg[0] == '\n') {
//         std::cout << std::endl << std::flush;
//         msg.remove_prefix(1);
//     }
//
//     std::string str{msg};
//     auto lock = std::lock_guard(_mutex);
//     std::cout << str << std::endl << std::flush;
// }

////// //---------------------------------------------------------
////// ///  Пустой вывод
////// void DBG::smart_dispatch(const int level, const char *func)
////// {
//////     dbg_print(level, func, "");
////// }
//////
////// //---------------------------------------------------------
////// ///  Одиночный аргумент (строка или результат std::format)
////// void DBG::smart_dispatch(const int level, const char *func, const std::string_view message)
////// {
//////     dbg_print(level, func, message);
////// }
