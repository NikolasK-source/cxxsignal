/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxsignal.hpp"
#include <cassert>
#include <iostream>
#include <unistd.h>

class TestHandler final : public cxxsignal::SignalHandler {
private:
    static std::size_t counter;
    std::size_t        single_counter = 0;

public:
    [[maybe_unused]] explicit TestHandler(int signal_number) : cxxsignal::SignalHandler(signal_number) {}

    void handler(int signal_number, siginfo_t *, ucontext_t *) override {
        std::cout << signal_number << std::endl;
        ++counter;
        ++single_counter;
    }

    [[nodiscard]] static inline std::size_t get_counter() { return counter; }
    [[nodiscard]] inline std::size_t        get_single_counter() const { return single_counter; }
};

std::size_t TestHandler::counter = 0;

static std::size_t alarm_counter = 0;
static void        dummy_alarm_handler(int) { ++alarm_counter; }

int main() {
    auto tmp = signal(SIGALRM, dummy_alarm_handler);
    assert(tmp != SIG_ERR);

    TestHandler        handler(SIGUSR1);
    TestHandler        handler2(SIGURG);
    cxxsignal::Default h2_default(SIGURG);
    cxxsignal::Ignore  ignore(SIGUSR1);
    handler.no_revoke();
    handler2.no_revoke();
    ignore.no_revoke();
    h2_default.no_revoke();

    handler.establish();
    handler2.establish();
    raise(SIGUSR1);
    raise(SIGUSR1);
    raise(SIGURG);
    raise(SIGURG);
    raise(SIGURG);
    raise(SIGUSR1);

    assert(handler.get_counter() == 6);
    assert(handler.is_established());
    assert(handler2.is_established());
    assert(!ignore.is_established());

    ignore.establish();
    raise(SIGUSR1);
    assert(handler.get_counter() == 6);
    assert(!handler.is_established());
    assert(handler2.is_established());
    assert(ignore.is_established());

    raise(SIGUSR1);
    raise(SIGURG);
    raise(SIGUSR1);
    raise(SIGURG);
    assert(handler.get_counter() == 8);
    assert(!handler.is_established());
    assert(handler2.is_established());
    assert(ignore.is_established());

    raise(SIGALRM);
    assert(alarm_counter == 1);

    {
        TestHandler alarm_handler(SIGALRM);
        alarm_handler.establish();

        assert(handler.get_counter() == 8);
        assert(alarm_handler.get_counter() == 8);
        std::cout << alarm_handler.get_single_counter() << std::endl;
        assert(!handler.is_established());
        assert(handler2.is_established());
        assert(ignore.is_established());
        assert(alarm_handler.is_established());

        raise(SIGALRM);
        raise(SIGALRM);
        assert(alarm_counter == 1);
        assert(alarm_handler.get_single_counter() == 2);
        assert(alarm_handler.get_counter() == 10);

        alarm_handler.revoke();
        raise(SIGALRM);
        assert(alarm_counter == 2);
        assert(alarm_handler.get_single_counter() == 2);
        assert(alarm_handler.get_counter() == 10);

        alarm_handler.establish();
        raise(SIGALRM);
        assert(alarm_counter == 2);
        assert(alarm_handler.get_single_counter() == 3);
        assert(alarm_handler.get_counter() == 11);
    }

    assert(alarm_counter == 2);
    raise(SIGALRM);
    assert(alarm_counter == 3);
    assert(handler.get_counter() == 11);

    h2_default.establish();
    raise(SIGURG);
    raise(SIGURG);
    assert(handler2.get_counter() == 11);
}
