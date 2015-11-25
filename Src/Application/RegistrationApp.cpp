#include "stdafx.h"
#include "RegistrationApp.h"
#include "RegistrationAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "PointShopApp.h"
#include "AppManager.h"
#include "GPPDefines.h"
#include "PointCloud.h"
#include "Parser.h"
#include "ToolPointCloud.h"
#include "DumpInfo.h"
#include "ConsolidatePointCloud.h"
#include "RegistratePointCloud.h"
#include "DumpRegistratePointCloud.h"
#include "FusePointCloud.h"
#include <algorithm>

namespace MagicApp
{
    RegistrationApp::RegistrationApp() :
        mpUI(NULL),
        mMouseMode(MM_VIEW),
        mpViewTool(NULL),
        mpPickTool(NULL),
        mpDumpInfo(NULL),
        mpPointCloudRef(NULL),
        mpPointCloudFrom(NULL),
        mpFusePointCloud(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mPickedPointsRef(),
        mPickedPointsFrom()
    {
    }

    RegistrationApp::~RegistrationApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
        GPPFREEPOINTER(mpDumpInfo);
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpFusePointCloud);
    }

    bool RegistrationApp::Enter(void)
    {
        InfoLog << "Enter RegistrationApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new RegistrationAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool RegistrationApp::Update(float timeElapsed)
    {
        return true;
    }

    bool RegistrationApp::Exit(void)
    {
        InfoLog << "Exit RegistrationApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool RegistrationApp::MouseMoved( const OIS::MouseEvent &arg )
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

    bool RegistrationApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mMouseMode == MM_VIEW && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if ((mMouseMode == MM_PICK_POINT_REF || mMouseMode == MM_PICK_POINT_FROM) && mpPickTool != NULL)
        {
            mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool RegistrationApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((mMouseMode == MM_PICK_POINT_REF || mMouseMode == MM_PICK_POINT_FROM) && mpPickTool != NULL)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickPointId();
            mpPickTool->ClearPickedIds();
            if (pickedId == -1)
            {
                return true;
            }
            if (mMouseMode == MM_PICK_POINT_REF)
            {
                mPickedPointsRef.push_back(mpPointCloudRef->GetPointCoord(pickedId));
                UpdatePointPairsRendering();
            }
            else if (mMouseMode == MM_PICK_POINT_FROM)
            {
                mPickedPointsFrom.push_back(mpPointCloudFrom->GetPointCoord(pickedId));
                UpdatePointPairsRendering();
            }
        }
        return true;
    }

    bool RegistrationApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_D)
        {
            RunDumpInfo();
        }
        return true;
    }

    void RegistrationApp::SetupScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("RegistrationApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
    }

    void RegistrationApp::ShutdownScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("RegistrationApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PickedRefPoint_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PickedFromPoint_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PickedPointPairLines_RegistrationApp"); 
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        }
    }

    void RegistrationApp::ClearData(void)
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
        GPPFREEPOINTER(mpDumpInfo);
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpFusePointCloud);
    }

    bool RegistrationApp::ImportPointCloudRef()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloudRef);
                mpPointCloudRef = pointCloud;
                InfoLog << "Import Point Cloud Ref: " << mpPointCloudRef->GetPointCount() << " points" << std::endl;
                InitViewTool();
                UpdatePointCloudRefRendering();
                
                mPickedPointsRef.clear();
                mPickedPointsFrom.clear();
                UpdatePointPairsRendering();
                if (mpPointCloudFrom)
                {
                    GPPFREEPOINTER(mpPointCloudFrom);
                    MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
                }

                GPPFREEPOINTER(mpFusePointCloud);

                ModelView();
                return true;
            }
        }
        return false;
    }

    void RegistrationApp::CalculateRefNormal()
    {
        if (mpPointCloudRef == NULL)
        {
            return;
        }
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloudRef, 0);
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
        UpdatePointCloudRefRendering();
    }

    void RegistrationApp::FlipRefNormal()
    {
        if (mpPointCloudRef == NULL || mpPointCloudRef->HasNormal() == false)
        {
            return;
        }
        GPP::Int pointCount = mpPointCloudRef->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloudRef->SetPointNormal(pid, mpPointCloudRef->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudRefRendering();
    }

    void RegistrationApp::PushRef()
    {
        if (mpPointCloudRef == NULL || mpPointCloudRef->HasNormal() == false)
        {
            return;
        }
        mMouseMode = MM_PICK_POINT_REF;
        InitPickTool();
        mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, mpPointCloudRef, NULL);
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
        UpdatePointCloudRefRendering();
    }

    void RegistrationApp::PopRef()
    {
        mPickedPointsRef.pop_back();
        UpdatePointPairsRendering();
    }

    void RegistrationApp::FuseRef()
    {
        if (mpPointCloudRef == NULL || mpPointCloudFrom == NULL)
        {
            return;
        }
        GPP::ErrorCode res = GPP_NO_ERROR;
        if (mpFusePointCloud == NULL)
        {
            mpFusePointCloud = new GPP::FusePointCloud(1024, 1024, 1024, GPP::Vector3(-1.5, -1.5, -1.5), GPP::Vector3(1.5, 1.5, 1.5));
            res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudRef, NULL);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(mpFusePointCloud);
                return;
            }
        }
        res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudFrom, NULL);
        if (res != GPP_NO_ERROR)
        {
            GPPFREEPOINTER(mpFusePointCloud);
            return;
        }
        GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
        res = mpFusePointCloud->ExtractPointCloud(extractPointCloud);
        if (res != GPP_NO_ERROR)
        {
            GPPFREEPOINTER(extractPointCloud);
            GPPFREEPOINTER(mpFusePointCloud);
            return;
        }
        GPPFREEPOINTER(mpPointCloudRef);
        mpPointCloudRef = extractPointCloud;
        UpdatePointCloudRefRendering();

        mPickedPointsRef.clear();
        mPickedPointsFrom.clear();
        UpdatePointPairsRendering();

        GPPFREEPOINTER(mpPointCloudFrom);
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");

        ModelView();
    }

    bool RegistrationApp::ImportPointCloudFrom()
    {
        if (mpPointCloudRef == NULL)
        {
            InfoLog << "Please Import Ref Point Cloud First" << std::endl;
            return false;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloudFrom);
                mpPointCloudFrom = pointCloud;
                SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0.86, 0, 0));
                InfoLog << "Import Point Cloud From: " << mpPointCloudFrom->GetPointCount() << " points" << std::endl;
                InitViewTool();
                UpdatePointCloudFromRendering();

                mPickedPointsFrom.clear();
                UpdatePointPairsRendering();
                
                ModelView();
                return true;
            }
        }
        return false;
    }

    void RegistrationApp::CalculateFromNormal()
    {
        if (mpPointCloudFrom == NULL)
        {
            return;
        }
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloudFrom, 0);
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
        UpdatePointCloudFromRendering();
    }

    void RegistrationApp::FlipFromNormal(void)
    {
        if (mpPointCloudFrom == NULL || mpPointCloudFrom->HasNormal() == false)
        {
            return;
        }
        GPP::Int pointCount = mpPointCloudFrom->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloudFrom->SetPointNormal(pid, mpPointCloudFrom->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudFromRendering();
    }

    void RegistrationApp::PushFrom()
    {
        if (mpPointCloudFrom == NULL || mpPointCloudFrom->HasNormal() == false)
        {
            return;
        }
        mMouseMode = MM_PICK_POINT_FROM;
        InitPickTool();
        mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, mpPointCloudFrom, NULL);
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
        UpdatePointCloudFromRendering();
    }

    void RegistrationApp::PopFrom()
    {
        mPickedPointsFrom.pop_back();
        UpdatePointPairsRendering();
    }

    void RegistrationApp::AlignFrom()
    {
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            return;
        }
        std::vector<GPP::Vector3> pickedRefPointCoords;
        pickedRefPointCoords.reserve(mPickedPointsRef.size());
        for (std::vector<GPP::Vector3>::iterator itr = mPickedPointsRef.begin(); itr != mPickedPointsRef.end(); ++itr)
        {
            pickedRefPointCoords.push_back(*itr);
        }
        std::vector<GPP::Vector3> pickedFromPointCoords;
        pickedFromPointCoords.reserve(mPickedPointsFrom.size());
        for (std::vector<GPP::Vector3>::iterator itr = mPickedPointsFrom.begin(); itr != mPickedPointsFrom.end(); ++itr)
        {
            pickedFromPointCoords.push_back(*itr);
        }
        int pickedRefPointCount = pickedRefPointCoords.size();
        int pickedFromPointCount = pickedFromPointCoords.size();
        if (pickedRefPointCount < pickedFromPointCount)
        {
            pickedFromPointCoords.erase(pickedFromPointCoords.begin() + pickedRefPointCount, pickedFromPointCoords.end());
        }
        else if (pickedRefPointCount > pickedFromPointCount)
        {
            pickedRefPointCoords.erase(pickedRefPointCoords.begin() + pickedFromPointCount, pickedRefPointCoords.end());
        }
        GPP::Matrix4x4* initTransform = NULL;
        if (pickedRefPointCoords.size() > 0)
        {
            initTransform = new GPP::Matrix4x4;
            GPP::ErrorCode res = GPP::RegistratePointCloud::AlignByPointPairs(pickedRefPointCoords, pickedFromPointCoords, initTransform);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(initTransform);
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
                return;
            }
        }
        GPP::Matrix4x4 resultTransform;
        GPP::ErrorCode res = GPP::RegistratePointCloud::ICPRegistrate(mpPointCloudRef, mpPointCloudFrom, &resultTransform, initTransform);
        if (res != GPP_NO_ERROR)
        {
            GPPFREEPOINTER(initTransform);
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        if (initTransform != NULL)
        {
            resultTransform *= (*initTransform);
        }
        //Update mpPointCloudFrom
        GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
        for (GPP::Int pid = 0; pid < fromPointCount; pid++)
        {
            mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
            mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
        }
        //Update mark points
        for (std::vector<GPP::Vector3>::iterator itr = mPickedPointsFrom.begin(); itr != mPickedPointsFrom.end(); ++itr)
        {
            (*itr) = resultTransform.TransformPoint(*itr);
        }
        //
        UpdatePointCloudFromRendering();
        UpdatePointPairsRendering();

        mPickedPointsRef.clear();
        mPickedPointsFrom.clear();
        UpdatePointPairsRendering();

        ModelView();
    }

    void RegistrationApp::ModelView()
    {
        mMouseMode = MM_VIEW;
        UpdatePointCloudFromRendering();
        UpdatePointCloudRefRendering();
        UpdatePointPairsRendering();
    }

    void RegistrationApp::SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color)
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

    void RegistrationApp::EnterPointShop()
    {
        if (mpPointCloudRef == NULL)
        {
            return;
        }
        GPP::PointCloud* copiedPointCloud = GPP::CopyPointCloud(mpPointCloudRef);
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop)
        {
            pointShop->SetPointCloud(copiedPointCloud);
        }
        else
        {
            delete copiedPointCloud;
            copiedPointCloud = NULL;
        }
    }

    void RegistrationApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        if (dumpInfo == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpDumpInfo);
        mpDumpInfo = dumpInfo;
        if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTPAIR)
        {
            GPP::DumpPointCloudRegistrationAlignPointPair* dumpDetail = dynamic_cast<GPP::DumpPointCloudRegistrationAlignPointPair*>(mpDumpInfo);
            if (dumpDetail != NULL)
            {
                mPickedPointsRef = dumpDetail->GetPointsRef();
                mPickedPointsFrom = dumpDetail->GetPointsFrom();
                const GPP::Matrix4x4* initTransform = dumpDetail->GetInitTransform();
                if (initTransform != NULL)
                {
                    for (std::vector<GPP::Vector3>::iterator itr = mPickedPointsFrom.begin(); itr != mPickedPointsFrom.end(); ++itr)
                    {
                        (*itr) = initTransform->TransformPoint(*itr);
                    }
                }
                UpdatePointPairsRendering();
            }
        }
        else if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP)
        {
            if (mpDumpInfo->GetPointCloud(0) == NULL || mpDumpInfo->GetPointCloud(1) == NULL)
            {
                return;
            }
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = CopyPointCloud(mpDumpInfo->GetPointCloud(0));
            GPPFREEPOINTER(mpPointCloudFrom);
            mpPointCloudFrom = CopyPointCloud(mpDumpInfo->GetPointCloud(1));
            SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0.86, 0, 0));
            GPP::DumpPointCloudRegistrationICP* dumpDetail = dynamic_cast<GPP::DumpPointCloudRegistrationICP*>(mpDumpInfo);
            if (dumpDetail != NULL && dumpDetail->GetInitTransform() != NULL)
            {
                const GPP::Matrix4x4* initTransform = dumpDetail->GetInitTransform();
                GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
                for (GPP::Int pid = 0; pid < fromPointCount; pid++)
                {
                    mpPointCloudFrom->SetPointCoord(pid, initTransform->TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
                    mpPointCloudFrom->SetPointNormal(pid, initTransform->RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
                }
            }
            UpdatePointCloudRefRendering();
            UpdatePointCloudFromRendering();
        }
        else if (mpDumpInfo->GetApiName() == GPP::POINT_FUSION_UPDATE)
        {
            if (mpDumpInfo->GetPointCloud(1) == NULL)
            {
                return;
            }
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = mpDumpInfo->GetPointCloud(0);
            GPPFREEPOINTER(mpPointCloudFrom);
            mpPointCloudFrom = CopyPointCloud(mpDumpInfo->GetPointCloud(1));
            SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0.86, 0, 0));
            UpdatePointCloudRefRendering();
            UpdatePointCloudFromRendering();

            mPickedPointsRef.clear();
            mPickedPointsFrom.clear();
            UpdatePointPairsRendering();
        }

        InitViewTool();
    }

    void RegistrationApp::RunDumpInfo()
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
        if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTPAIR)
        {
            GPP::DumpPointCloudRegistrationAlignPointPair* dumpDetail = dynamic_cast<GPP::DumpPointCloudRegistrationAlignPointPair*>(mpDumpInfo);
            if (dumpDetail != NULL)
            {
                mPickedPointsFrom = dumpDetail->GetPointsFrom();
                UpdatePointPairsRendering();
            }
        }
        else if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP)
        {
            if (mpDumpInfo->GetPointCloud(1) == NULL)
            {
                return;
            }
            GPPFREEPOINTER(mpPointCloudFrom);
            mpPointCloudFrom = CopyPointCloud(mpDumpInfo->GetPointCloud(1));
            SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0.86, 0, 0));
            UpdatePointCloudFromRendering();
        }
        else if (mpDumpInfo->GetApiName() == GPP::POINT_FUSION_UPDATE)
        {
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = mpDumpInfo->GetPointCloud(0);
            UpdatePointCloudRefRendering();
            if (mpPointCloudFrom)
            {
                GPPFREEPOINTER(mpPointCloudFrom);
                MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
            }
        }
        else if (mpDumpInfo->GetApiName() == GPP::POINT_FUSION_EXTRACT)
        {
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = mpDumpInfo->GetPointCloud();
            UpdatePointCloudRefRendering();
            if (mpPointCloudFrom)
            {
                GPPFREEPOINTER(mpPointCloudFrom);
                MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
            }
        }
        GPPFREEPOINTER(mpDumpInfo);
    }

    void RegistrationApp::UpdatePointCloudFromRendering()
    {
        if (mpPointCloudFrom == NULL)
        {
            return;
        }
        if (mpPointCloudFrom->HasNormal())
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_RegistrationApp", "CookTorrancePoint", mpPointCloudFrom);
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_RegistrationApp", "SimplePoint", mpPointCloudFrom);
        }
    }

    void RegistrationApp::UpdatePointCloudRefRendering()
    {
        if (mpPointCloudRef == NULL)
        {
            return;
        }
        if (mpPointCloudRef->HasNormal())
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_RegistrationApp", "CookTorrancePoint", mpPointCloudRef);
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_RegistrationApp", "SimplePoint", mpPointCloudRef);
        }
    }

    void RegistrationApp::UpdatePointPairsRendering()
    {
        MagicCore::RenderSystem::Get()->RenderPointList("PickedRefPoint_RegistrationApp", "SimplePoint_Large", GPP::Vector3(0, 0, 1), mPickedPointsRef);
        MagicCore::RenderSystem::Get()->RenderPointList("PickedFromPoint_RegistrationApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), mPickedPointsFrom);
        MagicCore::RenderSystem::Get()->RenderLineSegments("PickedPointPairLines_RegistrationApp", "SimpleLine", mPickedPointsRef, mPickedPointsFrom);
    }

    void RegistrationApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    void RegistrationApp::InitPickTool()
    {
        if (mpPickTool == NULL)
        {
            mpPickTool = new MagicCore::PickTool;
        }
        mpPickTool->Reset();
    }
}
