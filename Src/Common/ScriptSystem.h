#pragma once

#include "lua.hpp"

namespace MagicCore
{
    class ScriptSystem
    {
    private:
        ScriptSystem();
        static ScriptSystem* mpScriptSystem;
    public:
        static ScriptSystem* Get();
        void Init();
        void Close();
        bool RunScript(const char* buff);
        bool RunScriptFile(const char* fileName);
        bool IsOnRunningScript();
        ~ScriptSystem();

    private:
        void Registrate();

    private:
        lua_State *mpLuaState;
        bool mIsOnRunning;
    };
}