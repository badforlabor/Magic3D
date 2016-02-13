#include "stdafx.h"
#include <process.h>
#include "PointShopApp.h"
#include "PointShopAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "AppManager.h"
#include "MeshShopApp.h"
#include <algorithm>

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        PointShopApp* app = (PointShopApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    PointShopApp::PointShopApp() :
        mpUI(NULL),
        mpPointCloud(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mpViewTool(NULL),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mCommandType(NONE),
        mUpdatePointCloudRendering(false),
        mIsCommandInProgress(false),
        mpTriMesh(NULL),
        mIsDepthImage(0)
    {
    }

    PointShopApp::~PointShopApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpPointCloud);
        GPPFREEPOINTER(mpViewTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
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

    bool PointShopApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            int progressValue = int(GPP::GetApiProgress() * 100.0);
            mpUI->SetProgressbar(progressValue);
        }
        if (mUpdatePointCloudRendering)
        {
            UpdatePointCloudRendering();
            mUpdatePointCloudRendering = false;
        }
        if (mpTriMesh)
        {
            AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
            MeshShopApp* meshShop = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
            if (meshShop)
            {
                mpTriMesh->UpdateNormal();
                meshShop->SetMesh(mpTriMesh, mObjCenterCoord, mScaleValue);
                mpTriMesh = NULL;
            }
            else
            {
                GPPFREEPOINTER(mpTriMesh);
            }
        }
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
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        return true;
    }

    bool PointShopApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_D)
        {
#if DEBUGDUMPFILE
            RunDumpInfo();
#endif
        }
        else if (arg.key == OIS::KC_R) // A temporary command
        {
            if (mpPointCloud)
            {
                GPP::ConsolidatePointCloud::ConsolidateRawScanData(mpPointCloud, 1280, 1024, false);
                UpdatePointCloudRendering();
                mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
            }
        }
        return true;
    }

    void PointShopApp::WindowFocusChanged(Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
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
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
    }

    void PointShopApp::DoCommand(bool isSubThread)
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
            case MagicApp::PointShopApp::NONE:
                break;
            case MagicApp::PointShopApp::EXPORT:
                ExportPointCloud(false);
                break;
            case MagicApp::PointShopApp::NORMALCALCULATION:
                CalculatePointCloudNormal(mIsDepthImage, false);
                break;
            case MagicApp::PointShopApp::NORMALSMOOTH:
                SmoothPointCloudNormal(false);
                break;
            case MagicApp::PointShopApp::OUTLIER:
                RemovePointCloudOutlier(false);
                break;
            case MagicApp::PointShopApp::ISOLATE:
                RemoveIsolatePart(false);
                break;
            case MagicApp::PointShopApp::COORDSMOOTH:
                SmoothPointCloudByNormal(false);
                break;
            case MagicApp::PointShopApp::RECONSTRUCTION:
                ReconstructMesh(false);
                return;
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
        }
    }

    bool PointShopApp::ImportPointCloud()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "��ȴ���ǰ����ִ����", "��ܰ��ʾ", MB_OK);
            return false;
        }
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloud);
                mpPointCloud = pointCloud;
                InfoLog << "Import Point Cloud: " << mpPointCloud->GetPointCount() << " points" << std::endl;
                InitViewTool();
                UpdatePointCloudRendering();
                mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
                return true;
            }
            else
            {
                mpUI->SetPointCloudInfo(0);
                MessageBox(NULL, "���Ƶ���ʧ��, Ŀǰ֧�ֵ����ʽ��asc��obj", "��ܰ��ʾ", MB_OK);
            }
        }
        return false;
    }

    void PointShopApp::ExportPointCloud(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        std::string fileName;
        char filterName[] = "Support format(*.obj, *.ply, *.asc)\0*.*\0";
        if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
        {
            mpPointCloud->UnifyCoords(1.0 / mScaleValue, mObjCenterCoord * (-mScaleValue));
            GPP::ErrorCode res = GPP::Parser::ExportPointCloud(fileName, mpPointCloud);
            mpPointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "��������ʧ��", "��ܰ��ʾ", MB_OK);
            }
        }
    }

    void PointShopApp::SmoothPointCloudNormal(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "���ȸ����Ƽ��㷨����", "��ܰ��ʾ", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = NORMALSMOOTH;
            DoCommand(true);
        }
        else
        {
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::SmoothNormal(mpPointCloud, 1.0);
            mIsCommandInProgress = false;
            mUpdatePointCloudRendering = true;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "���Ʒ��߹⻬ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
        }
    }

    void PointShopApp::SmoothPointCloudByNormal(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "���ȸ����Ƽ��㷨����", "��ܰ��ʾ", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = COORDSMOOTH;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::SmoothNormal(mpPointCloud);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "��������⻬ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
            mIsCommandInProgress = true;
            res = GPP::ConsolidatePointCloud::SmoothGeometryByNormal(mpPointCloud);
            mIsCommandInProgress = false;
            mUpdatePointCloudRendering = true;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "��������⻬ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
        }
    }

    void PointShopApp::RemovePointCloudOutlier(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = OUTLIER;
            DoCommand(true);
        }
        else
        {
            GPP::Int pointCount = mpPointCloud->GetPointCount();
            if (pointCount < 1)
            {
                MessageBox(NULL, "���Ƶĵ����С��1������ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
            std::vector<GPP::Real> uniformity;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateUniformity(mpPointCloud, &uniformity);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "����ȥ���ɵ�ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
            GPP::Real cutValue = 0.8;
            std::vector<GPP::Int> deleteIndex;
            for (GPP::Int pid = 0; pid < pointCount; pid++)
            {
                if (uniformity[pid] > cutValue)
                {
                    deleteIndex.push_back(pid);
                }
            }
            res = DeletePointCloudElements(mpPointCloud, deleteIndex);
            if (res != GPP_NO_ERROR)
            {
                return;
            }
            mUpdatePointCloudRendering = true;
            mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
        }
    }

    void PointShopApp::RemoveIsolatePart(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "���ȸ����Ƽ��㷨����", "��ܰ��ʾ", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = ISOLATE;
            DoCommand(true);
        }
        else
        {
            GPP::Int pointCount = mpPointCloud->GetPointCount();
            if (pointCount < 1)
            {
                MessageBox(NULL, "���Ƶĵ����С��1������ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
            std::vector<GPP::Real> isolation;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateIsolation(mpPointCloud, &isolation);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "����ȥ��������ʧ��", "��ܰ��ʾ", MB_OK);
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
                return;
            }
            mUpdatePointCloudRendering = true;
            mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
        }
    }

    void PointShopApp::SamplePointCloud(int targetPointCount)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::Int* sampleIndex = new GPP::Int[targetPointCount];
        GPP::ErrorCode res = GPP::SamplePointCloud::UniformSample(mpPointCloud, targetPointCount, sampleIndex, 0);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
            MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
        }
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "���Ʋ���ʧ��", "��ܰ��ʾ", MB_OK);
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
        mUpdatePointCloudRendering = true;
        GPPFREEARRAY(sampleIndex);
        mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
    }

    void PointShopApp::CalculatePointCloudNormal(bool isDepthImage, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = NORMALCALCULATION;
            mIsDepthImage = isDepthImage;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloud, isDepthImage);
            mIsCommandInProgress = false;
            mUpdatePointCloudRendering = true;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "���Ʒ��߼���ʧ��", "��ܰ��ʾ", MB_OK);
                return;
            }
        }
    }

    void PointShopApp::FlipPointCloudNormal()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "���ȸ����Ƽ��㷨����", "��ܰ��ʾ", MB_OK);
            return;
        }
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloud->SetPointNormal(pid, mpPointCloud->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudRendering();
    }

    void PointShopApp::ReconstructMesh(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "���ȸ����Ƽ��㷨����", "��ܰ��ʾ", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = RECONSTRUCTION;
            DoCommand(true);
        }
        else
        {
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
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ReconstructMesh::Reconstruct(mpPointCloud, triMesh, GPP::RECONSTRUCT_QUALITY_MEDIUM, false, &pointColorFields, &vertexColorField);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MagicCore::ToolKit::Get()->SetAppRunning(false);
                MessageBox(NULL, "GeometryPlusPlus API���ڣ���������ر�", "��ܰ��ʾ", MB_OK);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "�������ǻ�ʧ��", "��ܰ��ʾ", MB_OK);
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
            mpTriMesh = triMesh;
        }
    }

    void PointShopApp::SetPointCloud(GPP::PointCloud* pointCloud, GPP::Vector3 objCenterCoord, GPP::Real scaleValue)
    {
        if (!mpPointCloud)
        {
            delete mpPointCloud;
        }
        mpPointCloud = pointCloud;
        mObjCenterCoord = objCenterCoord;
        mScaleValue = scaleValue;
        InitViewTool();
        UpdatePointCloudRendering();
        if (mpPointCloud)
        {
            mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
        }
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

#if DEBUGDUMPFILE
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
                meshShop->SetMesh(triMesh, mObjCenterCoord, mScaleValue);
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
#endif

    bool PointShopApp::IsCommandInProgress(void)
    {
        return mIsCommandInProgress;
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

    bool PointShopApp::IsCommandAvaliable()
    {
        if (mpPointCloud == NULL)
        {
            MessageBox(NULL, "���ȵ������", "��ܰ��ʾ", MB_OK);
            return false;
        }
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "��ȴ���ǰ����ִ����", "��ܰ��ʾ", MB_OK);
            return false;
        }
        return true;
    }

    void PointShopApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }
}
