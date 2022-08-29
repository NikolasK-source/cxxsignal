/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxsignal/SignalHandler.hpp"

#include <cstring>
#include <mutex>
#include <system_error>
#include <vector>

namespace cxxsignal {

static struct SignalMapper final {
    std::vector<SignalHandler *> map;

    SignalMapper() { map = std::vector<SignalHandler *>(static_cast<unsigned long>(SIGRTMAX), nullptr); }
} sig_map;

static std::mutex signal_lock;

static void signal_handler(int signal_number, siginfo_t *info, void *context) {
    SignalHandler *handler = sig_map.map[static_cast<std::size_t>(signal_number)];
    handler->handler(signal_number, info, static_cast<ucontext_t *>(context));
}

SignalHandler::SignalHandler(int signal_number, bool restart)
    : SIGNUM(signal_number), sigset_changed(false), revoke_on_destruction(true) {
    if (signal_number > SIGRTMAX || signal_number <= 0) throw std::runtime_error("invalid signal number");

    memset(&current_signal_action, 0, sizeof(current_signal_action));

    // set handler
#ifdef COMPILER_CLANG
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdisabled-macro-expansion"
#endif
    current_signal_action.sa_sigaction = signal_handler;
#ifdef COMPILER_CLANG
#    pragma GCC diagnostic pop
#endif

    // set flags
    current_signal_action.sa_flags = SA_SIGINFO;
    if (restart) current_signal_action.sa_flags |= SA_RESTART;

    // init block list
    int tmp = sigemptyset(&current_signal_action.sa_mask);
    if (tmp == -1) throw std::system_error(errno, std::generic_category(), "call of sigemptyset failed");
}

SignalHandler::~SignalHandler() {
    if (revoke_on_destruction) revoke();
}

void SignalHandler::establish() {
    // make changes to the signal configuration atomic
    std::lock_guard<std::mutex> guard(signal_lock);

    auto &sigmap_entry = sig_map.map[static_cast<std::size_t>(SIGNUM)];

    // check if there is already a signal handler registered
    if (sigmap_entry != nullptr) {
        // existing handler is this handler --> do nothing
        if (sigmap_entry == this) {
            if (sigset_changed) {
                int tmp = sigaction(SIGNUM, &current_signal_action, nullptr);
                if (tmp != 0) throw std::system_error(errno, std::generic_category(), "call of sigaction failed");
            }
            return;
        }

        struct sigaction tmp_action = sigmap_entry->old_signal_action;

        // establish this header
        int tmp = sigaction(SIGNUM, &current_signal_action, nullptr);
        if (tmp != 0) throw std::system_error(errno, std::generic_category(), "call of sigaction failed");

        // store this handler in signal handler map;
        // race condition possible: new signal handler is established, signal occurs, but new signal handler is not set
        //                          in map --> old signal handler is called. No need to fix this, because from the
        //                          applications perspective it does not matter. It is like the signal was issued a few
        //                          instructions earlier.
        sigmap_entry = this;

        old_signal_action = tmp_action;

        return;
    }

    // no handler established --> establish this one
    sigmap_entry = this;  // no handler established --> no race condition (this method is "atomic")

    int tmp = sigaction(SIGNUM, &current_signal_action, &old_signal_action);
    if (tmp != 0) throw std::system_error(errno, std::generic_category(), "call of sigaction failed");
}

void SignalHandler::revoke() {
    // make changes to the signal configuration atomic
    std::lock_guard<std::mutex> guard(signal_lock);

    auto &sigmap_entry = sig_map.map[static_cast<std::size_t>(SIGNUM)];

    if (sigmap_entry != this) return;

    int tmp = sigaction(SIGNUM, &old_signal_action, nullptr);
    if (tmp != 0) throw std::system_error(errno, std::generic_category(), "call of sigaction failed");

    sigmap_entry = nullptr;
}

[[maybe_unused]] void SignalHandler::block_signal(int signal_number) {
    if (signal_number > SIGRTMAX || signal_number <= 0) throw std::runtime_error("invalid signal number");

    int tmp = sigaddset(&current_signal_action.sa_mask, signal_number);
    if (tmp == -1) throw std::system_error(errno, std::generic_category(), "call of block_signal failed");
}

[[maybe_unused]] void SignalHandler::unblock_signal(int signal_number) {
    if (signal_number > SIGRTMAX || signal_number <= 0) throw std::runtime_error("invalid signal number");

    int tmp = sigdelset(&current_signal_action.sa_mask, signal_number);
    if (tmp == -1) throw std::system_error(errno, std::generic_category(), "call of sigdelset failed");
}

bool SignalHandler::is_established() const {
    // make changes to the signal configuration atomic
    std::lock_guard<std::mutex> guard(signal_lock);
    return sig_map.map[static_cast<std::size_t>(SIGNUM)] == this;
}

}  // namespace cxxsignal
