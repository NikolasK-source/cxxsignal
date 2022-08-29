/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include "SignalHandler.hpp"

namespace cxxsignal {

/**
 * @brief signal handler class that represents the "ignore signal" signal handler
 */
class Ignore final : public SignalHandler {
public:
    /**
     * @brief create ignore handler
     * @param signal_number signal number of the signal this object shall handle
     */
    [[maybe_unused]] explicit Ignore(int signal_number);

    /**
     * @brief dummy signal handler
     * @details will never be called
     */
    void handler(int, siginfo_t *, ucontext_t *) override {}
};

}  // namespace cxxsignal
