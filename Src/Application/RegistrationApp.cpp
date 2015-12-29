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
#include "GPP.h"
#include "DumpRegistratePointCloud.h"
#include <algorithm>

namespace MagicApp
{
    RegistrationApp::RegistrationApp() :
        mpUI(NULL),
        mpViewTool(NULL),
        mpPickTool(NULL),
        mpDumpInfo(NULL),
        mpPointCloudRef(NULL),
        mpPointCloudFrom(NULL),
        mpFusePointCloud(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mMouseMode(MM_VIEW),
        mRefMarks(),
        mFromMarks()
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
        else if ((mMouseMode == MM_PICK_REF || mMouseMode == MM_PICK_FROM) && mpPickTool != NULL)
        {
            mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool RegistrationApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((mMouseMode == MM_PICK_REF || mMouseMode == MM_PICK_FROM) && mpPickTool != NULL)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickPointId();
            mpPickTool->ClearPickedIds();
            if (pickedId == -1)
            {
                return true;
            }
            if (mMouseMode == MM_PICK_REF)
            {
                mRefMarks.push_back(mpPointCloudRef->GetPointCoord(pickedId));
                UpdateMarkRefRendering();
            }
            else if (mMouseMode == MM_PICK_FROM)
            {
                mFromMarks.push_back(mpPointCloudFrom->GetPointCoord(pickedId));
                UpdateMarkFromRendering();
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
        MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_RegistrationApp");
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
        mRefMarks.clear();
        mFromMarks.clear();
    }

    bool RegistrationApp::ImportPointCloudRef()
    {
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0";
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
                
                if (mpPointCloudFrom)
                {
                    GPPFREEPOINTER(mpPointCloudFrom);
                    MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
                }
                GPPFREEPOINTER(mpFusePointCloud);

                mRefMarks.clear();
                UpdateMarkRefRendering();
                mFromMarks.clear();
                UpdateMarkFromRendering();

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
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloudRef, GPP::NORMAL_QUALITY_LOW);
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

    void RegistrationApp::SmoothRefNormal()
    {
        if (mpPointCloudRef == NULL || mpPointCloudRef->HasNormal() == false)
        {
            return;
        }
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::SmoothNormal(mpPointCloudRef, 1.0);
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

    void RegistrationApp::SwitchRefControlState()
    {
        if (mpPointCloudRef == NULL)
        {
            return;
        }
        if (mMouseMode != MM_PICK_REF)
        {
            mMouseMode = MM_PICK_REF;
            InitPickTool();
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, mpPointCloudRef, NULL);
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
            MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_RegistrationApp");
            UpdatePointCloudRefRendering();
            UpdateMarkRefRendering();
        }
        else
        {
            SwitchToViewMode();
        }
    }

    void RegistrationApp::DeleteRefMark()
    {
        if (mRefMarks.size() > 0)
        {
            mRefMarks.pop_back();
            UpdateMarkRefRendering();
        }
    }

    void RegistrationApp::ImportRefMark()
    {
        std::string fileName;
        char filterName[] = "Ref Files(*.ref)\0*.ref\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            std::ifstream fin(fileName.c_str());
            const int maxSize = 512;
            char pLine[maxSize];
            GPP::Vector3 markCoord;
            mRefMarks.clear();
            while (fin.getline(pLine, maxSize))
            {
                char* tok = strtok(pLine, " ");            
                for (int i = 0; i < 3; i++)
                {
                    markCoord[i] = (GPP::Real)atof(tok);
                    tok = strtok(NULL, " ");
                }
                mRefMarks.push_back((markCoord - mObjCenterCoord) * mScaleValue);
            }
            fin.close();
            UpdateMarkRefRendering();
        }
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
            mpFusePointCloud = new GPP::FusePointCloud(2048, 2048, 2048, GPP::Vector3(-3, -3, -3), GPP::Vector3(3, 3, 3));
            if (mpPointCloudRef->HasColor())
            {
                GPP::Int pointCountRef = mpPointCloudRef->GetPointCount();
                std::vector<GPP::Real> pointColorFieldsRef(pointCountRef * 3);
                for (GPP::Int pid = 0; pid < pointCountRef; pid++)
                {
                    GPP::Vector3 color = mpPointCloudRef->GetPointColor(pid);
                    GPP::Int baseId = pid * 3;
                    pointColorFieldsRef.at(baseId) = color[0];
                    pointColorFieldsRef.at(baseId + 1) = color[1];
                    pointColorFieldsRef.at(baseId + 2) = color[2];
                }
                res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudRef, NULL, &pointColorFieldsRef);
            }
            else
            {
                res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudRef, NULL, NULL);
            }         
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(mpFusePointCloud);
                return;
            }
        }
        GPP::Int pointCountFrom = mpPointCloudFrom->GetPointCount();
        if (mpPointCloudFrom->HasColor())
        {
            std::vector<GPP::Real> pointColorFieldsFrom(pointCountFrom * 3);
            for (GPP::Int pid = 0; pid < pointCountFrom; pid++)
            {
                GPP::Vector3 color = mpPointCloudFrom->GetPointColor(pid);
                GPP::Int baseId = pid * 3;
                pointColorFieldsFrom.at(baseId) = color[0];
                pointColorFieldsFrom.at(baseId + 1) = color[1];
                pointColorFieldsFrom.at(baseId + 2) = color[2];
            }
            res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudFrom, NULL, &pointColorFieldsFrom);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(mpFusePointCloud);
                return;
            }
            GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
            std::vector<GPP::Real> pointColorFieldsFused;
            res = mpFusePointCloud->ExtractPointCloud(extractPointCloud, &pointColorFieldsFused);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(extractPointCloud);
                GPPFREEPOINTER(mpFusePointCloud);
                return;
            }
            GPP::Int pointCountFused = extractPointCloud->GetPointCount();
            for (GPP::Int pid = 0; pid < pointCountFused; pid++)
            {
                GPP::Int baseIndex = pid * 3;
                extractPointCloud->SetPointColor(pid, GPP::Vector3(pointColorFieldsFused.at(baseIndex), pointColorFieldsFused.at(baseIndex + 1), pointColorFieldsFused.at(baseIndex + 2)));
            }
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = extractPointCloud;
        }
        else
        {
            res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudFrom, NULL, NULL);
            GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
            res = mpFusePointCloud->ExtractPointCloud(extractPointCloud, NULL);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(extractPointCloud);
                GPPFREEPOINTER(mpFusePointCloud);
                return;
            }
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = extractPointCloud;
        }
        UpdatePointCloudRefRendering();
        GPPFREEPOINTER(mpPointCloudFrom);
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
        //Update Ref Marks
        for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
        {
            mRefMarks.push_back(*markItr);
        }
        UpdateMarkRefRendering();
        mFromMarks.clear();
        UpdateMarkFromRendering();
    }

    bool RegistrationApp::ImportPointCloudFrom()
    {
        if (mpPointCloudRef == NULL)
        {
            InfoLog << "Please Import Ref Point Cloud First" << std::endl;
            return false;
        }
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloudFrom);
                mpPointCloudFrom = pointCloud;
                if (mpPointCloudFrom->HasColor() == false)
                {
                    SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0.86, 0, 0));
                }
                InfoLog << "Import Point Cloud From: " << mpPointCloudFrom->GetPointCount() << " points" << std::endl;
                InitViewTool();

                UpdatePointCloudFromRendering();
                mFromMarks.clear();
                UpdateMarkFromRendering();
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
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloudFrom, GPP::NORMAL_QUALITY_LOW);
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

    void RegistrationApp::SmoothFromNormal()
    {
        if (mpPointCloudFrom == NULL || mpPointCloudFrom->HasNormal() == false)
        {
            return;
        }
        GPP::ErrorCode res = GPP::ConsolidatePointCloud::SmoothNormal(mpPointCloudFrom, 1.0);
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

    void RegistrationApp::SwitchFromControlState()
    {
        if (mpPointCloudFrom == NULL)
        {
            return;
        }
        if (mMouseMode != MM_PICK_FROM)
        {
            mMouseMode = MM_PICK_FROM;
            InitPickTool();
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, mpPointCloudFrom, NULL);
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
            MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_RegistrationApp");
            UpdatePointCloudFromRendering();
            UpdateMarkFromRendering();
        }
        else
        {
            SwitchToViewMode();
        }
    }

    void RegistrationApp::SwitchToViewMode()
    {
        mMouseMode = MM_VIEW;
        UpdatePointCloudRefRendering();
        UpdatePointCloudFromRendering();
        UpdateMarkRefRendering();
        UpdateMarkFromRendering();
    }

    void RegistrationApp::DeleteFromMark()
    {
        if (mFromMarks.size() > 0)
        {
            mFromMarks.pop_back();
            UpdateMarkFromRendering();
        }
    }

    void RegistrationApp::ImportFromMark()
    {
        std::string fileName;
        char filterName[] = "Ref Files(*.ref)\0*.ref\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            std::ifstream fin(fileName.c_str());
            const int maxSize = 512;
            char pLine[maxSize];
            GPP::Vector3 markCoord;
            mFromMarks.clear();
            while (fin.getline(pLine, maxSize))
            {
                char* tok = strtok(pLine, " ");            
                for (int i = 0; i < 3; i++)
                {
                    markCoord[i] = (GPP::Real)atof(tok);
                    tok = strtok(NULL, " ");
                }
                mFromMarks.push_back((markCoord - mObjCenterCoord) * mScaleValue);
            }
            fin.close();
            UpdateMarkFromRendering();
        }
    }
        
    void RegistrationApp::AlignFast()
    {
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            return;
        }
        GPP::Matrix4x4 resultTransform;
        //GPP::DumpOnce();
        std::vector<GPP::Vector3>* marksRef = NULL;
        if (mRefMarks.size() > 0)
        {
            marksRef = &mRefMarks;
        }
        std::vector<GPP::Vector3>* marksFrom = NULL;
        if (mFromMarks.size() > 0)
        {
            marksFrom = &mFromMarks;
        }
        GPP::ErrorCode res = GPP::RegistratePointCloud::GlobalRegistrate(mpPointCloudRef, marksRef, mpPointCloudFrom, marksFrom, &resultTransform, GPP::REGISTRATE_QUALITY_LOW);
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        //Update mpPointCloudFrom
        GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
        for (GPP::Int pid = 0; pid < fromPointCount; pid++)
        {
            mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
            mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
        }     
        UpdatePointCloudFromRendering();
        //Update from marks
        for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
        {
            (*markItr) = resultTransform.TransformPoint(*markItr);
        }
        UpdateMarkFromRendering();
    }

    void RegistrationApp::AlignPrecise()
    {
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            return;
        }
        GPP::Matrix4x4 resultTransform;
        std::vector<GPP::Vector3>* marksRef = NULL;
        if (mRefMarks.size() > 0)
        {
            marksRef = &mRefMarks;
        }
        std::vector<GPP::Vector3>* marksFrom = NULL;
        if (mFromMarks.size() > 0)
        {
            marksFrom = &mFromMarks;
        }
        GPP::ErrorCode res = GPP::RegistratePointCloud::GlobalRegistrate(mpPointCloudRef, marksRef, mpPointCloudFrom, marksFrom, &resultTransform, GPP::REGISTRATE_QUALITY_HIGH);
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        //Update mpPointCloudFrom
        GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
        for (GPP::Int pid = 0; pid < fromPointCount; pid++)
        {
            mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
            mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
        }     
        UpdatePointCloudFromRendering();
        //Update from marks
        for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
        {
            (*markItr) = resultTransform.TransformPoint(*markItr);
        }
        UpdateMarkFromRendering();
    }

    void RegistrationApp::AlignICP()
    {
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            return;
        }
        GPP::Matrix4x4 resultTransform;
        GPP::ErrorCode res = GPP::RegistratePointCloud::ICPRegistrate(mpPointCloudRef, mpPointCloudFrom, &resultTransform);
        if (res != GPP_NO_ERROR)
        {
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
            return;
        }
        //Update mpPointCloudFrom
        GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
        for (GPP::Int pid = 0; pid < fromPointCount; pid++)
        {
            mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
            mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
        }     
        UpdatePointCloudFromRendering();
        //Update from marks
        for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
        {
            (*markItr) = resultTransform.TransformPoint(*markItr);
        }
        UpdateMarkFromRendering();
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
        if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP || mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_GLOBAL)
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
        if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP || mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_GLOBAL)
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

    void RegistrationApp::UpdateMarkRefRendering()
    {
        if (mRefMarks.empty())
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_RegistrationApp");
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointList("RefMarks_RegistrationApp", "SimplePoint_Large", GPP::Vector3(0, 1, 1), mRefMarks);
        }
    }

    void RegistrationApp::UpdateMarkFromRendering()
    {
        if (mFromMarks.empty())
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_RegistrationApp");
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointList("FromMarks_RegistrationApp", "SimplePoint_Large", GPP::Vector3(1, 1, 0), mFromMarks);
        }
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
