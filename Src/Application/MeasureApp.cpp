#include "stdafx.h"
#include <process.h>
#include "MeasureApp.h"
#include "MeasureAppUI.h"
#include "AppManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "../Common/RenderSystem.h"
#include "DumpMeasureMesh.h"

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        MeasureApp* app = (MeasureApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    MeasureApp::MeasureApp() :
        mpUI(NULL),
        mpTriMeshRef(NULL),
        mpPointCloudRef(NULL),
        mpTriMeshFrom(NULL),
        mpPointCloudFrom(NULL),
        mpViewTool(NULL),
        mpPickTool(NULL),
        mpDumpInfo(NULL),
        mMeshRefMarkIds(),
        mMarkPoints(),
        mCommandType(NONE),
        mIsCommandInProgress(false),
        mUpdateModelRendering(false),
        mUpdateMarkRendering(false)
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

    bool MeasureApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            int progressValue = int(GPP::GetApiProgress() * 100.0);
            mpUI->SetProgressbar(progressValue);
        }
        if (mUpdateMarkRendering)
        {
            UpdateMarkRendering();
            mUpdateMarkRendering = false;
        }
        if (mUpdateModelRendering)
        {
            UpdateModelRendering();
            mUpdateModelRendering = false;
        }
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
        if (arg.state.buttonDown(OIS::MB_Middle) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_MIDDLE_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_LEFT_DOWN);
        }     
        
        return true;
    }

    bool MeasureApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((arg.state.buttonDown(OIS::MB_Middle) || arg.state.buttonDown(OIS::MB_Left)) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (arg.state.buttonDown(OIS::MB_Right) && mIsCommandInProgress == false && mpPickTool)
        {
            mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool MeasureApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpPickTool && mIsCommandInProgress == false && id == OIS::MB_Right)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickVertexId();
            mpPickTool->ClearPickedIds();
            if (pickedId != -1)
            {
                mMeshRefMarkIds.push_back(pickedId);
                UpdateMarkRendering();
            }
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
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
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
        mMeshRefMarkIds.clear();
        mMarkPoints.clear();
    }

    void MeasureApp::DoCommand(bool isSubThread)
    {
        if (isSubThread)
        {
            GPP::ResetApiProgress();
            mpUI->StartProgressbar(100);
            _beginthreadex(NULL, 0, RunThread, (void *)this, 0, NULL);
        }
        else
        {
            switch (mCommandType)
            {
            case MagicApp::MeasureApp::NONE:
                break;
            case MagicApp::MeasureApp::GEODESICS_APPROXIMATE:
                ComputeApproximateGeodesics(false);
                break;
            case MagicApp::MeasureApp::GEODESICS_EXACT:
                ComputeExactGeodesics(false);
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
        }
    }

    bool MeasureApp::IsCommandAvaliable()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        return true;
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

        if (mpDumpInfo->GetApiName() == GPP::MESH_MEASURE_SECTION_EXACT)
        {
            GPP::DumpMeshMeasureSectionExact* dumpDetails = dynamic_cast<GPP::DumpMeshMeasureSectionExact*>(mpDumpInfo);
            if (dumpDetails)
            {
                mMarkPoints = dumpDetails->GetSectionPathPoints();
            }
        }

        UpdateModelRendering();
        GPPFREEPOINTER(mpDumpInfo);
    }

    bool MeasureApp::IsCommandInProgress()
    {
        return mIsCommandInProgress;
    }

    bool MeasureApp::ImportModelRef()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
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
                    if (triMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
                    {
                        triMesh->FuseVertex();
                    }
                    triMesh->UnifyCoords(2.0);
                    triMesh->UpdateNormal();
                    GPPFREEPOINTER(mpTriMeshRef);
                    GPPFREEPOINTER(mpPointCloudRef);
                    mpTriMeshRef = triMesh;
                    InfoLog << "Import Mesh,  vertex: " << mpTriMeshRef->GetVertexCount() << " triangles: " << mpTriMeshRef->GetTriangleCount() << std::endl;
                    InitViewTool();
                    UpdateModelRendering();
                    updateMark = true;
                    GPPFREEPOINTER(mpPickTool);
                    mpPickTool = new MagicCore::PickTool;
                    mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, mpTriMeshRef, "ModelNode");
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
                MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
                return true;
            }
        }
        return false;
    }

    void MeasureApp::DeleteMeshMarkRef()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        mMeshRefMarkIds.pop_back();
        mMarkPoints.clear();
        UpdateMarkRendering();
    }

    void MeasureApp::ComputeApproximateGeodesics(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMeshRef == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMeshRefMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = GEODESICS_APPROXIMATE;
            DoCommand(true);
        }
        else
        {
            std::vector<GPP::Int> pathVertexIds;
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(mpTriMeshRef, mMeshRefMarkIds, true, pathVertexIds, distance);
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mMarkPoints.clear();
            for (std::vector<GPP::Int>::iterator pathItr = pathVertexIds.begin(); pathItr != pathVertexIds.end(); ++pathItr)
            {
                mMarkPoints.push_back(mpTriMeshRef->GetVertexCoord(*pathItr));
            }
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::ComputeExactGeodesics(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMeshRef == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMeshRefMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        else if (mpTriMeshRef->GetVertexCount() > 200000 && isSubThread)
        {
            if (MessageBox(NULL, "测量网格顶点大于200k，测量时间会比较长，是否继续？", "温馨提示", MB_OKCANCEL) != IDOK)
            {
                return;
            }
        }
        if (isSubThread)
        {
            mCommandType = GEODESICS_EXACT;
            DoCommand(true);
        }
        else
        {
            std::vector<GPP::Vector3> pathPoints;
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeExactGeodesics(mpTriMeshRef, mMeshRefMarkIds, true, pathPoints, distance);
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                return;
            }
            mMarkPoints.clear();
            mMarkPoints.swap(pathPoints);
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
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
        MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLine_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mMarkPoints, true);
    }

}
