#include "stdafx.h"
#include "AppApi.h"
#include "ModelManager.h"
#include "MagicMesh.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "AppManager.h"
#include "PointShopApp.h"
#include "MeshShopApp.h"
#include "MeasureApp.h"
#include "DepthVideoApp.h"
#include "Homepage.h"
#include "RegistrationApp.h"
#include "ReliefApp.h"
#include "TextureApp.h"
#include "AnimationApp.h"
#include "UVUnfoldApp.h"
#include "opencv2/opencv.hpp"
#include <cstring>

namespace MagicApp
{

    bool AppApi::EnterApp(const char* appName)
    {
        std::string appString(appName);
        if (appString == "PointShopApp")
        {
            AppManager::Get()->EnterApp(new PointShopApp, appString);
        }
        else if (appString == "MeshShopApp")
        {
            AppManager::Get()->EnterApp(new MeshShopApp, appString);
        }
        else if (appString == "MeasureApp")
        {
            AppManager::Get()->EnterApp(new MeasureApp, appString);
        }
        else if (appString == "RegistrationApp")
        {
            AppManager::Get()->EnterApp(new RegistrationApp, appString);
        }
        else if (appString == "ReliefApp")
        {
            AppManager::Get()->EnterApp(new ReliefApp, appString);
        }
        else if (appString == "UVUnfoldApp")
        {
            AppManager::Get()->EnterApp(new UVUnfoldApp, appString);
        }
        else if (appString == "Homepage")
        {
            AppManager::Get()->EnterApp(new Homepage, appString);
        }
        else if (appString == "DepthVideoApp")
        {
            AppManager::Get()->EnterApp(new DepthVideoApp, appString);
        }
        else if (appString == "AnimationApp")
        {
            AppManager::Get()->EnterApp(new AnimationApp, appString);
        }
        else
        {
            return false;
        }
        return true;
    }

    MagicApp::MeshShopApp* AppApi::GetMeshShopApp()
    {
        AppBase* pApp = MagicApp::AppManager::Get()->GetCurrentApp();
        MagicApp::MeshShopApp* pShopApp = dynamic_cast<MagicApp::MeshShopApp*>(pApp);
        return pShopApp;
    }

    MagicApp::ReliefApp* AppApi::GetReliefApp()
    {
        AppBase* pApp = MagicApp::AppManager::Get()->GetCurrentApp();
        MagicApp::ReliefApp* pCastApp = dynamic_cast<MagicApp::ReliefApp*>(pApp);
        return pCastApp;
    }

    GPP::TriMesh* AppApi::GetMesh()
    {
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        return triMesh;
    }

}