#include "GUISystem.h"
#include "MyGUI.h"
#include "MyGUI_OgrePlatform.h"

namespace MagicCore
{
    GUISystem* GUISystem::mpGUISystem = NULL;

    GUISystem::GUISystem() :
        mpGUI(NULL),
        mpPlatform(NULL)
    {
    }

    GUISystem* GUISystem::Get()
    {
        if (mpGUISystem == NULL)
        {
            mpGUISystem = new GUISystem;
        }
        return mpGUISystem;
    }

    GUISystem::~GUISystem()
    {
    }

    void GUISystem::Init(Ogre::RenderWindow* window, Ogre::SceneManager* sceneManager, std::string resourceName)
    {
        mpPlatform = new MyGUI::OgrePlatform();
        mpPlatform->initialise(window, sceneManager, resourceName);
        mpGUI = new MyGUI::Gui();
        mpGUI->initialise();
        MyGUI::PointerManager::getInstance().setVisible(false);
    }
}
