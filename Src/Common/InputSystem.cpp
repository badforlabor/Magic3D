#include "InputSystem.h"
#include "LogSystem.h"
#include "MagicListener.h"

namespace MagicCore
{
    InputSystem* InputSystem::mpInputSystem = NULL;

    InputSystem::InputSystem(void) :
        mpInputManager(NULL),
        mpMouse(NULL),
        mpKeyboard(NULL)
    {
    }

    InputSystem* InputSystem::Get()
    {
        if (mpInputSystem == NULL)
        {
            mpInputSystem = new InputSystem;
        }
        return mpInputSystem;
    }

    void InputSystem::Init(Ogre::RenderWindow* window)
    {
        InfoLog << "InputSystem init....";
        // OIS setup
        OIS::ParamList paramList;
        size_t windowHnd = 0;
        std::ostringstream windowHndStr;

        // get window handle
        window->getCustomAttribute("WINDOW", &windowHnd);

        // fill param list
        windowHndStr << (unsigned int)windowHnd;
        paramList.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
        paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
        paramList.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
        paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
        paramList.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));

        // create input system
        mpInputManager = OIS::InputManager::createInputSystem(paramList);

        mpKeyboard = static_cast<OIS::Keyboard*>(mpInputManager->createInputObject(OIS::OISKeyboard, true));
        mpKeyboard->setEventCallback(MagicListener::Get());
        mpKeyboard->setBuffered(true);

        mpMouse = static_cast<OIS::Mouse*>(mpInputManager->createInputObject(OIS::OISMouse, true));
        mpMouse->setEventCallback(MagicListener::Get());
        mpMouse->setBuffered(true);

        unsigned int width, height, depth;
        int left, top;

        window->getMetrics(width, height, depth, left, top);
        const OIS::MouseState& mouseState = mpMouse->getMouseState();
        mouseState.width = width;
        mouseState.height = height;
        InfoLog << "OK" << std::endl;
    }

    void InputSystem::Update()
    {
        mpMouse->capture();
        mpKeyboard->capture();
        Ogre::WindowEventUtilities::messagePump();
    }

    void InputSystem::UpdateMouseState(int w, int h)
    {
        mpMouse->getMouseState().width  = w;
        mpMouse->getMouseState().height = h;
    }

    InputSystem::~InputSystem(void)
    {
    }
}
