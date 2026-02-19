#include "./TimerMgr.h"

namespace remoteLight
{
    std::shared_ptr<Timer> TimerManager::createTimer(FuncCallback func)
    {
        return std::make_shared<Timer>(pool, func);
    }
}