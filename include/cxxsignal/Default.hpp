/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include "SignalHandler.hpp"

namespace cxxsignal {

/**
 * @brief signal handler class that represents the default signal handler
 *
 * @details the default behaviour depends on the signal number. See man 7 signal for default signal actions.
 */
class Default final : public SignalHandler {
public:
    /**
     * @brief create default handler
     * @param signal_number signal number of the signal this object shall handle
     */
    [[maybe_unused]] explicit Default(int signal_number);

    /**
     * @brief dummy signal handler
     * @details will never be called
     */
    void handler(int, siginfo_t *, ucontext_t *) override {}
};

}  // namespace cxxsignal
