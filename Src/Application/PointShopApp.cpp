#include "stdafx.h"
#include "PointShopApp.h"
#include "PointShopAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "AppManager.h"
#include "MeshShopApp.h"
#include "PointCloud.h"
#include "Parser.h"
#include "ErrorCodes.h"
#include "ConsolidatePointCloud.h"
#include "SamplePointCloud.h"
#include "PoissonReconstructMesh.h"

namespace MagicApp
{
    PointShopApp::PointShopApp() :
        mpUI(NULL),
        mpPointCloud(NULL),
        mpViewTool(NULL)
    {
    }

    PointShopApp::~PointShopApp()
    {
        if (mpUI != NULL)
        {
            delete mpUI;
            mpUI = NULL;
        }
        if (mpPointCloud != NULL)
        {
            delete mpPointCloud;
            mpPointCloud = NULL;
        }
        if (mpViewTool != NULL)
        {
            delete mpViewTool;
            mpViewTool = NULL;
        }
    }

    bool PointShopApp::Enter(void)
    {
        InfoLog << "Enter PointShopApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new PointShopAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool PointShopApp::Update(float timeElapsed)
    {
        return true;
    }

    bool PointShopApp::Exit(void)
    {
        InfoLog << "Exit PointShopApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool PointShopApp::MouseMoved( const OIS::MouseEvent &arg )
    {
        if (mpViewTool != NULL)
        {
            MagicCore::ViewTool::MouseMode mm;
            if (arg.state.buttonDown(OIS::MB_Left))
            {
                mm = MagicCore::ViewTool::MM_LEFT_DOWN;
            }
            else if (arg.state.buttonDown(OIS::MB_Middle))
            {
                mm = MagicCore::ViewTool::MM_MIDDLE_DOWN;
            }
            else if (arg.state.buttonDown(OIS::MB_Right))
            {
                mm = MagicCore::ViewTool::MM_RIGHT_DOWN;
            }
            else
            {
                mm = MagicCore::ViewTool::MM_NONE;
            }
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, mm);
        }
        
        return true;
    }

    bool PointShopApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool PointShopApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        return true;
    }

    bool PointShopApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        return true;
    }

    void PointShopApp::SetupScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("PointShop_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
    }

    void PointShopApp::ShutdownScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("PointShop_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloud_PointShop");
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        }
    }

    void PointShopApp::ClearData(void)
    {
        if (mpUI != NULL)
        {
            delete mpUI;
            mpUI = NULL;
        }
        if (mpPointCloud != NULL)
        {
            delete mpPointCloud;
            mpPointCloud = NULL;
        }
        if (mpViewTool != NULL)
        {
            delete mpViewTool;
            mpViewTool = NULL;
        }
    }

    bool PointShopApp::ImportPointCloud()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(2.0);
                if (mpPointCloud != NULL)
                {
                    delete mpPointCloud;
                }
                mpPointCloud = pointCloud;
                InfoLog << "Import Point Cloud: " << mpPointCloud->GetPointCount() << " points" << std::endl;
                InitViewTool();
                UpdatePointCloudRendering();
                return true;
            }
        }
        return false;
    }

    void PointShopApp::ExportPointCloud()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        std::string fileName;
        char filterName[] = "Support format(*.obj, *.ply)\0*.*\0";
        if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
        {
            GPP::Parser::ExportPointCloud(fileName, mpPointCloud);
        }
    }

    void PointShopApp::SmoothPointCloud()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        GPP::Int res = GPP::ConsolidatePointCloud::LaplaceSmooth(mpPointCloud, 0.2, 5);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        UpdatePointCloudRendering();
    }

    void PointShopApp::SamplePointCloud(int targetPointCount)
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        int* sampleIndex = new int[targetPointCount];
        GPP::Int res = GPP::SamplePointCloud::UniformSample(mpPointCloud, targetPointCount, sampleIndex);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            if (sampleIndex != NULL)
            {
                delete []sampleIndex;
                sampleIndex = NULL;
            }
            return;
        }
        GPP::PointCloud* samplePointCloud = new GPP::PointCloud;
        for (int sid = 0; sid < targetPointCount; sid++)
        {
            samplePointCloud->InsertPoint(mpPointCloud->GetPointCoord(sampleIndex[sid]), mpPointCloud->GetPointNormal(sampleIndex[sid]));
        }
        samplePointCloud->SetHasNormal(mpPointCloud->HasNormal());
        delete mpPointCloud;
        mpPointCloud = samplePointCloud;
        UpdatePointCloudRendering();
        if (sampleIndex != NULL)
        {
            delete []sampleIndex;
            sampleIndex = NULL;
        }
    }

    void PointShopApp::CalculatePointCloudNormal()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        GPP::Int res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloud, 0);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        mpPointCloud->SetHasNormal(true);
        UpdatePointCloudRendering();
    }

    void PointShopApp::FlipPointCloudNormal()
    {
        if (mpPointCloud == NULL || mpPointCloud->HasNormal() == false)
        {
            return;
        }
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloud->SetPointNormal(pid, mpPointCloud->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudRendering();
    }

    void PointShopApp::ReconstructMesh()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        GPP::PoissonReconstructMesh reconstructTool;
        GPP::Int res = reconstructTool.Init(mpPointCloud);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        GPP::TriMesh* triMesh = reconstructTool.ReconstructMesh();
        if (!triMesh)
        {
            return;
        }
        AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop)
        {
            meshShop->SetMesh(triMesh);
        }
        else
        {
            delete triMesh;
            triMesh = NULL;
        }
    }

    void PointShopApp::SetPointCloud(GPP::PointCloud* pointCloud)
    {
        if (!mpPointCloud)
        {
            delete mpPointCloud;
        }
        mpPointCloud = pointCloud;
        InitViewTool();
        UpdatePointCloudRendering();
    }

    int PointShopApp::GetPointCount()
    {
        if (mpPointCloud != NULL)
        {
            return mpPointCloud->GetPointCount();
        }
        else
        {
            return 0;
        }
    }

    void PointShopApp::UpdatePointCloudRendering()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        if (mpPointCloud->HasNormal())
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloud_PointShop", "CookTorrancePoint", mpPointCloud);
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloud_PointShop", "SimplePoint", mpPointCloud);
        }
    }

    void PointShopApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }
}
