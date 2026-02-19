#include "./Timer.h"
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <chrono>
#include <thread> // For sleep_for()
#endif

namespace remoteLight
{
    void Timer::startTimer()
    {
        mClear.store(false);

        uint16_t duration = mDuration.load();
        auto func = mFunction;

        mPool->enqueue([this, duration, func]
                       {
        delay(duration);
        if (mClear.load()) return;
        func(); });
    }

    void Timer::startTimerInterval()
    {
        mClear.store(false);
        uint16_t duration = mDuration.load();
        auto func = mFunction;

        mPool->enqueue([this, duration, func]
                       {
        while (!mClear.load()) {
            delay(duration);
            if (!mClear.load()) {
                func();
            }
        } });
    }

    void Timer::stopTimer()
    {
        mClear.store(true);
    }

    void Timer::stopTimerInterval()
    {
        mClear.store(true);
    }

    void Timer::updateTimer(FuncCallback func, uint16_t duration)
    {
        mFunction = std::move(func);
        mDuration.store(duration); // Ensure atomic update
    }

    uint16_t Timer::getDuration() const
    {
        return mDuration.load();
    }
} // namespace remoteLight