#include "pc_startup_sync.h"
#include <array>
#include <atomic>
#include <condition_variable>
#include <limits>
#include <mutex>

namespace deadlocked {
namespace {
std::atomic<std::uint32_t> next_handle{0};
std::atomic<std::uint32_t> next_thread{0};

std::int32_t reserve_identity(std::atomic<std::uint32_t>& source) noexcept {
    auto value = source.load(std::memory_order_relaxed);
    const auto maximum = static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max());
    while (value <= maximum) {
        if (source.compare_exchange_weak(value, value + 1, std::memory_order_relaxed))
            return static_cast<std::int32_t>(value);
    }
    return -1;
}

struct Semaphore {
    std::mutex mutex;
    std::condition_variable changed;
    bool available = true;
    bool closed = false;
};
}

struct StartupSync::Impl {
    struct Slot { std::int32_t handle = -1; std::shared_ptr<Semaphore> resource; };
    std::mutex mutex;
    std::array<Slot, 32> slots;

    std::shared_ptr<Semaphore> find(std::int32_t handle) {
        std::lock_guard<std::mutex> guard(mutex);
        for (const auto& slot : slots)
            if (slot.resource && slot.handle == handle) return slot.resource;
        return {};
    }
};

StartupSync::StartupSync() : impl_(std::make_unique<Impl>()) {}
StartupSync::~StartupSync() = default;

StartupSync::Result StartupSync::create(std::int32_t& handle) noexcept {
    try {
        std::lock_guard<std::mutex> guard(impl_->mutex);
        for (auto& slot : impl_->slots) {
            if (slot.resource) continue;
            auto resource = std::make_shared<Semaphore>();
            const auto identity = reserve_identity(next_handle);
            if (identity < 0) return Result::exhausted;
            slot.handle = identity;
            slot.resource = std::move(resource);
            handle = identity;
            return Result::success;
        }
        return Result::exhausted;
    } catch (...) { return Result::unavailable; }
}

StartupSync::Result StartupSync::wait(std::int32_t handle) noexcept {
    try {
        auto resource = impl_->find(handle);
        if (!resource) return Result::invalid_handle;
        std::unique_lock<std::mutex> guard(resource->mutex);
        resource->changed.wait(guard, [&] { return resource->available || resource->closed; });
        if (resource->closed) return Result::closed;
        resource->available = false;
        return Result::success;
    } catch (...) { return Result::unavailable; }
}

StartupSync::Result StartupSync::signal(std::int32_t handle) noexcept {
    try {
        auto resource = impl_->find(handle);
        if (!resource) return Result::invalid_handle;
        std::lock_guard<std::mutex> guard(resource->mutex);
        if (resource->closed) return Result::closed;
        if (resource->available) return Result::exhausted;
        resource->available = true;
        resource->changed.notify_one();
        return Result::success;
    } catch (...) { return Result::unavailable; }
}

StartupSync::Result StartupSync::close(std::int32_t handle) noexcept {
    try {
        std::lock_guard<std::mutex> registry_guard(impl_->mutex);
        for (auto& slot : impl_->slots) {
            if (!slot.resource || slot.handle != handle) continue;
            auto resource = slot.resource;
            std::lock_guard<std::mutex> resource_guard(resource->mutex);
            slot.resource->closed = true;
            slot.resource->changed.notify_all();
            slot.resource.reset();
            slot.handle = -1;
            return Result::success;
        }
        return Result::invalid_handle;
    } catch (...) { return Result::unavailable; }
}

std::int32_t StartupSync::thread_identity() noexcept {
    thread_local const auto identity = reserve_identity(next_thread);
    return identity;
}
}
