#include "stdafx.h"
#include "MeshShopApp.h"
#include "MeshShopAppUI.h"
#include "PointShopApp.h"
#include "AppManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/RenderSystem.h"
#include "Mesh.h"
#include "PointCloud.h"
#include "Parser.h"
#include "ErrorCodes.h"
#include "ConsolidateMesh.h"
#include "SubdivideMesh.h"
#include "SimplifyMesh.h"
#include "ParameterizeMesh.h"
#include "ToolMesh.h"
#include "ToolAnn.h"
#include "ToolLog.h"
#include "SparseMatrix.h"
#include "ToolMatrix.h"
#include "DumpInfo.h"

namespace MagicApp
{
    MeshShopApp::MeshShopApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mpViewTool(NULL),
        mpDumpInfo(NULL)
    {
    }

    MeshShopApp::~MeshShopApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpDumpInfo);
    }

    bool MeshShopApp::Enter()
    {
        InfoLog << "Enter MeshShopApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new MeshShopAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool MeshShopApp::Update(float timeElapsed)
    {
        return true;
    }

    bool MeshShopApp::Exit()
    {
        InfoLog << "Exit MeshShopApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool MeshShopApp::MouseMoved( const OIS::MouseEvent &arg )
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

    bool MeshShopApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool MeshShopApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        return  true;
    }

    bool MeshShopApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_V && mpTriMesh !=NULL)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_POINTS);
        }
        else if (arg.key == OIS::KC_E && mpTriMesh !=NULL)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
        }
        else if (arg.key == OIS::KC_F && mpTriMesh !=NULL)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
        }
        else if (arg.key == OIS::KC_D)
        {
            RunDumpInfo();
        }
        return true;
    }

    void MeshShopApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("MeshShop_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
    }

    void MeshShopApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("MeshShop_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_MeshShop");
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        } 
    }

    void MeshShopApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
    }

    bool MeshShopApp::ImportMesh()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::TriMesh* triMesh = GPP::Parser::ImportTriMesh(fileName);
            if (triMesh != NULL)
            { 
                triMesh->UnifyCoords(2.0);
                triMesh->UpdateNormal();
                if (mpTriMesh != NULL)
                {
                    delete mpTriMesh;
                }
                mpTriMesh = triMesh;
                InfoLog << "Import Mesh,  vertex: " << mpTriMesh->GetVertexCount() << " triangles: " << triMesh->GetTriangleCount() << std::endl;
                InitViewTool();
                UpdateMeshRendering();
                return true;
            }
        }
        return false;
    }

    void MeshShopApp::ExportMesh()
    {
        if (mpTriMesh != NULL)
        {
            std::string fileName;
            char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0Color Files(*.cps)\0*.cps\0";
            if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
            {
                GPP::Parser::ExportTriMesh(fileName, mpTriMesh);
            }
        }
    }

    void MeshShopApp::SetMesh(GPP::TriMesh* triMesh)
    {
        if (!mpTriMesh)
        {
            delete mpTriMesh;
        }
        mpTriMesh = triMesh;
        InitViewTool();
        UpdateMeshRendering();
    }

    void MeshShopApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        if (dumpInfo == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpDumpInfo);
        mpDumpInfo = dumpInfo;
        if (mpDumpInfo->GetTriMesh() == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = CopyTriMesh(mpDumpInfo->GetTriMesh());
        mpTriMesh->UpdateNormal();
        InitViewTool();
        UpdateMeshRendering();
    }

    void MeshShopApp::RunDumpInfo()
    {
        if (mpDumpInfo == NULL)
        {
            return;
        }
        GPP::Int res = mpDumpInfo->Run();
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        //Copy result
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = CopyTriMesh(mpDumpInfo->GetTriMesh());
        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
        GPPFREEPOINTER(mpDumpInfo);
        
    }

    void MeshShopApp::ConsolidateTopology()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::Int res = GPP::ConsolidateMesh::MakeTriMeshManifold(mpTriMesh);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::ReverseDirection()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        GPP::Int faceCount = mpTriMesh->GetTriangleCount();
        GPP::Int vertexIds[3];
        for (GPP::Int fid = 0; fid < faceCount; fid++)
        {
            mpTriMesh->GetTriangleVertexIds(fid, vertexIds);
            mpTriMesh->SetTriangleVertexIds(fid, vertexIds[1], vertexIds[0], vertexIds[2]);
        }
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::ConsolidateGeometry()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        int maxItrCount = 1;
        for (int itr = 0; itr < maxItrCount; itr++)
        {
            int singlarVertexModified = 0;
            GPP::Int res = GPP::ConsolidateMesh::ConsolidateGeometry(mpTriMesh, 0.0174532925199433 * 5.0, GPP::REAL_TOL, &singlarVertexModified);
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
            if (singlarVertexModified == 0)
            {
                break;
            }
        }
        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::SmoothMesh()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::Int res = GPP::ConsolidateMesh::LaplaceSmooth(mpTriMesh, 0.2, 5, true);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::LoopSubdivide()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::Int res = GPP::SubdivideMesh::LoopSubdivideMesh(mpTriMesh);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::CCSubdivide()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::Int res = GPP::SubdivideMesh::CCSubdivideMesh(mpTriMesh);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::RefineMesh(int targetVertexCount)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::Int res = GPP::SubdivideMesh::RefineMesh(mpTriMesh, targetVertexCount);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::SimplifyMesh(int targetVertexCount)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::Int res = GPP::SimplifyMesh::QuadricSimplify(mpTriMesh, targetVertexCount);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::ParameterizeMesh()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        std::vector<GPP::Real> texCoords;
        GPP::Int res = GPP::ParameterizeMesh::AnglePreservingParameterize(mpTriMesh, &texCoords);
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
        GPP::Int vertexCount = mpTriMesh->GetVertexCount();
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            mpTriMesh->SetVertexCoord(vid, GPP::Vector3(texCoords.at(vid * 2), texCoords.at(vid * 2 + 1), 0));
        }
        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::SampleMesh()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        GPP::Int vertexCount = mpTriMesh->GetVertexCount();
        if (vertexCount < 3)
        {
            return;
        }
        GPP::PointCloud* pointCloud = new GPP::PointCloud;
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            pointCloud->InsertPoint(mpTriMesh->GetVertexCoord(vid), mpTriMesh->GetVertexNormal(vid));
        }
        pointCloud->SetHasNormal(true);
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop)
        {
            pointShop->SetPointCloud(pointCloud);
        }
        else
        {
            delete pointCloud;
            pointCloud = NULL;
        }
    }

    int MeshShopApp::GetMeshVertexCount()
    {
        if (mpTriMesh != NULL)
        {
            return mpTriMesh->GetVertexCount();
        }
        else
        {
            return 0;
        }
    }

    void MeshShopApp::UpdateMeshRendering()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        MagicCore::RenderSystem::Get()->RenderMesh("Mesh_MeshShop", "CookTorrance", mpTriMesh);
    }

    void MeshShopApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }
}
