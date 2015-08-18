#pragma once
#include "OIS.h"

namespace Ogre
{
    class RenderWindow;
}

namespace MagicCore
{
    class InputSystem
    {
    private:
        static InputSystem* mpInputSystem;
        InputSystem(void);
    public:
        static InputSystem* Get(void);
        void    Init(Ogre::RenderWindow* window);
        void    Update();
        void    UpdateMouseState(int w, int h);
        virtual ~InputSystem(void);
    private:
        OIS::InputManager* mpInputManager;
        OIS::Mouse*        mpMouse;
        OIS::Keyboard*     mpKeyboard;
    };
}
