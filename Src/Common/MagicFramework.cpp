#include "MagicFramework.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "ToolKit.h"
#include "ResourceManager.h"
#include "../Application/AppManager.h"
#include "GUISystem.h"
#include "LogSystem.h"

namespace MagicCore
{
    MagicFramework::MagicFramework() :
        mTimeAccumulate(0.0),
        mRenderDeltaTime(0.025)
    {
    }

    MagicFramework::~MagicFramework()
    {
    }

    void MagicFramework::Init()
    {
        InfoLog << "MagicFramework init" << std::endl;
        RenderSystem::Get()->Init();
        ResourceManager::Init();
        GUISystem::Get()->Init(RenderSystem::Get()->GetRenderWindow(), RenderSystem::Get()->GetSceneManager(), "MyGUIResource");
        InputSystem::Get()->Init(RenderSystem::Get()->GetRenderWindow());
        MagicApp::AppManager::Get()->Init();
    }

    void MagicFramework::Run()
    {
        InfoLog << "MagicFramework run" << std::endl;
        double timeLastFrame = ToolKit::GetTime();
        while (Running())
        {
            double timeCurrentFrame = ToolKit::GetTime();
            double timeSinceLastFrame = timeCurrentFrame - timeLastFrame;
            timeLastFrame = timeCurrentFrame;
            Update(timeSinceLastFrame);
        }
    }

    void MagicFramework::Update(double timeElapsed)
    {
        InputSystem::Get()->Update();
        MagicApp::AppManager::Get()->Update(timeElapsed);
        mTimeAccumulate += timeElapsed;
        if (mTimeAccumulate > mRenderDeltaTime)
        {
            mTimeAccumulate = mTimeAccumulate - mRenderDeltaTime;
            RenderSystem::Get()->Update();
        }
    }

    bool MagicFramework::Running(void)
    {
        return ToolKit::Get()->IsAppRunning();
    }
}
