#include "MagicFramework.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "ToolKit.h"
#include "ResourceManager.h"
#include "LicenseSystem.h"
#include "../Application/AppManager.h"
#include "GUISystem.h"
#include "LogSystem.h"
#include "DumpInfo.h"
#if DEBUGDUMPFILE
#include "DumpBase.h"
#endif
//#include "vld.h"

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
        LicenseSystem::Init();
        RenderSystem::Get()->Init();
        ResourceManager::Init();
        GUISystem::Get()->Init(RenderSystem::Get()->GetRenderWindow(), RenderSystem::Get()->GetSceneManager(), "MyGUIResource");
        InputSystem::Get()->Init(RenderSystem::Get()->GetRenderWindow());
        MagicApp::AppManager::Get()->Init();
        std::ifstream fin("magic3d.cfg");
        if (fin)
        {
            std::string str;
            double r, g, b;
            fin >> str >> r >> g >> b;
            RenderSystem::Get()->SetBackgroundColor(r, g, b);
            int fps;
            fin >> str >> fps; 
            mRenderDeltaTime = 1.0 / double(fps);
            fin.close();
        }
#if DEBUGDUMPFILE
        GPP::RegisterDumpInfo();
#endif
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
