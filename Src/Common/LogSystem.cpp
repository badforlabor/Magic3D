#include "LogSystem.h"

namespace MagicCore
{
    const LogLevel gSystemLogLevel = LOGLEVEL_DEBUG;
    LogSystem* LogSystem::mpLogSystem = NULL;

    LogSystem::LogSystem(void)
        : mOFStream("Log_Magic3D.txt")
    {
    }

    LogSystem* LogSystem::Get()
    {
        if (mpLogSystem == NULL)
        {
            mpLogSystem = new LogSystem;
        }
        return mpLogSystem;
    }

    LogSystem::~LogSystem(void)
    {
    }

    std::ofstream& LogSystem::GetOFStream()
    {
        return mOFStream;
    }
}
