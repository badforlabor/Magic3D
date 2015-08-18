#include "stdafx.h"
#include "MagicListener.h"
#include "../Application/AppManager.h"
#include "MyGUI.h"
#include "ToolKit.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "GUISystem.h"

namespace MagicCore
{
    MagicListener* MagicListener::mpListener = NULL;

    MagicListener::MagicListener()
    {
    }

    MagicListener* MagicListener::Get()
    {
        if (mpListener == NULL)
        {
            mpListener = new MagicListener;
        }
        return mpListener;
    }

    bool MagicListener::frameStarted(const Ogre::FrameEvent& evt)
    {
        MagicApp::FrameEvent fe = {evt.timeSinceLastEvent, evt.timeSinceLastFrame};
        return MagicApp::AppManager::Get()->FrameStarted(fe);
    }

    bool MagicListener::frameEnded(const Ogre::FrameEvent& evt)
    {
        MagicApp::FrameEvent fe = {evt.timeSinceLastEvent, evt.timeSinceLastFrame};
        return MagicApp::AppManager::Get()->FrameEnded(fe);
    }

    bool MagicListener::mouseMoved( const OIS::MouseEvent &arg )
    {
        ToolKit::Get()->SetMousePressLocked(false);
        MyGUI::InputManager::getInstance().injectMouseMove(arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs);
        return MagicApp::AppManager::Get()->MouseMoved(arg);
        
    }

    bool MagicListener::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (ToolKit::Get()->IsMousePressLocked())
        {
            return true;
        }
        else
        {
            MyGUI::InputManager::getInstance().injectMousePress(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
            return MagicApp::AppManager::Get()->MousePressed(arg, id);
        }
    }

    bool MagicListener::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (ToolKit::Get()->IsMousePressLocked())
        {
            return true;
        }
        else
        {
            MyGUI::InputManager::getInstance().injectMouseRelease(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
            return MagicApp::AppManager::Get()->MouseReleased(arg, id);
        }
    }

    bool MagicListener::keyPressed( const OIS::KeyEvent &arg )
    {
        MyGUI::KeyCode code = MyGUI::KeyCode::Enum(arg.key);
        MyGUI::InputManager::getInstance().injectKeyPress(code, arg.text);
        return MagicApp::AppManager::Get()->KeyPressed(arg);
    }

    bool MagicListener::keyReleased( const OIS::KeyEvent &arg )
    {
        MyGUI::KeyCode code = MyGUI::KeyCode::Enum(arg.key);
        MyGUI::InputManager::getInstance().injectKeyRelease(code);
        return MagicApp::AppManager::Get()->KeyReleased(arg);
    }

    void MagicListener::windowResized(Ogre::RenderWindow* rw)
    {
        RenderSystem::Get()->GetMainCamera()->setAspectRatio((Ogre::Real)rw->getWidth() / (Ogre::Real)rw->getHeight());
        InputSystem::Get()->UpdateMouseState(rw->getWidth(), rw->getHeight());
        MagicApp::AppManager::Get()->WindowResized(rw);
    }

    bool MagicListener::windowClosing(Ogre::RenderWindow* rw)
    {
        ToolKit::Get()->SetAppRunning(false);
        return true;
    }

    void MagicListener::windowFocusChange(Ogre::RenderWindow* rw)
    {
        ToolKit::Get()->SetMousePressLocked(true);
    }

    MagicListener::~MagicListener()
    {
    }
}
