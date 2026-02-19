#ifndef TIMER_H
#define TIMER_H
#include "./ThreadPool.h"
#ifdef ARDUINO
#include <Arduino.h>
typedef uint32_t TimePoint; // Use millis() on ESP32
#else
#include <chrono>
typedef std::chrono::steady_clock::time_point TimePoint; // Use steady_clock on Win32
#endif

namespace remoteLight
{
    class Timer
    {
    public:
        Timer(std::shared_ptr<ThreadPool> pool, FuncCallback func) : mPool(pool),
                                                                     mClear(false),
                                                                     mFunction(std::move(func)),
                                                                     mDuration(0U) {}
        /**
         * @brief Start a one-shot timer
         */
        void startTimer();

        /**
         * @brief Start a periodic timer
         */
        void startTimerInterval();

        /**
         * @brief Stop the timer
         */
        void stopTimer();

        /**
         * @brief Stop the periodic timer
         */
        void stopTimerInterval();

        /**
         * @brief Update the timer duration
         * @param func The callback function to be called on timer timeout
         * @param duration The new duration for the timer in milliseconds
         */
        void updateTimer(FuncCallback func, uint16_t duration);

        /**
         * @brief Get the current duration of the timer
         * @return The duration of the timer in milliseconds
         */
        uint16_t getDuration() const;

    private:
        std::shared_ptr<ThreadPool> mPool;
        std::atomic<bool> mClear;
        std::function<void()> mFunction;
        std::atomic<uint16_t> mDuration;
    };
} // namespace remoteLight

#endif // TIMER_H