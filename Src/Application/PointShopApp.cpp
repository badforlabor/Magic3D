#include "stdafx.h"
#include "PointShopApp.h"
#include "PointShopAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "AppManager.h"
#include "MeshShopApp.h"
#include "GPP.h"
#include <algorithm>

namespace MagicApp
{
    PointShopApp::PointShopApp() :
        mpUI(NULL),
        mpPointCloud(NULL),
        mpViewTool(NULL),
        mpDumpInfo(NULL)
    {
    }

    PointShopApp::~PointShopApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpPointCloud);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpDumpInfo);
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
        if (arg.key == OIS::KC_D)
        {
            RunDumpInfo();
        }
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
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpPointCloud);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpDumpInfo);
    }

    bool PointShopApp::ImportPointCloud()
    {
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(2.0);
                GPPFREEPOINTER(mpPointCloud);
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
        char filterName[] = "Support format(*.obj, *.ply, *.asc)\0*.*\0";
        if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
        {
            GPP::Parser::ExportPointCloud(fileName, mpPointCloud);
        }
    }

    void PointShopApp::SmoothPointCloudNormal()
    {
        if (mpPointCloud == NULL || mpPointCloud->HasNormal() == false)
        {
            return;
        }
        //GPP::DumpOnce();
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::SmoothNormal(mpPointCloud, 1.0);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        UpdatePointCloudRendering();
    }

    void PointShopApp::SmoothPointCloudByNormal(void)
    {
        if (mpPointCloud == NULL || mpPointCloud->HasNormal() == false)
        {
            return;
        }
        //GPP::DumpOnce();
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::SmoothGeometryByNormal(mpPointCloud);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        UpdatePointCloudRendering();
    }

    void PointShopApp::RemovePointCloudOutlier(void)
    {
        if (mpPointCloud == NULL || mpPointCloud->HasNormal() == false)
        {
            return;
        }
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        if (pointCount < 1)
        {
            return;
        }
        GPP::Real* isolation = new GPP::Real[pointCount];
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateIsolation(mpPointCloud, isolation);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            GPPFREEARRAY(isolation);
            return;
        }

        GPP::Real cutValue = 0.05;
        std::vector<GPP::Int> deleteIndex;
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            if (isolation[pid] < cutValue)
            {
                deleteIndex.push_back(pid);
            }
        }
        res = DeletePointCloudElements(mpPointCloud, deleteIndex);
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            GPPFREEARRAY(isolation);
            return;
        }

        GPPFREEARRAY(isolation);
        UpdatePointCloudRendering();
    }

    void PointShopApp::SamplePointCloud(int targetPointCount)
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        GPP::Int* sampleIndex = new GPP::Int[targetPointCount];
        GPP::ErrorCode res = GPP::SamplePointCloud::UniformSample(mpPointCloud, targetPointCount, sampleIndex, 0);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            GPPFREEARRAY(sampleIndex);
            return;
        }
        GPP::PointCloud* samplePointCloud = new GPP::PointCloud;
        for (GPP::Int sid = 0; sid < targetPointCount; sid++)
        {
            samplePointCloud->InsertPoint(mpPointCloud->GetPointCoord(sampleIndex[sid]), mpPointCloud->GetPointNormal(sampleIndex[sid]));
            samplePointCloud->SetPointColor(sid, mpPointCloud->GetPointColor(sampleIndex[sid]));
        }
        samplePointCloud->SetHasNormal(mpPointCloud->HasNormal());
        delete mpPointCloud;
        mpPointCloud = samplePointCloud;
        UpdatePointCloudRendering();
        GPPFREEARRAY(sampleIndex);
    }

    void PointShopApp::CalculatePointCloudNormal()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloud, GPP::NORMAL_QUALITY_LOW);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
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
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        std::vector<GPP::Real> pointColorFields(pointCount * 3);
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            GPP::Vector3 color = mpPointCloud->GetPointColor(pid);
            GPP::Int baseId = pid * 3;
            pointColorFields.at(baseId) = color[0];
            pointColorFields.at(baseId + 1) = color[1];
            pointColorFields.at(baseId + 2) = color[2];
        }
        std::vector<GPP::Real> vertexColorField;
        GPP::TriMesh* triMesh = new GPP::TriMesh;
        GPP::ErrorCode res = GPP::PoissonReconstructMesh::Reconstruct(mpPointCloud, triMesh, GPP::RECONSTRUCT_QUALITY_MEDIUM, &pointColorFields, &vertexColorField);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            GPPFREEPOINTER(triMesh);
            return;
        }
        if (!triMesh)
        {
            return;
        }
        GPP::Int vertexCount = triMesh->GetVertexCount();
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            GPP::Int baseId = vid * 3;
            triMesh->SetVertexColor(vid, GPP::Vector3(vertexColorField.at(baseId), vertexColorField.at(baseId + 1), vertexColorField.at(baseId + 2)));
        }
        AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop)
        {
            triMesh->UpdateNormal();
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

    void PointShopApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        if (dumpInfo == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpDumpInfo);
        mpDumpInfo = dumpInfo;
        if (mpDumpInfo->GetPointCloud() == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpPointCloud);
        mpPointCloud = CopyPointCloud(mpDumpInfo->GetPointCloud());
        InitViewTool();
        UpdatePointCloudRendering();
    }

    void PointShopApp::RunDumpInfo()
    {
        if (mpDumpInfo == NULL)
        {
            return;
        }
        GPP::ErrorCode res = mpDumpInfo->Run();
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        if (mpDumpInfo->GetTriMesh() != NULL)
        {
            GPP::TriMesh* triMesh = CopyTriMesh(mpDumpInfo->GetTriMesh());         
            if (!triMesh)
            {
                return;
            }
            AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
            MeshShopApp* meshShop = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
            if (meshShop)
            {
                triMesh->UpdateNormal();
                meshShop->SetMesh(triMesh);
            }
            else
            {
                GPPFREEPOINTER(triMesh);
            }
        }
        else
        {
            GPPFREEPOINTER(mpPointCloud);
            mpPointCloud = CopyPointCloud(mpDumpInfo->GetPointCloud());
            UpdatePointCloudRendering();
            GPPFREEPOINTER(mpDumpInfo);
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
