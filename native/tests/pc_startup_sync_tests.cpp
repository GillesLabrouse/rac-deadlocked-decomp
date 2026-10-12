#include "pc_startup_sync.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <future>
#include <set>
#include <stdexcept>
#include <thread>
#include <vector>

using deadlocked::StartupSync;
using R = StartupSync::Result;
using namespace std::chrono_literals;
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (false)

int main() {
    try {
        StartupSync registry, foreign;
        std::int32_t handle = -99, other = -98;
        CHECK(registry.create(handle) == R::success);
        CHECK(foreign.create(other) == R::success && handle != other);
        CHECK(registry.wait(other) == R::invalid_handle);
        CHECK(foreign.signal(handle) == R::invalid_handle);
        CHECK(registry.signal(handle) == R::exhausted);
        CHECK(registry.wait(handle) == R::success);
        std::promise<void> started;
        auto ready = started.get_future();
        auto waiting = std::async(std::launch::async, [&] {
            started.set_value();
            return registry.wait(handle);
        });
        CHECK(ready.wait_for(2s) == std::future_status::ready);
        CHECK(waiting.wait_for(100ms) == std::future_status::timeout);
        CHECK(registry.signal(handle) == R::success);
        CHECK(waiting.wait_for(2s) == std::future_status::ready);
        CHECK(waiting.get() == R::success);
        CHECK(registry.signal(handle) == R::success);

        std::atomic<int> in_section{0}, violations{0};
        std::atomic<int> completed{0};
        int counter = 0;
        std::vector<std::thread> threads;
        std::vector<std::int32_t> identities(8, -1);
        for (int i = 0; i < 8; ++i) threads.emplace_back([&, i] {
            identities[static_cast<std::size_t>(i)] = StartupSync::thread_identity();
            if (identities[static_cast<std::size_t>(i)] < 0 ||
                identities[static_cast<std::size_t>(i)] != StartupSync::thread_identity())
                ++violations;
            for (int n = 0; n < 1000; ++n) {
                if (registry.wait(handle) != R::success) { ++violations; return; }
                if (in_section.fetch_add(1) != 0) ++violations;
                const int old = counter;
                std::this_thread::yield();
                counter = old + 1;
                if (in_section.fetch_sub(1) != 1) ++violations;
                if (registry.signal(handle) != R::success) { ++violations; return; }
            }
            ++completed;
        });
        for (auto& thread : threads) thread.join();
        CHECK(counter == 8000 && completed == 8 && violations == 0);
        CHECK(std::set<std::int32_t>(identities.begin(), identities.end()).size() == 8);
        const auto main_identity = StartupSync::thread_identity();
        CHECK(main_identity >= 0 && main_identity == StartupSync::thread_identity());
        CHECK(std::set<std::int32_t>(identities.begin(), identities.end()).count(main_identity) == 0);

        CHECK(registry.wait(handle) == R::success);
        std::promise<void> closing_started;
        auto closing_ready = closing_started.get_future();
        auto closing_wait = std::async(std::launch::async, [&] {
            closing_started.set_value();
            return registry.wait(handle);
        });
        CHECK(closing_ready.wait_for(2s) == std::future_status::ready);
        CHECK(closing_wait.wait_for(100ms) == std::future_status::timeout);
        CHECK(registry.close(handle) == R::success);
        CHECK(closing_wait.wait_for(2s) == std::future_status::ready);
        const auto closing_result = closing_wait.get();
        CHECK(closing_result == R::closed || closing_result == R::invalid_handle);
        CHECK(registry.close(handle) == R::invalid_handle);
        CHECK(registry.wait(handle) == R::invalid_handle);
        CHECK(registry.signal(handle) == R::invalid_handle);

        std::vector<std::int32_t> live;
        for (int n = 0; n < 32; ++n) {
            std::int32_t fresh = -5;
            CHECK(registry.create(fresh) == R::success && fresh != handle && fresh != other);
            live.push_back(fresh);
        }
        CHECK(std::set<std::int32_t>(live.begin(), live.end()).size() == 32);
        std::int32_t unchanged = -123;
        CHECK(registry.create(unchanged) == R::exhausted && unchanged == -123);
        for (const auto item : live) CHECK(registry.close(item) == R::success);
        CHECK(registry.create(unchanged) == R::success && unchanged != handle);
        CHECK(registry.close(unchanged) == R::success);
        CHECK(foreign.close(other) == R::success);
        std::puts("PASS: 8000 contended acquisitions, stable thread identities, close wakeup, stale/foreign handles, capacity refusal.");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
