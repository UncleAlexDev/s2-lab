//
// Created by LAV on 02.04.2026.
//

#ifndef DBG_H
#define DBG_H

#include "dbg_impl.h"
#include <fmt/core.h>

#define DBG(LEVEL, ...)                                                                                                \
    do {                                                                                                               \
        if (DBG::debug_level >= (LEVEL)) {                                                                             \
            DBG::dbg_print(LEVEL, __PRETTY_FUNCTION__, std::string_view(__VA_ARGS__));                                 \
        }                                                                                                              \
    } while (false)


#define DBGF(LEVEL, fmtstr, ...)                                                                                       \
    do {                                                                                                               \
        if (DBG::debug_level >= (LEVEL)) {                                                                             \
            DBG::dbg_print(LEVEL, __PRETTY_FUNCTION__, fmt::format(fmtstr, __VA_ARGS__));                              \
        }                                                                                                              \
    } while (false)


#define DBGNL(LEVEL)                                                                                                   \
    do {                                                                                                               \
        if (DBG::debug_level >= LEVEL) {                                                                               \
            DBG::dbg_print_nl();                                                                                       \
        }                                                                                                              \
    } while (false)

#endif // DBG_H
