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
        mObjCenterCoord(),
        mScaleValue(0),
        mpTriMeshFrom(NULL),
        mpPointCloudFrom(NULL),
        mpViewTool(NULL),
        mIsSeparateDisplay(false),
        mpPickToolRef(NULL),
        mpPickToolFrom(NULL),
        mpDumpInfo(NULL),
        mRefMarkIds(),
        mRefMarkPoints(),
        mFromMarkIds(),
        mFromMarkPoints(),
        mCommandType(NONE),
        mIsCommandInProgress(false),
        mUpdateModelRefRendering(false),
        mUpdateMarkRefRendering(false),
        mUpdateModelFromRendering(false),
        mUpdateMarkFromRendering(false)
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
        GPPFREEPOINTER(mpPickToolRef);
        GPPFREEPOINTER(mpPickToolFrom);
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
        if (mUpdateMarkRefRendering)
        {
            UpdateMarkRefRendering();
            mUpdateMarkRefRendering = false;
        }
        if (mUpdateModelRefRendering)
        {
            UpdateModelRefRendering();
            mUpdateModelRefRendering = false;
        }
        if (mUpdateMarkFromRendering)
        {
            UpdateMarkFromRendering();
            mUpdateMarkFromRendering = false;
        }
        if (mUpdateModelFromRendering)
        {
            UpdateModelFromRendering();
            mUpdateModelFromRendering = false;
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
            if (mpPickToolRef)
            {
                mpPickToolRef->MousePressed(arg.state.X.abs, arg.state.Y.abs);
            }
            if (mpPickToolFrom)
            {
                mpPickToolFrom->MousePressed(arg.state.X.abs, arg.state.Y.abs);
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
            if (mpPickToolRef)
            {
                mpPickToolRef->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
                GPP::Int pickedId = -1;
                if (mpTriMeshRef)
                {
                    pickedId = mpPickToolRef->GetPickVertexId();
                }
                else if (mpPointCloudRef)
                {
                    pickedId = mpPickToolRef->GetPickPointId();
                }
                mpPickToolRef->ClearPickedIds();
                if (pickedId != -1)
                {
                    mRefMarkIds.push_back(pickedId);
                    UpdateMarkRefRendering();
                }
            }
            if (mpPickToolFrom)
            {
                mpPickToolFrom->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
                GPP::Int pickedId = -1;
                if (mpTriMeshFrom)
                {
                    pickedId = mpPickToolFrom->GetPickVertexId();
                }
                else if (mpPointCloudFrom)
                {
                    pickedId = mpPickToolFrom->GetPickPointId();
                }
                mpPickToolFrom->ClearPickedIds();
                if (pickedId != -1)
                {
                    mFromMarkIds.push_back(pickedId);
                    UpdateMarkFromRendering();
                }
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
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("MeasureApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
        MagicCore::RenderSystem::Get()->ResertAllSceneNode();
    }

    void MeasureApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("MeasureApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Left_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Left_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsRef_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsRef_Left_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineRef_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineRef_Left_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshFrom_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshFrom_Right_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Right_Measure");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsFrom_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsFrom_Right_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineFrom_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineFrom_Right_MeasureApp");
        MagicCore::RenderSystem::Get()->ResertAllSceneNode();
    }

    void MeasureApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMeshRef);
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpTriMeshFrom);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickToolRef);
        GPPFREEPOINTER(mpPickToolFrom);
        GPPFREEPOINTER(mpDumpInfo);
        mRefMarkIds.clear();
        mRefMarkPoints.clear();
        mFromMarkIds.clear();
        mFromMarkPoints.clear();
    }

    void MeasureApp::ClearModelFromData(void)
    {
        GPPFREEPOINTER(mpPickToolFrom);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpTriMeshFrom);
        mFromMarkIds.clear();
        mFromMarkPoints.clear();
        mUpdateModelFromRendering = true;
        mUpdateMarkFromRendering = true;
        mpUI->SetFromModelInfo(0, 0);
        mpUI->SetDeviationInfo(0);
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
            case MagicApp::MeasureApp::DEVIATION:
                ComputeDeviation(false);
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
        InitViewTool();
        UpdateModelRefRendering();
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
                mRefMarkPoints = dumpDetails->GetSectionPathPoints();
            }
        }

        UpdateModelRefRendering();
        GPPFREEPOINTER(mpDumpInfo);
    }

    bool MeasureApp::IsCommandInProgress()
    {
        return mIsCommandInProgress;
    }

    void MeasureApp::SwitchSeparateDisplay()
    {
        mIsSeparateDisplay = !mIsSeparateDisplay;
        UpdateModelRefRendering();
        UpdateModelFromRendering();
        UpdateMarkRefRendering();
        UpdateMarkFromRendering();
        if (mpPickToolRef)
        {
            if (mIsSeparateDisplay)
            {
                mpPickToolRef->SetModelNodeName("ModelNodeLeft");
            }
            else
            {
                mpPickToolRef->SetModelNodeName("ModelNode");
            }
        }
        if (mpPickToolFrom)
        {
            if (mIsSeparateDisplay)
            {
                mpPickToolFrom->SetModelNodeName("ModelNodeRight");
            }
            else
            {
                mpPickToolFrom->SetModelNodeName("ModelNode");
            }
        }
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
            mpUI->SetGeodesicsInfo(0);
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
                    triMesh->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                    triMesh->UpdateNormal();
                    ClearModelFromData();
                    GPPFREEPOINTER(mpTriMeshRef);
                    GPPFREEPOINTER(mpPointCloudRef);
                    mpTriMeshRef = triMesh;
                    SetMeshColor(mpTriMeshRef, GPP::Vector3(0.6, 0.85, 0.75));
                    //InfoLog << "Import Mesh,  vertex: " << mpTriMeshRef->GetVertexCount() << " triangles: " << mpTriMeshRef->GetTriangleCount() << std::endl;
                    mpUI->SetRefModelInfo(mpTriMeshRef->GetVertexCount(), mpTriMeshRef->GetTriangleCount());
                    InitViewTool();
                    UpdateModelRefRendering();
                    updateMark = true;
                    // set up pick tool
                    GPPFREEPOINTER(mpPickToolRef);
                    mpPickToolRef = new MagicCore::PickTool;
                    mpPickToolRef->SetPickParameter(MagicCore::PM_POINT, true, NULL, mpTriMeshRef, "ModelNode");
                    if (mIsSeparateDisplay)
                    {
                        mpPickToolRef->SetModelNodeName("ModelNodeLeft");
                    }
                }
            }
            else if (extName == std::string("asc"))
            {
                GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
                if (pointCloud != NULL)
                {
                    pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                    ClearModelFromData();
                    GPPFREEPOINTER(mpTriMeshRef);
                    GPPFREEPOINTER(mpPointCloudRef);
                    mpPointCloudRef = pointCloud;
                    SetPointCloudColor(mpPointCloudRef, GPP::Vector3(0.6, 0.85, 0.75));
                    //InfoLog << "Import Point Cloud: " << mpPointCloudRef->GetPointCount() << " points" << std::endl;
                    mpUI->SetRefModelInfo(mpPointCloudRef->GetPointCount(), 0);
                    InitViewTool();
                    UpdateModelRefRendering();
                    updateMark = true;
                    // set up pick tool
                    GPPFREEPOINTER(mpPickToolRef);
                    mpPickToolRef = new MagicCore::PickTool;
                    mpPickToolRef->SetPickParameter(MagicCore::PM_POINT, false, mpPointCloudRef, NULL, "ModelNode");
                    if (mIsSeparateDisplay)
                    {
                        mpPickToolRef->SetModelNodeName("ModelNodeLeft");
                    }
                }
            }
            if (updateMark)
            {
                mRefMarkIds.clear();
                mRefMarkPoints.clear();
                UpdateMarkRefRendering();
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
        mRefMarkIds.pop_back();
        mRefMarkPoints.clear();
        UpdateMarkRefRendering();
        mpUI->SetGeodesicsInfo(0);
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
        else if (mRefMarkIds.size() < 2)
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
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(mpTriMeshRef, mRefMarkIds, true, pathVertexIds, distance);
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / mScaleValue);
            mRefMarkPoints.clear();
            for (std::vector<GPP::Int>::iterator pathItr = pathVertexIds.begin(); pathItr != pathVertexIds.end(); ++pathItr)
            {
                mRefMarkPoints.push_back(mpTriMeshRef->GetVertexCoord(*pathItr));
            }
            mUpdateMarkRefRendering = true;
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
        else if (mRefMarkIds.size() < 2)
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
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeExactGeodesics(mpTriMeshRef, mRefMarkIds, true, pathPoints, distance);
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / mScaleValue);
            mRefMarkPoints.clear();
            mRefMarkPoints.swap(pathPoints);
            mUpdateMarkRefRendering = true;
        }
    }

    bool MeasureApp::ImportModelFrom()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0ASC Files(*.asc)\0*.asc\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            mpUI->SetDeviationInfo(0);
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
                    triMesh->UnifyCoords(mScaleValue, mObjCenterCoord);
                    triMesh->UpdateNormal();
                    GPPFREEPOINTER(mpTriMeshFrom);
                    GPPFREEPOINTER(mpPointCloudFrom);
                    mpTriMeshFrom = triMesh;
                    SetMeshColor(mpTriMeshFrom, GPP::Vector3(0.6, 0.75, 0.85));
                    //InfoLog << "Import Mesh,  vertex: " << mpTriMeshFrom->GetVertexCount() << " triangles: " << mpTriMeshFrom->GetTriangleCount() << std::endl;
                    mpUI->SetFromModelInfo(mpTriMeshFrom->GetVertexCount(), mpTriMeshFrom->GetTriangleCount());
                    InitViewTool();
                    UpdateModelFromRendering();
                    updateMark = true;
                    // set up pick tool
                    GPPFREEPOINTER(mpPickToolFrom);
                    mpPickToolFrom = new MagicCore::PickTool;
                    mpPickToolFrom->SetPickParameter(MagicCore::PM_POINT, true, NULL, mpTriMeshFrom, "ModelNode");
                    if (mIsSeparateDisplay)
                    {
                        mpPickToolFrom->SetModelNodeName("ModelNodeRight");
                    }
                }
            }
            else if (extName == std::string("asc"))
            {
                GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
                if (pointCloud != NULL)
                {
                    pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                    GPPFREEPOINTER(mpTriMeshFrom);
                    GPPFREEPOINTER(mpPointCloudFrom);
                    mpPointCloudFrom = pointCloud;
                    SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0.6, 0.75, 0.85));
                    //InfoLog << "Import Point Cloud: " << mpPointCloudFrom->GetPointCount() << " points" << std::endl;
                    mpUI->SetFromModelInfo(mpPointCloudFrom->GetPointCount(), 0);
                    InitViewTool();
                    UpdateModelFromRendering();
                    updateMark = true;
                    // set up pick tool
                    GPPFREEPOINTER(mpPickToolFrom);
                    mpPickToolFrom = new MagicCore::PickTool;
                    mpPickToolFrom->SetPickParameter(MagicCore::PM_POINT, false, mpPointCloudFrom, NULL, "ModelNode");
                    if (mIsSeparateDisplay)
                    {
                        mpPickToolFrom->SetModelNodeName("ModelNodeRight");
                    }
                }
            }
            if (updateMark)
            {
                mFromMarkIds.clear();
                mFromMarkPoints.clear();
                UpdateMarkFromRendering();
                return true;
            }
        }
        return false;
    }

    void MeasureApp::ComputeDeviation(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMeshRef == NULL && mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请导入测量模型", "温馨提示", MB_OK);
            return;
        }
        if (mpTriMeshFrom == NULL && mpPointCloudFrom == NULL)
        {
            MessageBox(NULL, "请导入参考模型", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = DEVIATION;
            DoCommand(true);
        }
        else
        {
            if (mpTriMeshRef != NULL)
            {
                MessageBox(NULL, "目前暂时没有支持", "温馨提示", MB_OK);
            }
            else if (mpPointCloudRef != NULL)
            {
                GPP::PointCloudPointList pointListRef(mpPointCloudRef);
                GPP::IPointList* pointListFrom = NULL;
                if (mpPointCloudFrom != NULL)
                {
                    pointListFrom = new GPP::PointCloudPointList(mpPointCloudFrom);
                }
                else
                {
                    pointListFrom = new GPP::TriMeshPointList(mpTriMeshFrom);
                }
                GPP::Real maxDistance = 0;
                GPP::Int maxPointId = 0;
                GPP::Int pointFromCount = pointListFrom->GetPointCount();
                GPP::Real* fromDistance = new GPP::Real[pointFromCount];
                mIsCommandInProgress = true;
                GPP::ErrorCode res = GPP::MeasurePointCloud::ComputeOneSideDistance(&pointListRef, pointListFrom, maxDistance, maxPointId, fromDistance);
                mIsCommandInProgress = false;
                GPPFREEPOINTER(pointListFrom);
                if (res != GPP_NO_ERROR)
                {
                    GPPFREEARRAY(fromDistance);
                    MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                    return;
                }
                mpUI->SetDeviationInfo(maxDistance / mScaleValue);
                if (mpTriMeshFrom != NULL)
                {
                    for (GPP::Int vid = 0; vid < pointFromCount; vid++)
                    {
                        mpTriMeshFrom->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.4 + fromDistance[vid]));
                    }
                }
                else if (mpPointCloudFrom != NULL)
                {
                    for (GPP::Int pid = 0; pid < pointFromCount; pid++)
                    {
                        mpPointCloudFrom->SetPointColor(pid, MagicCore::ToolKit::ColorCoding(0.4 + fromDistance[pid]));
                    }
                }
                GPPFREEARRAY(fromDistance);
                mUpdateModelFromRendering = true;
            }
            //std::vector<GPP::Int> pathVertexIds;
            //GPP::Real distance = 0;
            ////GPP::DumpOnce();
            //mIsCommandInProgress = true;
            //GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(mpTriMeshRef, mRefMarkIds, true, pathVertexIds, distance);
            //mIsCommandInProgress = false;
            //if (res != GPP_NO_ERROR)
            //{
            //    MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
            //    return;
            //}
            //mRefMarkPoints.clear();
            //for (std::vector<GPP::Int>::iterator pathItr = pathVertexIds.begin(); pathItr != pathVertexIds.end(); ++pathItr)
            //{
            //    mRefMarkPoints.push_back(mpTriMeshRef->GetVertexCoord(*pathItr));
            //}
            //mUpdateMarkRefRendering = true;
        }
    }

    void MeasureApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    void MeasureApp::UpdateModelRefRendering()
    {
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");
            MagicCore::RenderSystem::Get()->RenderMesh("MeshRef_Left_Measure", "CookTorrance", mpTriMeshRef, MagicCore::RenderSystem::MODEL_NODE_LEFT);
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Measure");
            if (mpPointCloudRef && mpPointCloudRef->HasNormal())
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Left_Measure", "CookTorrancePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_LEFT);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Left_Measure", "SimplePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_LEFT);
            }
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Left_Measure");
            MagicCore::RenderSystem::Get()->RenderMesh("MeshRef_Measure", "CookTorrance", mpTriMeshRef, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Left_Measure");
            if (mpPointCloudRef && mpPointCloudRef->HasNormal())
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Measure", "CookTorrancePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Measure", "SimplePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
        }
    }

    void MeasureApp::UpdateMarkRefRendering()
    {
        std::vector<GPP::Vector3> markCoords = mRefMarkPoints;
        if (mRefMarkIds.size() > 0)
        {
            if (mpTriMeshRef != NULL)
            {
                for (std::vector<GPP::Int>::iterator itr = mRefMarkIds.begin(); itr != mRefMarkIds.end(); ++itr)
                {
                    markCoords.push_back(mpTriMeshRef->GetVertexCoord(*itr));
                }
            }
            else if (mpPointCloudRef != NULL)
            {
                for (std::vector<GPP::Int>::iterator itr = mRefMarkIds.begin(); itr != mRefMarkIds.end(); ++itr)
                {
                    markCoords.push_back(mpPointCloudRef->GetPointCoord(*itr));
                }
            }
        }
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsRef_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPointList("MarkPointsRef_Left_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords, MagicCore::RenderSystem::MODEL_NODE_LEFT);
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineRef_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLineRef_Left_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mRefMarkPoints, true, MagicCore::RenderSystem::MODEL_NODE_LEFT);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsRef_Left_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPointList("MarkPointsRef_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineRef_Left_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLineRef_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mRefMarkPoints, true, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
    }

    void MeasureApp::UpdateModelFromRendering()
    {
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshFrom_Measure");
            MagicCore::RenderSystem::Get()->RenderMesh("MeshFrom_Right_Measure", "CookTorrance", mpTriMeshFrom, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Measure");
            if (mpPointCloudFrom && mpPointCloudFrom->HasNormal())
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_Right_Measure", "CookTorrancePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_Right_Measure", "SimplePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
            }
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshFrom_Right_Measure");
            MagicCore::RenderSystem::Get()->RenderMesh("MeshFrom_Measure", "CookTorrance", mpTriMeshFrom, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Right_Measure");
            if (mpPointCloudFrom && mpPointCloudFrom->HasNormal())
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_Measure", "CookTorrancePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_Measure", "SimplePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
        }
    }

    void MeasureApp::UpdateMarkFromRendering()
    {
        std::vector<GPP::Vector3> markCoords = mFromMarkPoints;
        if (mFromMarkIds.size() > 0)
        {
            if (mpTriMeshFrom != NULL)
            {
                for (std::vector<GPP::Int>::iterator itr = mFromMarkIds.begin(); itr != mFromMarkIds.end(); ++itr)
                {
                    markCoords.push_back(mpTriMeshFrom->GetVertexCoord(*itr));
                }
            }
            else if (mpPointCloudFrom != NULL)
            {
                for (std::vector<GPP::Int>::iterator itr = mFromMarkIds.begin(); itr != mFromMarkIds.end(); ++itr)
                {
                    markCoords.push_back(mpPointCloudFrom->GetPointCoord(*itr));
                }
            }
        }
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsFrom_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPointList("MarkPointsFrom_Right_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineFrom_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLineFrom_Right_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mFromMarkPoints, true, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointsFrom_Right_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPointList("MarkPointsFrom_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLineFrom_Right_MeasureApp");
            MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLineFrom_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mFromMarkPoints, true, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
    }

    void MeasureApp::SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color)
    {
        if (pointCloud == NULL)
        {
            return;
        }
        GPP::Int pointCount = pointCloud->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            pointCloud->SetPointColor(pid, color);
        }
    }

    void MeasureApp::SetMeshColor(GPP::TriMesh* triMesh, const GPP::Vector3& color)
    {
        if (triMesh == NULL)
        {
            return;
        }
        GPP::Int vertexCount = triMesh->GetVertexCount();
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            triMesh->SetVertexColor(vid, color);
        }
    }
}
