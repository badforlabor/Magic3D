#include "stdafx.h"
#include "ViewTool.h"
#include "RenderSystem.h"

namespace MagicCore
{
    ViewTool::ViewTool() : 
        mMouseCoordX(0),
        mMouseCoordY(0),
        mScale(1.0)
    {
    }

    ViewTool::ViewTool(double scale) : 
        mMouseCoordX(0),
        mMouseCoordY(0),
        mScale(scale)
    {
    }

    ViewTool::~ViewTool()
    {
    }

    void ViewTool::MousePressed(int mouseCoordX, int mouseCoordY)
    {
        mMouseCoordX = mouseCoordX;
        mMouseCoordY = mouseCoordY;
    }

    void ViewTool::MouseMoved(int mouseCoordX, int mouseCoordY, MouseMode mm)
    {
        if (mm == MM_MIDDLE_DOWN)
        {
            int mouseDiffX = mouseCoordX - mMouseCoordX;
            int mouseDiffY = mouseCoordY - mMouseCoordY;
            mMouseCoordX = mouseCoordX;
            mMouseCoordY = mouseCoordY;
            if (RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
            {
                RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->yaw(Ogre::Degree(mouseDiffX) * 0.2, Ogre::Node::TS_PARENT);
                RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->pitch(Ogre::Degree(mouseDiffY) * 0.2, Ogre::Node::TS_PARENT);
            }
            if (RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNodeLeft"))
            {
                RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNodeLeft")->yaw(Ogre::Degree(mouseDiffX) * 0.2, Ogre::Node::TS_PARENT);
                RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNodeLeft")->pitch(Ogre::Degree(mouseDiffY) * 0.2, Ogre::Node::TS_PARENT);
            }
            if (RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNodeRight"))
            {
                RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNodeRight")->yaw(Ogre::Degree(mouseDiffX) * 0.2, Ogre::Node::TS_PARENT);
                RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNodeRight")->pitch(Ogre::Degree(mouseDiffY) * 0.2, Ogre::Node::TS_PARENT);
            }
        }
        else if (mm == MM_LEFT_DOWN)
        {
            int mouseDiffY = mouseCoordY - mMouseCoordY;
            mMouseCoordX = mouseCoordX;
            mMouseCoordY = mouseCoordY;
            RenderSystem::Get()->GetMainCamera()->move(Ogre::Vector3(0, 0, mouseDiffY) * 0.007 * mScale);
        }
    }

    void ViewTool::SetScale(double scale)
    {
        mScale = scale;
    }
}
