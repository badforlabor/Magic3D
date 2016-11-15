#pragma once
#include "AppBase.h"
#include <map>
#include <algorithm>
#include <string>

namespace MagicApp
{
    class AppManager
    {
    private:
        static AppManager* mpAppManager;
        AppManager(void);
    public:
        static AppManager* Get(void);

        void Init(void);
        void Update(double timeElapsed);
        void EnterApp(AppBase* pApp, std::string name);
        bool SwitchCurrentApp(std::string name);
        AppBase* GetApp(std::string name);
        AppBase* GetCurrentApp();

        bool FrameStarted(const FrameEvent& evt);
        bool FrameEnded(const FrameEvent& evt);
        bool MouseMoved(const OIS::MouseEvent &arg);
        bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        bool KeyPressed(const OIS::KeyEvent &arg);
        bool KeyReleased(const OIS::KeyEvent &arg);
        void WindowResized(Ogre::RenderWindow* rw);
        void WindowFocusChanged(Ogre::RenderWindow* rw);

        virtual ~AppManager(void);

    private:
        std::map<std::string, AppBase* > mAppSet;
        AppBase* mpCurrentApp;
    };
}
