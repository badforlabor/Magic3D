#include "AppBase.h"

namespace MagicApp
{
    AppBase::AppBase(void)
    {
    }

    bool AppBase::Enter()
    {
        return true;
    }

    bool AppBase::Update(double timeElapsed)
    {
        return true;
    }

    bool AppBase::Exit()
    {
        return true;
    }

    bool AppBase::FrameStarted(const FrameEvent& evt)
    {
        return true;
    }

    bool AppBase::FrameEnded(const FrameEvent& evt)
    {
        return true;
    }

    bool AppBase::MouseMoved( const OIS::MouseEvent &arg )
    {
        return true;
    }

    bool AppBase::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        return true;
    }

    bool AppBase::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        return true;
    }

    bool AppBase::KeyPressed( const OIS::KeyEvent &arg )
    {
        return true;
    }

    bool AppBase::KeyReleased( const OIS::KeyEvent &arg )
    {
        return true;
    }

    void AppBase::WindowResized( Ogre::RenderWindow* rw )
    {

    }

    void AppBase::WindowFocusChanged( Ogre::RenderWindow* rw )
    {

    }
 
    AppBase::~AppBase(void)
    {
    }
}
