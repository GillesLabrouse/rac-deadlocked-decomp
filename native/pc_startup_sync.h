#ifndef DEADLOCKED_PC_STARTUP_SYNC_H
#define DEADLOCKED_PC_STARTUP_SYNC_H

#include <cstdint>
#include <memory>

namespace deadlocked {

/* New PC platform services, not a PS2 kernel implementation.
 * At most 32 live semaphores are supported per registry. The caller owns
 * registry lifetime and must join its users before destroying the registry. */
class StartupSync {
public:
    enum class Result { success, invalid_handle, closed, exhausted, unavailable };
    StartupSync();
    ~StartupSync();
    StartupSync(const StartupSync&) = delete;
    StartupSync& operator=(const StartupSync&) = delete;

    /* Own a binary semaphore with initial count one and maximum count one.
     * On failure the output handle remains unchanged. Handles are never reused. */
    Result create(std::int32_t& handle) noexcept;
    Result wait(std::int32_t handle) noexcept;
    Result signal(std::int32_t handle) noexcept;
    /* Closing wakes blocked waits with Result::closed. A stale/foreign handle
     * fails. Queue order and cancellation are intentionally unspecified. */
    Result close(std::int32_t handle) noexcept;
    /* Stable, nonnegative process-local identity, distinct for each thread.
     * Failure returns -1. This is not a PS2 thread ID or host pointer. */
    static std::int32_t thread_identity() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif
