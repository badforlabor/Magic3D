#pragma once
#include <string>

namespace Ogre
{
    class SceneManager;
    class RenderWindow;
}

namespace MyGUI
{
    class Gui;
    class OgrePlatform;
}

namespace MagicCore
{
    class GUISystem
    {
    private:
        GUISystem();
        static GUISystem* mpGUISystem;
    public:
        static GUISystem* Get();
        void Init(Ogre::RenderWindow* window, Ogre::SceneManager* sceneManager, std::string resourceName);
        ~GUISystem();

    private:
        MyGUI::Gui* mpGUI;
        MyGUI::OgrePlatform* mpPlatform;
    };
}
