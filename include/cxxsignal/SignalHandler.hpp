/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include <csignal>

namespace cxxsignal {

/**
 * @brief Abstract class that encapsulates a signal handler
 */
class SignalHandler {
private:
    //* Signal action of this handler
    struct sigaction current_signal_action {};

    /**
     * @brief Previous signal action
     *
     * @details
     * required because handler is restored by call of revoke()
     */
    struct sigaction old_signal_action {};

    //* signal number
    const int SIGNUM;

    //* flag that indicates that the list of blocked signals was changed
    bool sigset_changed;

    //* flag that indicates that the handler should be revoked on destruction (default: true)
    bool revoke_on_destruction;

protected:
    /**
     * @brief create new signal handler object
     * @param signal_number signal number of the signal this object shall handle
     * @param restart make some system calls restartable across signals. See man sigaction for details (SA_RESTART).
     *
     * @exception std::system_error failed to perform action on sigset_t
     */
    explicit SignalHandler(int signal_number, bool restart = true);

public:
    /**
     * @brief destroy signal handler object
     *
     * @details
     * revokes the handler if it is established
     */
    virtual ~SignalHandler();

    /**
     * @brief the signal handling function
     *
     * @param signal_number The number of the signal that caused invocation of the handler.
     * @param info a structure containing further information about the signal. See man sigaction for details.
     * @param context The structure pointed to by this field contains signal context information that was saved on the
     *                user-space stack by the kernel. See man sigaction, man sigreturn and man getcontext for details.
     */
    virtual void handler(int signal_number, siginfo_t *info, ucontext_t *context) = 0;

    /**
     * @brief establish the signal handler
     *
     * @exception std::system_error thrown if call of sigaction failed
     */
    virtual void establish() final;

    /**
     * @brief revoke the signal handler
     *
     * @details
     * restores the signal handler that was active before the first signal handler was established using a SignalHandler
     * object. This must not be the signal handler that was active when the application started.
     *
     * @exception std::system_error thrown if call of sigaction failed
     */
    virtual void revoke() final;

    /**
     * @brief add a signal to the list of blocked signals
     *
     * @details
     * establish must be called again to apply changes to an already established signal handler
     *
     * @param signal_number number of the blocked signal
     *
     * @exception std::system_error failed to perform action on sigset_t (should never happen)
     */
    [[maybe_unused]] virtual void block_signal(int signal_number) final;

    /**
     * @brief remove a signal from the list of blocked signals
     *
     * @details
     * establish must be called again to apply changes to an already established signal handler
     *
     * @param signal_number number of the unblocked signal
     *
     * @exception std::system_error failed to perform action on sigset_t (should never happen)
     */
    [[maybe_unused]] virtual void unblock_signal(int signal_number) final;

    /**
     * @brief check if the handler is currently established
     * @return true handler established
     * @return false handler not established
     */
    [[nodiscard]] virtual bool is_established() const final;

    /**
     * @brief disable revocation of the handler on destruction
     *
     * @details
     * Can be enabled on handlers that persist until the application is terminated
     * (created in main function or as global objects).
     *     --> No unnecessary calls of sigaction when the program is terminated.
     * !!! Must not be enabled on handler objects that are created in another scope! !!!
     *
     * @param no_revoke enable/disable norevoke mode
     */
    [[maybe_unused]] inline void no_revoke(bool no_revoke = true) { revoke_on_destruction = !no_revoke; }

    /**
     * @brief wait for the signal
     * @details if the handler is established, it is called once the signal arrives
     * @param timeout timeout (default: no timeout)
     * @exception std::system_error failed to perform action on sigset_t (should never happen)
     * @exception std::system_error call of sigtimedwait or sigwaitinfo failed
     * @exception std::system_error call of sigprocmask failed
     */
    bool wait(const struct timespec &timeout = {0, 0});

    friend class Ignore;
    friend class Default;
};

}  // namespace cxxsignal
