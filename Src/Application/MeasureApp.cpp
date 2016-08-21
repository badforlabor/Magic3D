#include "stdafx.h"
#include <process.h>
#include "MeasureApp.h"
#include "MeasureAppUI.h"
#include "AppManager.h"
#include "ModelManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "../Common/RenderSystem.h"
#if DEBUGDUMPFILE
#include "DumpMeasureMesh.h"
#endif

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
        mpViewTool(NULL),
        mpPickTool(NULL),
        mDisplayMode(0),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mMarkIds(),
        mMarkPoints(),
        mCommandType(NONE),
        mIsCommandInProgress(false),
        mUpdateModelRendering(false),
        mUpdateMarkRendering(false),
        mGeodesicAccuracy(0.5),
        mIsFlatRenderingMode(true)
    {
    }

    MeasureApp::~MeasureApp()
    {
        GPPFREEPOINTER(mpUI);        
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
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
        UpdateModelRendering();
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
        else if (arg.state.buttonDown(OIS::MB_Right) && mIsCommandInProgress == false)
        {
            if (mpPickTool)
            {
                mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
            }
        }
        return true;
    }

    bool MeasureApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        if (mIsCommandInProgress == false && id == OIS::MB_Right)
        {
            if (mpPickTool)
            {
                mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
                GPP::Int pickedId = -1;
                pickedId = mpPickTool->GetPickVertexId();
                mpPickTool->ClearPickedIds();
                if (pickedId != -1)
                {
                    mMarkIds.push_back(pickedId);
                    UpdateMarkRendering();
                }
            }
        }
        return  true;
    }

    bool MeasureApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_D)
        {
#if DEBUGDUMPFILE
            RunDumpInfo();
#endif
        }
        else if (arg.key == OIS::KC_G)
        {
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            std::vector<GPP::Real> curvature;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeGaussCurvature(triMesh, curvature);
            if (res != GPP_NO_ERROR)
            {
                return true;
            }
            GPP::Int vertexCount = triMesh->GetVertexCount();
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                triMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.6 + fabs(curvature.at(vid)) * 2.0));
            }
            UpdateModelRendering();
        }
        else if (arg.key == OIS::KC_L)
        {
            std::vector<GPP::Vector3> lineSegments;
            for (int mid = 0; mid < mMarkPoints.size() - 1; mid++)
            {
                lineSegments.push_back(mMarkPoints.at(mid));
                lineSegments.push_back(mMarkPoints.at(mid + 1));
            }
            GPP::Parser::ExportLineSegmentToPovray("edge.inc", lineSegments, 0.0025, GPP::Vector3(0.09, 0.48627, 0.69));
        }
        return true;
    }

    void MeasureApp::WindowFocusChanged( Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
    }

    void MeasureApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
        Ogre::Light* light = sceneManager->createLight("MeasureApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
        InitViewTool();
        if (ModelManager::Get()->GetMesh())
        {
            // set up pick tool
            GPPFREEPOINTER(mpPickTool);
            mpPickTool = new MagicCore::PickTool;
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, ModelManager::Get()->GetMesh(), "ModelNode");
            mpUI->SetModelInfo(ModelManager::Get()->GetMesh()->GetVertexCount(), ModelManager::Get()->GetMesh()->GetTriangleCount());
        }
    }

    void MeasureApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("MeasureApp_SimpleLight");
        //MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_Measure");       
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPoints_MeasureApp");       
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
        //MagicCore::RenderSystem::Get()->ResertAllSceneNode();
    }

    void MeasureApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        mMarkIds.clear();
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
            case MagicApp::MeasureApp::GEOMESICS_FAST_EXACT:
                FastComputeExactGeodesics(mGeodesicAccuracy, false);
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

#if DEBUGDUMPFILE
    void MeasureApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        /*if (dumpInfo == NULL)
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
        InitViewTool();
        UpdateModelRefRendering();*/
    }

    void MeasureApp::RunDumpInfo()
    {
        //if (mpDumpInfo == NULL)
        //{
        //    return;
        //}
        //GPP::ErrorCode res = mpDumpInfo->Run();
        //if (res != GPP_NO_ERROR)
        //{
        //    MagicCore::ToolKit::Get()->SetAppRunning(false);
        //    return;
        //}

        ////Copy result
        //GPPFREEPOINTER(mpTriMeshRef);
        //mpTriMeshRef = CopyTriMesh(mpDumpInfo->GetTriMesh());
        //mpTriMeshRef->UnifyCoords(2.0);
        //mpTriMeshRef->UpdateNormal();

        //if (mpDumpInfo->GetApiName() == GPP::MESH_MEASURE_SECTION_EXACT || mpDumpInfo->GetApiName() == GPP::MESH_MEASURE_SECTION_FAST_EXACT)
        //{
        //    GPP::DumpMeshMeasureSectionExact* dumpDetails = dynamic_cast<GPP::DumpMeshMeasureSectionExact*>(mpDumpInfo);
        //    if (dumpDetails)
        //    {
        //        mRefMarkPoints = dumpDetails->GetSectionPathPoints();
        //    }
        //}

        //UpdateModelRefRendering();
        //UpdateMarkRefRendering();
        //GPPFREEPOINTER(mpDumpInfo);
    }
#endif

    bool MeasureApp::IsCommandInProgress()
    {
        return mIsCommandInProgress;
    }

    void MeasureApp::SwitchDisplayMode()
    {
        mDisplayMode++;
        if (mDisplayMode > 3)
        {
            mDisplayMode = 0;
        }
        if (mDisplayMode == 0)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            mIsFlatRenderingMode = true;
        }
        else if (mDisplayMode == 1)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            mIsFlatRenderingMode = false;
        }
        else if (mDisplayMode == 2)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
            mIsFlatRenderingMode = false;
        }
        else if (mDisplayMode == 3)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_POINTS);
            mIsFlatRenderingMode = false;
        }
        Ogre::Material* material = dynamic_cast<Ogre::Material*>(Ogre::MaterialManager::getSingleton().getByName("CookTorrance").getPointer());
        if (material)
        {
            if (mDisplayMode == 0 || mDisplayMode == 1)
            {
                material->setCullingMode(Ogre::CullingMode::CULL_NONE);
            }
            else
            {
                material->setCullingMode(Ogre::CullingMode::CULL_CLOCKWISE);
            }
        }
        mUpdateModelRendering = true;
    }

    bool MeasureApp::ImportModel()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            mpUI->SetGeodesicsInfo(0);
            ModelManager::Get()->ClearPointCloud();
            if (ModelManager::Get()->ImportMesh(fileName) == false)
            {
                MessageBox(NULL, "网格导入失败", "温馨提示", MB_OK);
                return false;
            }
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            if (triMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
            {
                triMesh->FuseVertex();
            }
            mpUI->SetModelInfo(triMesh->GetVertexCount(), triMesh->GetTriangleCount());
            UpdateModelRendering();
            mMarkIds.clear();
            mMarkPoints.clear();
            UpdateMarkRendering();
            // set up pick tool
            GPPFREEPOINTER(mpPickTool);
            mpPickTool = new MagicCore::PickTool;
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, triMesh, "ModelNode");
            return true;
        }
        return false;
    }

    void MeasureApp::DeleteMeshMark()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        mMarkIds.pop_back();
        mMarkPoints.clear();
        UpdateMarkRendering();
        mpUI->SetGeodesicsInfo(0);
    }

    void MeasureApp::ComputeApproximateGeodesics(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
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
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, mMarkIds, true, pathVertexIds, distance);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            for (std::vector<GPP::Int>::iterator pathItr = pathVertexIds.begin(); pathItr != pathVertexIds.end(); ++pathItr)
            {
                mMarkPoints.push_back(triMesh->GetVertexCoord(*pathItr));
            }
            mUpdateMarkRendering = true;
        }
    }
 
    void MeasureApp::FastComputeExactGeodesics(double accuracy, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = GEOMESICS_FAST_EXACT;
            mGeodesicAccuracy = accuracy;
            DoCommand(true);
        }
        else
        {
            std::vector<GPP::Vector3> pathPoints;
            std::vector<GPP::PointOnEdge> pathInfos;
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::FastComputeExactGeodesics(triMesh, mMarkIds, true, 
                pathPoints, distance, &pathInfos, accuracy);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            mMarkPoints.swap(pathPoints);
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::ComputeExactGeodesics(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        else if (triMesh->GetVertexCount() > 200000 && isSubThread)
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
            std::vector<GPP::PointOnEdge> pathInfos;
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeExactGeodesics(triMesh, mMarkIds, true, pathPoints, distance, &pathInfos);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            mMarkPoints.swap(pathPoints);
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::MeasureArea()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        GPP::Real area = 0;
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeArea(triMesh, area);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        mpUI->SetModelArea(area / ModelManager::Get()->GetScaleValue() / ModelManager::Get()->GetScaleValue());
    }

    void MeasureApp::MeasureVolume()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        GPP::Real volume = 0;
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeVolume(triMesh, volume);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        mpUI->SetModelVolume(volume / ModelManager::Get()->GetScaleValue() / ModelManager::Get()->GetScaleValue() / ModelManager::Get()->GetScaleValue());
    }

    void MeasureApp::MeasureCurvature()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        std::vector<GPP::Real> curvature;
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeMeanCurvature(triMesh, curvature);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        GPP::Int vertexCount = triMesh->GetVertexCount();
        triMesh->SetHasColor(true);
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            triMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.6 + curvature.at(vid) / 10.0));
        }
        UpdateModelRendering();
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
        if (ModelManager::Get()->GetMesh() == NULL)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_Measure");
            return;
        }
        MagicCore::RenderSystem::Get()->RenderMesh("Mesh_Measure", "CookTorrance", ModelManager::Get()->GetMesh(), 
            MagicCore::RenderSystem::MODEL_NODE_CENTER, NULL, NULL, mIsFlatRenderingMode);
    }

    void MeasureApp::UpdateMarkRendering()
    {
        std::vector<GPP::Vector3> markCoords = mMarkPoints;
        if (mMarkIds.size() > 0)
        {
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            if (triMesh != NULL)
            {
                for (std::vector<GPP::Int>::iterator itr = mMarkIds.begin(); itr != mMarkIds.end(); ++itr)
                {
                    markCoords.push_back(triMesh->GetVertexCoord(*itr));
                }
            }
            MagicCore::RenderSystem::Get()->RenderPointList("MarkPoints_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLine_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mMarkPoints, true, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else
        {     
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPoints_MeasureApp");       
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
        }
    }
}
