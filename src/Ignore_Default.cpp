/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxsignal/Default.hpp"
#include "cxxsignal/Ignore.hpp"

#include <cstring>

namespace cxxsignal {

/**
 * @brief create reusable sigactions
 */
static struct Actions final {
    struct sigaction _ignore {};  /**< ignore signal */
    struct sigaction _default {}; /**< use default handler */

    /**
     * @brief initialize reusable sigactions
     */
    Actions() {
        memset(&_ignore, 0, sizeof(_ignore));
        memset(&_default, 0, sizeof(_default));

#ifdef COMPILER_CLANG
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif
        _ignore.sa_handler  = SIG_IGN;
        _default.sa_handler = SIG_DFL;
#ifdef COMPILER_CLANG
#    pragma GCC diagnostic pop
#endif
    }
} actions;

[[maybe_unused]] Ignore::Ignore(int signal_number) : SignalHandler(signal_number) {
    SignalHandler::current_signal_action = actions._ignore;
}

[[maybe_unused]] Default::Default(int signal_number) : SignalHandler(signal_number) {
    SignalHandler::current_signal_action = actions._default;
}

}  // namespace cxxsignal
