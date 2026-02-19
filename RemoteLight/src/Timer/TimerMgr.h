#ifndef TIMER_MGR_H
#define TIMER_MGR_H

#include "./Timer.h"
#include "./ThreadPool.h"

namespace remoteLight
{
    class TimerManager
    {
    public:
        TimerManager(size_t numThreads) : pool(std::make_shared<ThreadPool>(numThreads)) {}

        /**
         * @brief Create a new timer with the specified callback function
         * @param func The callback function to be called on timer timeout
         * @return A shared pointer to the created Timer
         */
        std::shared_ptr<Timer> createTimer(FuncCallback func);

    private:
        std::shared_ptr<ThreadPool> pool;
    };
} // namespace remoteLight

#endif // TIMER_MGR_H