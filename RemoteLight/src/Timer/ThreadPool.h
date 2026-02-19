#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace remoteLight
{
    using FuncCallback = std::function<void()>;

    class ThreadPool
    {
    public:
        ThreadPool(size_t numThreads);
        ~ThreadPool();

        /**
         * @brief Enqueue a new task to be executed by the thread pool
         * @param task The task function to be executed
         */
        void enqueue(std::function<void()> task);

    private:
        std::vector<std::thread> workers;
        std::queue<FuncCallback> tasks;

        std::mutex queueMutex;
        std::condition_variable condition;
        std::atomic<bool> stop;
    };
} // namespace remoteLight
#endif // THREAD_POOL_H