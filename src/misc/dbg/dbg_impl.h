//
// Created by LAV on 02.04.2026.
//

#ifndef DBG_IMPL_H
#define DBG_IMPL_H

#include <string>

class DBG
{
public:
    inline static int print_thread_id = 0;
    constexpr static int FUNCNAMEWIDTH = 35;
    inline static int debug_level = 0;

public:
    static void dbg_print(int level, const char *funcnm, std::string_view message);
    static void dbg_print_nl();

private:
    static void trim(std::string &str);
    static void funcname_alignment(std::string &funcname);
};

#endif        // DBG_IMPL_H
