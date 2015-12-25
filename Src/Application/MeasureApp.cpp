#include "stdafx.h"
#include "MeasureApp.h"
#include "MeasureAppUI.h"
#include "AppManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "../Common/RenderSystem.h"

namespace MagicApp
{
    MeasureApp::MeasureApp() :
        mpUI(NULL),
        mMouseMode(MM_VIEW),
        mpTriMeshRef(NULL),
        mpPointCloudRef(NULL),
        mpTriMeshFrom(NULL),
        mpPointCloudFrom(NULL),
        mpViewTool(NULL),
        mpPickTool(NULL),
        mpDumpInfo(NULL),
        mMeshRefMarkIds(),
        mMarkPoints()
    {
    }

    MeasureApp::~MeasureApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMeshRef);
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpTriMeshFrom);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpDumpInfo);
    }

    bool MeasureApp::Enter()
    {
        InfoLog << "Enter MeasureApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new MeasureAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool MeasureApp::Update(float timeElapsed)
    {
        return true;
    }

    bool MeasureApp::Exit()
    {
        InfoLog << "Exit MeasureApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool MeasureApp::MouseMoved( const OIS::MouseEvent &arg )
    {
        if (mMouseMode == MM_VIEW && mpViewTool != NULL)
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

    bool MeasureApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mMouseMode == MM_VIEW && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (mMouseMode == MM_PICK_MESH_REF && mpPickTool != NULL)
        {
            mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool MeasureApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mMouseMode == MM_PICK_MESH_REF && mpPickTool != NULL)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickVertexId();
            mpPickTool->ClearPickedIds();
            if (pickedId == -1)
            {
                return true;
            }
            mMeshRefMarkIds.push_back(pickedId);
            UpdateMarkRendering();
        }
        return  true;
    }

    bool MeasureApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_V && (mpTriMeshRef !=NULL || mpTriMeshFrom != NULL))
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_POINTS);
        }
        else if (arg.key == OIS::KC_E && (mpTriMeshRef !=NULL || mpTriMeshFrom != NULL))
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
        }
        else if (arg.key == OIS::KC_F && (mpTriMeshRef !=NULL || mpTriMeshFrom != NULL))
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
        }
        else if (arg.key == OIS::KC_D)
        {
            RunDumpInfo();
        }
        return true;
    }

    void MeasureApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("MeasureApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
    }

    void MeasureApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("MeasureApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPoints_MeasureApp");
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        } 
    }

    void MeasureApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMeshRef);
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpTriMeshFrom);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpDumpInfo);
        mMouseMode = MM_VIEW;
        mMeshRefMarkIds.clear();
        mMarkPoints.clear();
    }

    void MeasureApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
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
        GPPFREEPOINTER(mpTriMeshRef);
        mpTriMeshRef = CopyTriMesh(mpDumpInfo->GetTriMesh());
        mpTriMeshRef->UpdateNormal();
        InitViewTool();
        UpdateModelRendering();
    }

    void MeasureApp::RunDumpInfo()
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

        //Copy result
        GPPFREEPOINTER(mpTriMeshRef);
        mpTriMeshRef = CopyTriMesh(mpDumpInfo->GetTriMesh());
        mpTriMeshRef->UnifyCoords(2.0);
        mpTriMeshRef->UpdateNormal();

        UpdateModelRendering();
        GPPFREEPOINTER(mpDumpInfo);
    }

    void MeasureApp::SwitchToViewMode()
    {
        mMouseMode = MM_VIEW;
    }

    bool MeasureApp::ImportModelRef()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0ASC Files(*.asc)\0*.asc\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            size_t dotPos = fileName.rfind('.');
            if (dotPos == std::string::npos)
            {
                return false;
            }
            std::string extName = fileName;
            extName = extName.substr(dotPos + 1);
            bool updateMark = false;
            if (extName == std::string("obj") || extName == std::string("stl") || extName == std::string("off") || extName == std::string("ply"))
            {
                GPP::TriMesh* triMesh = GPP::Parser::ImportTriMesh(fileName);
                if (triMesh != NULL)
                { 
                    triMesh->UnifyCoords(2.0);
                    triMesh->UpdateNormal();
                    GPPFREEPOINTER(mpTriMeshRef);
                    GPPFREEPOINTER(mpPointCloudRef);
                    mpTriMeshRef = triMesh;
                    InfoLog << "Import Mesh,  vertex: " << mpTriMeshRef->GetVertexCount() << " triangles: " << mpTriMeshRef->GetTriangleCount() << std::endl;
                    InitViewTool();
                    UpdateModelRendering();
                    updateMark = true;
                }
            }
            else if (extName == std::string("asc"))
            {
                GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
                if (pointCloud != NULL)
                {
                    pointCloud->UnifyCoords(2.0);
                    GPPFREEPOINTER(mpTriMeshRef);
                    GPPFREEPOINTER(mpPointCloudRef);
                    mpPointCloudRef = pointCloud;
                    InfoLog << "Import Point Cloud: " << mpPointCloudRef->GetPointCount() << " points" << std::endl;
                    InitViewTool();
                    UpdateModelRendering();
                    updateMark = true;
                }
            }
            if (updateMark)
            {
                mMeshRefMarkIds.clear();
                mMarkPoints.clear();
                MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPoints_MeasureApp");
                SwitchToViewMode();
                return true;
            }
        }
        return false;
    }

    void MeasureApp::SwitchMeshRefControlState()
    {
        if (mpTriMeshRef == NULL)
        {
            return;
        }
        if (mMouseMode == MM_VIEW)
        {
            mMouseMode = MM_PICK_MESH_REF;
            InitPickTool();
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, mpTriMeshRef);
        }
        else if (mMouseMode == MM_PICK_MESH_REF)
        {
            mMouseMode = MM_VIEW;
        }
    }

    void MeasureApp::DeleteMeshMarkRef()
    {
        mMeshRefMarkIds.pop_back();
        mMarkPoints.clear();
        UpdateMarkRendering();
    }

    void MeasureApp::ComputeApproximateGeodesics()
    {
        if (mpTriMeshRef == NULL || mMeshRefMarkIds.size() < 2)
        {
            return;
        }
        std::vector<GPP::Int> pathVertexIds;
        GPP::Real distance = 0;
        //GPP::DumpOnce();
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeGeodesics(mpTriMeshRef, mMeshRefMarkIds, true, pathVertexIds, distance);
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        mMarkPoints.clear();
        for (std::vector<GPP::Int>::iterator pathItr = pathVertexIds.begin(); pathItr != pathVertexIds.end(); ++pathItr)
        {
            mMarkPoints.push_back(mpTriMeshRef->GetVertexCoord(*pathItr));
        }
        UpdateMarkRendering();
        SwitchToViewMode();
    }

    void MeasureApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    void MeasureApp::InitPickTool()
    {
        if (mpPickTool == NULL)
        {
            mpPickTool = new MagicCore::PickTool;
        }
        mpPickTool->Reset();
    }

    void MeasureApp::UpdateModelRendering()
    {
        if (mpTriMeshRef != NULL)
        {
            MagicCore::RenderSystem::Get()->RenderMesh("MeshRef_Measure", "CookTorrance", mpTriMeshRef);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");
        }
        if (mpPointCloudRef != NULL)
        {
            if (mpPointCloudRef->HasNormal())
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Measure", "CookTorrancePoint", mpPointCloudRef);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Measure", "SimplePoint", mpPointCloudRef);
            }
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Measure");
        }
    }

    void MeasureApp::UpdateMarkRendering()
    {
        std::vector<GPP::Vector3> markCoords = mMarkPoints;
        if (mpTriMeshRef != NULL && mMeshRefMarkIds.size() > 0)
        {
            for (std::vector<GPP::Int>::iterator itr = mMeshRefMarkIds.begin(); itr != mMeshRefMarkIds.end(); ++itr)
            {
                markCoords.push_back(mpTriMeshRef->GetVertexCoord(*itr));
            }
        }
        MagicCore::RenderSystem::Get()->RenderPointList("MarkPoints_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords);
    }
}
