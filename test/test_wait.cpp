/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxsignal.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <unistd.h>

class TestHandler final : public cxxsignal::SignalHandler {
private:
    static std::size_t counter;
    std::size_t        single_counter = 0;

public:
    [[maybe_unused]] explicit TestHandler(int signal_number) : cxxsignal::SignalHandler(signal_number) {}

    void handler(int signal_number, siginfo_t *, ucontext_t *) override {
        std::cout << "Signal: " << signal_number << std::endl;
        ++counter;
        ++single_counter;
    }

    [[nodiscard]] static inline std::size_t get_counter() { return counter; }
    [[nodiscard]] inline std::size_t        get_single_counter() const { return single_counter; }
};

std::size_t TestHandler::counter = 0;

int main() {
    TestHandler handler(SIGALRM);

    handler.establish();

    std::cerr << "1" << std::endl;
    alarm(1);
    handler.wait();
    assert(handler.get_single_counter() == 1);

    std::cerr << "2" << std::endl;
    alarm(4);
    auto no_timeout = handler.wait({1, 0});
    assert(!no_timeout);
    static_cast<void>(no_timeout);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    assert(handler.get_single_counter() == 2);

    cxxsignal::Ignore ignore(SIGALRM);
    ignore.establish();

    std::cerr << "3" << std::endl;
    alarm(1);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    assert(handler.get_single_counter() == 2);

    std::cerr << "4" << std::endl;
    alarm(1);
    no_timeout = handler.wait({2, 0});
    assert(no_timeout);
    static_cast<void>(no_timeout);
    assert(handler.get_single_counter() == 2);
}
