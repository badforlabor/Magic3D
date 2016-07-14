#include "stdafx.h"
#include <process.h>
#include "RegistrationApp.h"
#include "RegistrationAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "PointShopApp.h"
#include "AppManager.h"
#if DEBUGDUMPFILE
#include "DumpRegistratePointCloud.h"
#endif
#include <algorithm>
//#define DEBUGGLOBALREGISTRATION

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        RegistrationApp* app = (RegistrationApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    RegistrationApp::RegistrationApp() :
        mpUI(NULL),
        mpViewTool(NULL),
        mIsSeparateDisplay(false),
        mpPickToolRef(NULL),
        mpPickToolFrom(NULL),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mpPointCloudRef(NULL),
        mpPointCloudFrom(NULL),
        mpFusePointCloud(NULL),
        mpSumPointCloud(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mRefMarks(),
        mFromMarks(),
        mCommandType(NONE),
        mIsCommandInProgress(false),
        mUpdatePointRefRendering(false),
        mUpdatePointFromRendering(false),
        mUpdateMarkRefRendering(false),
        mUpdateMarkFromRendering(false),
        mUpdatePointCloudListRendering(false),
        mIsDepthImageRef(0),
        mIsDepthImageFrom(0),
        mPointCloudList(),
        mMarkList(),
        mGlobalRegistrateProgress(-1),
        mEnterPointShop(0),
        mUpdateUIInfo(0),
        mReversePatchNormalRef(0),
        mReversePatchNormalFrom(0),
        mMaxGlobalIterationCount(15),
        mSaveGlobalRegistrateResult(false),
        mIsSum(true),
        mMaxSampleTripleCount(0)
    {
    }

    RegistrationApp::~RegistrationApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickToolRef);
        GPPFREEPOINTER(mpPickToolFrom);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpFusePointCloud);
        for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
        {
            GPPFREEPOINTER(*itr);
        }
        mPointCloudList.clear();
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

    bool RegistrationApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            if (mGlobalRegistrateProgress < 0)
            {
                int progressValue = int(GPP::GetApiProgress() * 100.0);
                mpUI->SetProgressbar(progressValue);
            }
            else
            {
                int progressValue = int(mGlobalRegistrateProgress * 100.0);
                mpUI->SetProgressbar(progressValue);
            }
        }
        if (mUpdatePointRefRendering)
        {
            UpdatePointCloudRefRendering();
            mUpdatePointRefRendering = false;
        }
        if (mUpdatePointFromRendering)
        {
            UpdatePointCloudFromRendering();
            mUpdatePointFromRendering = false;
        }
        if (mUpdateMarkRefRendering)
        {
            UpdateMarkRefRendering();
            mUpdateMarkRefRendering = false;
        }
        if (mUpdateMarkFromRendering)
        {
            UpdateMarkFromRendering();
            mUpdateMarkFromRendering = false;
        }
        if (mUpdatePointCloudListRendering)
        {
            UpdatePointCloudListRendering();
            mUpdatePointCloudListRendering = false;
        }
        if (mEnterPointShop)
        {
            mEnterPointShop = false;
            EnterPointShop();
        }
        if (mUpdateUIInfo)
        {
            mUpdateUIInfo = false;
            if (mpPointCloudRef)
            {
                mpUI->SetRefPointInfo(mpPointCloudRef->GetPointCount());
            }
            else
            {
                mpUI->SetRefPointInfo(0);
            }
            if (mpPointCloudFrom)
            {
                if (mPointCloudList.empty())
                {
                    mpUI->SetFromPointInfo(mpPointCloudFrom->GetPointCount(), 2);
                }
                else
                {
                    mpUI->SetFromPointInfo(mpPointCloudFrom->GetPointCount(), mPointCloudList.size() + 1);
                }
            }
            else
            {
                mpUI->SetFromPointInfo(0, 0);
            }
        }
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
        if (arg.state.buttonDown(OIS::MB_Middle) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_MIDDLE_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && arg.state.buttonDown(OIS::MB_Right) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_RIGHT_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_LEFT_DOWN);
        }     
        return true;
    }

    bool RegistrationApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (!arg.state.buttonDown(OIS::MB_Left) && id == OIS::MB_Right && mIsCommandInProgress == false && (mpPickToolRef || mpPickToolFrom))
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

    bool RegistrationApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {        
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        if (!arg.state.buttonDown(OIS::MB_Left) && (mpPickToolRef || mpPickToolFrom) && mIsCommandInProgress == false && id == OIS::MB_Right)
        {
            if (mpPickToolRef)
            {
                mpPickToolRef->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
                GPP::Int pickedIdRef = mpPickToolRef->GetPickPointId();
                mpPickToolRef->ClearPickedIds();
                if (pickedIdRef != -1)
                {     
                    if (mReversePatchNormalRef)
                    {
                        GPP::ErrorCode res = GPP::ConsolidatePointCloud::ReversePatchNormal(mpPointCloudRef, pickedIdRef);
                        if (res == GPP_API_IS_NOT_AVAILABLE)
                        {
                            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                            MagicCore::ToolKit::Get()->SetAppRunning(false);
                        }
                        if (res != GPP_NO_ERROR)
                        {
                            MessageBox(NULL, "点云法线方向修复失败", "温馨提示", MB_OK);
                            return true;
                        }
                        UpdatePointCloudRefRendering();
                    }
                    else
                    {
                        mRefMarks.push_back(mpPointCloudRef->GetPointCoord(pickedIdRef));
                        UpdateMarkRefRendering();
                    }
                }
            }
            if (mpPickToolFrom)
            {
                mpPickToolFrom->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
                GPP::Int pickedIdFrom = mpPickToolFrom->GetPickPointId();
                mpPickToolFrom->ClearPickedIds();
                if (pickedIdFrom != -1)
                {
                    if (mReversePatchNormalFrom)
                    {
                        GPP::ErrorCode res = GPP::ConsolidatePointCloud::ReversePatchNormal(mpPointCloudFrom, pickedIdFrom);
                        if (res == GPP_API_IS_NOT_AVAILABLE)
                        {
                            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                            MagicCore::ToolKit::Get()->SetAppRunning(false);
                        }
                        if (res != GPP_NO_ERROR)
                        {
                            MessageBox(NULL, "点云法线方向修复失败", "温馨提示", MB_OK);
                            return true;
                        }
                        UpdatePointCloudFromRendering();
                    }
                    else
                    {
                        mFromMarks.push_back(mpPointCloudFrom->GetPointCoord(pickedIdFrom));
                        UpdateMarkFromRendering();
                    }
                }
            }
        }
        return true;
    }

    bool RegistrationApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_D)
        {
#if DEBUGDUMPFILE
            RunDumpInfo();
#endif
        }
        else if (arg.key == OIS::KC_R)
        {
            UpdatePointCloudListRendering();
        }
        else if (arg.key == OIS::KC_U)
        {
            mUpdatePointCloudListRendering = true;
        }
        else if (arg.key == OIS::KC_S)
        {
            mSaveGlobalRegistrateResult = true;
        }
        else if (arg.key == OIS::KC_I)
        {
            ResetGlobalRegistrationData();
            ImportPointCloudList();
            UpdatePointCloudListRendering();
        }

        return true;
    }

    void RegistrationApp::WindowFocusChanged(Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
    }

    void RegistrationApp::SetupScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("RegistrationApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
        MagicCore::RenderSystem::Get()->ResertAllSceneNode();
    }

    void RegistrationApp::ShutdownScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("RegistrationApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Right_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Left_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_Left_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_Right_RegistrationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudList_RegistrationApp");
        MagicCore::RenderSystem::Get()->ResertAllSceneNode();
    }

    void RegistrationApp::ClearData(void)
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickToolRef);
        GPPFREEPOINTER(mpPickToolFrom);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpFusePointCloud);
        GPPFREEPOINTER(mpSumPointCloud);
        mRefMarks.clear();
        mFromMarks.clear();
        mIsSeparateDisplay = false;
        ResetGlobalRegistrationData();
        mEnterPointShop = false;
        mUpdateUIInfo = false;
        mReversePatchNormalRef = false;
        mReversePatchNormalFrom = false;
        mSaveGlobalRegistrateResult = false;
    }

    void RegistrationApp::ResetGlobalRegistrationData()
    {
        for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
        {
            GPPFREEPOINTER(*itr);
        }
        mPointCloudList.clear();
        mMarkList.clear();
        mGlobalRegistrateProgress = -1;
    }

    void RegistrationApp::ClearPairwiseRegistrationData()
    {
        GPPFREEPOINTER(mpPickToolRef);
        GPPFREEPOINTER(mpPickToolFrom);
        GPPFREEPOINTER(mpPointCloudRef);
        GPPFREEPOINTER(mpPointCloudFrom);
        GPPFREEPOINTER(mpFusePointCloud);
        GPPFREEPOINTER(mpSumPointCloud);
        mRefMarks.clear();
        mFromMarks.clear();
        mIsSeparateDisplay = false;
        UpdatePointCloudRefRendering();
        UpdatePointCloudFromRendering();
        UpdateMarkRefRendering();
        UpdateMarkFromRendering();
    }

    void RegistrationApp::DoCommand(bool isSubThread)
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
            case MagicApp::RegistrationApp::NONE:
                break;
            case MagicApp::RegistrationApp::ALIGN_MARK:
                AlignMark(false);
                break;
            case MagicApp::RegistrationApp::ALIGN_FREE:
                AlignFree(mMaxSampleTripleCount, false);
                break;
            case MagicApp::RegistrationApp::ALIGN_ICP:
                AlignICP(false);
                break;
            case MagicApp::RegistrationApp::NORMAL_REF:
                CalculateRefNormal(mIsDepthImageRef, false);
                break;
            case MagicApp::RegistrationApp::NORMAL_FROM:
                CalculateFromNormal(mIsDepthImageFrom, false);
                break;
            case MagicApp::RegistrationApp::OUTLIER_REF:
                RemoveOutlierRef(false);
                break;
            case MagicApp::RegistrationApp::OUTLIER_FROM:
                RemoveOutlierFrom(false);
                break;
            case MagicApp::RegistrationApp::GLOBAL_REGISTRATE:
                GlobalRegistrate(mMaxGlobalIterationCount, mIsSum, false);
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
        }
    }

    bool RegistrationApp::ImportPointCloudRef()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0PLY Files(*.ply)\0*.ply\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloudRef);
                mpPointCloudRef = pointCloud;
                //InfoLog << "Import Point Cloud Ref: " << mpPointCloudRef->GetPointCount() << " points" << std::endl;
                mUpdateUIInfo = true;
                if (mpPointCloudRef->HasColor() == false)
                {
                    SetPointCloudColor(mpPointCloudRef, GPP::Vector3(0.86, 0, 0));
                }
                InitViewTool();
                SetSeparateDisplay(false);
                UpdatePointCloudRefRendering();
                
                GPPFREEPOINTER(mpPointCloudFrom);
                UpdatePointCloudFromRendering();
                GPPFREEPOINTER(mpFusePointCloud);
                GPPFREEPOINTER(mpSumPointCloud);

                mRefMarks.clear();
                UpdateMarkRefRendering();
                mFromMarks.clear();
                UpdateMarkFromRendering();

                ResetGlobalRegistrationData();

                // set up pick tool
                GPPFREEPOINTER(mpPickToolRef);
                GPPFREEPOINTER(mpPickToolFrom);
                mpPickToolRef = new MagicCore::PickTool;
                mpPickToolRef->SetPickParameter(MagicCore::PM_POINT, false, mpPointCloudRef, NULL, "ModelNode");

                mReversePatchNormalRef = false;
                mReversePatchNormalFrom = false;

                return true;
            }
            else
            {
                MessageBox(NULL, "点云导入失败, 目前支持导入格式：asc，obj", "温馨提示", MB_OK);
            }
        }
        return false;
    }

    void RegistrationApp::CalculateRefNormal(bool isDepthImage, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = NORMAL_REF;
            mIsDepthImageRef = isDepthImage;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            int neighborCount = 9;
            if (isDepthImage)
            {
                neighborCount = 5;
            }
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloudRef, isDepthImage, neighborCount);
            mIsCommandInProgress = false;
            mUpdatePointRefRendering = true;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云法线计算失败", "温馨提示", MB_OK);
                return;
            }
        }
    }

    void RegistrationApp::FlipRefNormal()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云", "温馨提示", MB_OK);
            return;
        }
        else if (mpPointCloudRef->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        GPP::Int pointCount = mpPointCloudRef->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloudRef->SetPointNormal(pid, mpPointCloudRef->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudRefRendering();
    }

    void RegistrationApp::ReversePatchNormalRef()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloudRef->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        mReversePatchNormalRef = !mReversePatchNormalRef;
        if (mReversePatchNormalRef)
        {
            MessageBox(NULL, "鼠标右键进入点云法线修复模式，点击点云反转法线方向，再次点击本按钮返回拾取模式", "温馨提示", MB_OK);
        }
    }

    void RegistrationApp::RemoveOutlierRef(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = OUTLIER_REF;
            DoCommand(true);
        }
        else
        {
            GPP::Int pointCount = mpPointCloudRef->GetPointCount();
            std::vector<GPP::Real> uniformity;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateUniformity(mpPointCloudRef, &uniformity);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                mIsCommandInProgress = false;
                MessageBox(NULL, "点云去除飞点失败", "温馨提示", MB_OK);
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
            res = DeletePointCloudElements(mpPointCloudRef, deleteIndex);
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                return;
            }
            mUpdatePointRefRendering = true;
            mUpdateUIInfo = true;
        }
    }

    void RegistrationApp::DeleteRefMark()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mRefMarks.size() > 0)
        {
            mRefMarks.pop_back();
            UpdateMarkRefRendering();
        }
    }

    void RegistrationApp::ImportRefMark()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        MessageBox(NULL, "点云标记点格式为：\nMark0_X Mark0_Y Mark0_Z\nMark1_X Mark1_Y Mark1_Z\n... ... ...", "温馨提示", MB_OK);
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

    void RegistrationApp::FuseRef(bool isSum)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudRef == NULL || mpPointCloudFrom == NULL)
        {
            MessageBox(NULL, "请先导入参考点云和需要对齐的点云", "温馨提示", MB_OK);
            return;
        }
        if ((isSum && mpFusePointCloud != NULL) || (!isSum && mpSumPointCloud != NULL))
        {
            MessageBox(NULL, "融合和去重叠不能混合使用", "温馨提示", MB_OK);
            return;
        }

        GPP::Real markTol = 6.0 / 3072.0;
        GPP::ErrorCode res = GPP_NO_ERROR;
        if ((!isSum && mpFusePointCloud == NULL) || (isSum && mpSumPointCloud == NULL))
        {
            if (isSum)
            {
                GPP::PointCloudPointList pointList(mpPointCloudRef);
                double density = 0;
                GPP::ErrorCode res = GPP::CalculatePointListDensity(&pointList, 4, density);
                InfoLog << "pointlist density = " << density << std::endl;
                mpSumPointCloud = new GPP::SumPointCloud(density, GPP::Vector3(-3, -3, -3), 
                    GPP::Vector3(3, 3, 3), mpPointCloudRef->HasNormal(), 0, 0);
            }
            else
            {
                mpFusePointCloud = new GPP::FusePointCloud(3072, 3072, 3072, GPP::Vector3(-3, -3, -3), 
                    GPP::Vector3(3, 3, 3), mpPointCloudRef->HasNormal());
            }            
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
                if (isSum)
                {
                    res = mpSumPointCloud->UpdateSumFunction(mpPointCloudRef, NULL, &pointColorFieldsRef);
                }
                else
                {
                    res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudRef, NULL, &pointColorFieldsRef);
                }
            }
            else
            {
                if (isSum)
                {
                    res = mpSumPointCloud->UpdateSumFunction(mpPointCloudRef, NULL, NULL);
                }
                else
                {
                    res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudRef, NULL, NULL);
                }
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(mpFusePointCloud);
                GPPFREEPOINTER(mpSumPointCloud);
                return;
            }

            GPP::PointCloud* pointCloudRefAligned = GPP::CopyPointCloud(mpPointCloudRef);
            mPointCloudList.push_back(pointCloudRefAligned);
            mMarkList.push_back(mRefMarks);
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
            if (isSum)
            {
                res = mpSumPointCloud->UpdateSumFunction(mpPointCloudFrom, NULL, &pointColorFieldsFrom);
            }
            else
            {
                res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudFrom, NULL, &pointColorFieldsFrom);
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                return;
            }
            GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
            std::vector<GPP::Real> pointColorFieldsFused;
            if (isSum)
            {
                res = mpSumPointCloud->ExtractPointCloud(extractPointCloud, &pointColorFieldsFused);
                /*std::vector<GPP::Real> fieldsMin;
                fieldsMin.insert(fieldsMin.end(), 3, 0);
                std::vector<GPP::Real> fieldsMax;
                fieldsMax.insert(fieldsMax.end(), 3, 1);
                res = mpSumPointCloud->ExtractPointCloudWithFusion(extractPointCloud, &pointColorFieldsFused, 0, 6, &fieldsMin, &fieldsMax);*/
            }
            else
            {
                res = mpFusePointCloud->ExtractPointCloud(extractPointCloud, &pointColorFieldsFused);
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(extractPointCloud);
                MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                return;
            }
            GPP::Int pointCountFused = extractPointCloud->GetPointCount();
            for (GPP::Int pid = 0; pid < pointCountFused; pid++)
            {
                GPP::Int baseIndex = pid * 3;
                extractPointCloud->SetPointColor(pid, GPP::Vector3(pointColorFieldsFused.at(baseIndex), pointColorFieldsFused.at(baseIndex + 1), pointColorFieldsFused.at(baseIndex + 2)));
            }
            extractPointCloud->SetHasColor(true);
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = extractPointCloud;
        }
        else
        {
            if (isSum)
            {
                res = mpSumPointCloud->UpdateSumFunction(mpPointCloudFrom, NULL, NULL);
            }
            else
            {
                res = mpFusePointCloud->UpdateFuseFunction(mpPointCloudFrom, NULL, NULL);
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                return;
            }
            GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
            if (isSum)
            {
                res = mpSumPointCloud->ExtractPointCloud(extractPointCloud, NULL);
            }
            else
            {
                res = mpFusePointCloud->ExtractPointCloud(extractPointCloud, NULL);
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                GPPFREEPOINTER(extractPointCloud);
                MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                return;
            }
            extractPointCloud->SetHasNormal(mpPointCloudRef->HasNormal());
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = extractPointCloud;
        }
        GPP::PointCloud* pointCloudFromAligned = GPP::CopyPointCloud(mpPointCloudFrom);
        mPointCloudList.push_back(pointCloudFromAligned);
        mMarkList.push_back(mFromMarks);

        if (mpPointCloudRef && mpPointCloudRef->HasColor() == false)
        {
            SetPointCloudColor(mpPointCloudRef, GPP::Vector3(0.86, 0, 0));
        }
        GPPFREEPOINTER(mpPointCloudFrom);
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
        //Update pick tool
        GPPFREEPOINTER(mpPickToolFrom);
        mpPickToolRef->Reset();
        mpPickToolRef->SetPickParameter(MagicCore::PM_POINT, false, mpPointCloudRef, NULL, "ModelNode");
        //Update Ref Marks
        for (std::vector<GPP::Vector3>::iterator fromItr = mFromMarks.begin(); fromItr != mFromMarks.end(); ++fromItr)
        {
            bool merged = false;
            for (std::vector<GPP::Vector3>::iterator refItr = mRefMarks.begin(); refItr != mRefMarks.end(); ++refItr)
            {
                GPP::Real markDist = ((*refItr) - (*fromItr)).Length();
                if (markDist < markTol)
                {
                    (*refItr) = ((*refItr) + (*fromItr)) / 2.0;
                    merged = true;
                    //GPPDebug << "RegistrationApp::FuseRef: Merged one mark: " << markDist << " / " << markTol << std::endl;
                    break;
                }
            }
            if (!merged)
            {
                mRefMarks.push_back(*fromItr);
            }
        }
        mFromMarks.clear();
        SetSeparateDisplay(false);
        mUpdatePointRefRendering = true;
        mUpdatePointFromRendering = true;
        mUpdateMarkRefRendering = true;
        mUpdateMarkFromRendering = true;
        mUpdateUIInfo = true;
    }

#ifdef DEBUGGLOBALREGISTRATION
    void RegistrationApp::GlobalRegistrate(int maxIterationCount, bool isSum, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mPointCloudList.empty())
        {
            ImportPointCloudList();
            if (mPointCloudList.size() > 0)
            {
                ClearPairwiseRegistrationData();
                GlobalRegistrate(maxIterationCount, isSum, sumInterval);
            }
            return;
        }
        if (isSubThread)
        {
            DebugLog << "Global Registrate App in Main Thread..." << std::endl;
            mCommandType = GLOBAL_REGISTRATE;
            mMaxGlobalIterationCount = maxIterationCount;
            mIsSum = isSum;
            DoCommand(true);
        }
        else
        {
            DebugLog << "Global Registrate App in SubThread..." << std::endl;
            std::vector<GPP::Matrix4x4> resultTransform;
            std::vector<GPP::IPointCloud*> pointCloudList;
            bool hasNormalInfo = true;
            for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
            {
                pointCloudList.push_back(*itr);
                if ((*itr)->HasNormal() == false)
                {
                    hasNormalInfo = false;
                }
            }
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::RegistratePointCloud::GlobalRegistrate(&pointCloudList, maxIterationCount, 
                &resultTransform, NULL, hasNormalInfo, 0);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "全局注册失败", "温馨提示", MB_OK);
                mIsCommandInProgress = false;
                return;
            }
            int pointListCount = mPointCloudList.size();
            for (int cid = 0; cid < pointListCount; cid++)
            {
                GPP::Int pointCount = mPointCloudList.at(cid)->GetPointCount();
                for (GPP::Int pid = 0; pid < pointCount; pid++)
                {
                    mPointCloudList.at(cid)->SetPointCoord(pid, resultTransform.at(cid).TransformPoint(mPointCloudList.at(cid)->GetPointCoord(pid)));
                    mPointCloudList.at(cid)->SetPointNormal(pid, resultTransform.at(cid).RotateVector(mPointCloudList.at(cid)->GetPointNormal(pid)));
                }
            }
            mIsCommandInProgress = false;
            mUpdateUIInfo = true;
            mUpdatePointCloudListRendering = true;
        }
    }
#else
    void RegistrationApp::GlobalRegistrate(int maxIterationCount, bool isSum, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mPointCloudList.empty())
        {
            ImportPointCloudList();
            if (mPointCloudList.size() > 0)
            {
                ClearPairwiseRegistrationData();
                GlobalRegistrate(maxIterationCount, isSum);
            }
            return;
        }
        if (isSubThread)
        {
            DebugLog << "Global Registrate App in Main Thread..." << std::endl;
            mCommandType = GLOBAL_REGISTRATE;
            mMaxGlobalIterationCount = maxIterationCount;
            mIsSum = isSum;
            DoCommand(true);
        }
        else
        {
            DebugLog << "Global Registrate App in SubThread..." << std::endl;
            std::vector<GPP::Matrix4x4> resultTransform;
            std::vector<GPP::IPointCloud*> pointCloudList;
            bool hasNormalInfo = true;
            for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
            {
                pointCloudList.push_back(*itr);
                if ((*itr)->HasNormal() == false)
                {
                    hasNormalInfo = false;
                }
            }
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::RegistratePointCloud::GlobalRegistrate(&pointCloudList, maxIterationCount, 
                &resultTransform, NULL, hasNormalInfo, 0);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "全局注册失败", "温馨提示", MB_OK);
                mIsCommandInProgress = false;
                return;
            }
            // Save result
            if (mSaveGlobalRegistrateResult)
            {
                int pointListCount = pointCloudList.size();
                for (int cloudid = 0; cloudid < pointListCount; cloudid++)
                {
                    GPP::PointCloud* transPointCloud = GPP::CopyPointCloud(mPointCloudList.at(cloudid));
                    int curPointCount = transPointCloud->GetPointCount();
                    for (int pid = 0; pid < curPointCount; pid++)
                    {
                        transPointCloud->SetPointCoord(pid, resultTransform.at(cloudid).TransformPoint(transPointCloud->GetPointCoord(pid)));
                    }
                    transPointCloud->UnifyCoords(1.0 / mScaleValue, mObjCenterCoord * (-mScaleValue));
                    std::stringstream ss;
                    ss << "res_" << cloudid << ".asc" ;
                    std::string fileName;
                    ss >> fileName;
                    GPP::Parser::ExportPointCloud(fileName, transPointCloud);
                    GPPFREEPOINTER(transPointCloud);
                }
            }
            //
            GPPFREEPOINTER(mpFusePointCloud);
            GPPFREEPOINTER(mpSumPointCloud);
            GPP::Vector3 bboxMin, bboxMax;
            if (mpPointCloudRef)
            {
                GPP::PointCloudPointList pointList(mpPointCloudRef);
                res = GPP::CalculatePointListBoundingBox(&pointList, bboxMin, bboxMax);
            }
            else
            {
                res = GPP::CalculatePointCloudListBoundingBox(pointCloudList, &resultTransform, bboxMin, bboxMax);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "包围盒计算失败", "温馨提示", MB_OK);
                return;
            }
            GPP::Vector3 deltaVector(0.25, 0.25, 0.25);
            bboxMin -= deltaVector;
            bboxMax += deltaVector;
            DebugLog << "Global registration app: bboxMin=" << bboxMin[0] << " " << bboxMin[1] << " " << bboxMin[2] << 
                " bboxMax=" << bboxMax[0] << " " << bboxMax[1] << " " << bboxMax[2] << std::endl;
            double gridSize = 6.0 / 3072;
            int resolutionX = (bboxMax[0] - bboxMin[0]) / gridSize;
            int resolutionY = (bboxMax[1] - bboxMin[1]) / gridSize;
            int resolutionZ = (bboxMax[2] - bboxMin[2]) / gridSize;
            DebugLog << "Global registration app: resolution " << resolutionX << " " << resolutionY << " " << resolutionZ << std::endl;
            if (isSum)
            {
                GPP::PointCloudPointList pointList(pointCloudList.at(0));
                double density = 0;
                GPP::ErrorCode res = GPP::CalculatePointListDensity(&pointList, 4, density);
                InfoLog << "pointlist density = " << density << std::endl;
                mpSumPointCloud = new GPP::SumPointCloud(density, bboxMin, bboxMax, hasNormalInfo, 25, 2);
            }
            else
            {
                mpFusePointCloud = new GPP::FusePointCloud(resolutionX, resolutionY, resolutionZ, bboxMin, bboxMax, hasNormalInfo);
            }

            int pointCloudCount = mPointCloudList.size();
            bool hasColorInfo = false;
            for (int cid = 0; cid < pointCloudCount; cid++)
            {
                mGlobalRegistrateProgress = double(cid) / double(pointCloudCount);
                GPP::PointCloud* pointCloudFrom = mPointCloudList.at(cid);
                GPP::Int pointCountFrom = pointCloudFrom->GetPointCount();
 
                if (pointCloudFrom->HasColor())
                {
                    hasColorInfo = true;
                    std::vector<GPP::Real> pointColorFieldsFrom(pointCountFrom * 3);
                    for (GPP::Int pid = 0; pid < pointCountFrom; pid++)
                    {
                        GPP::Vector3 color = pointCloudFrom->GetPointColor(pid);
                        GPP::Int baseId = pid * 3;
                        pointColorFieldsFrom.at(baseId) = color[0];
                        pointColorFieldsFrom.at(baseId + 1) = color[1];
                        pointColorFieldsFrom.at(baseId + 2) = color[2];
                    }
                    resultTransform.at(cid).Print();
                    if (isSum)
                    {
                        res = mpSumPointCloud->UpdateSumFunction(pointCloudFrom, &resultTransform.at(cid), &pointColorFieldsFrom);
                    }
                    else
                    {
                        res = mpFusePointCloud->UpdateFuseFunction(pointCloudFrom, &resultTransform.at(cid), &pointColorFieldsFrom);
                    }
                    if (res == GPP_API_IS_NOT_AVAILABLE)
                    {
                        MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                        MagicCore::ToolKit::Get()->SetAppRunning(false);
                    }
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                        mGlobalRegistrateProgress = -1;
                        mIsCommandInProgress = false;
                        return;
                    }
                }
                else
                {
                    if (isSum)
                    {
                        res = mpSumPointCloud->UpdateSumFunction(pointCloudFrom, &resultTransform.at(cid), NULL);
                    }
                    else
                    {
                        res = mpFusePointCloud->UpdateFuseFunction(pointCloudFrom, &resultTransform.at(cid), NULL);
                    }
                    if (res == GPP_API_IS_NOT_AVAILABLE)
                    {
                        MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                        MagicCore::ToolKit::Get()->SetAppRunning(false);
                    }
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                        mGlobalRegistrateProgress = -1;
                        mIsCommandInProgress = false;
                        return;
                    }
                }
            }
            std::vector<int> cloudIds;
            if (hasColorInfo)
            {
                GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
                std::vector<GPP::Real> pointColorFieldsFused;
                GPP::ErrorCode res = GPP_NO_ERROR;
                if (isSum)
                {
                    //res = mpSumPointCloud->ExtractPointCloud(extractPointCloud, &pointColorFieldsFused, &cloudIds);
                    std::vector<GPP::Real> fieldsMin;
                    fieldsMin.insert(fieldsMin.end(), 3, 0);
                    std::vector<GPP::Real> fieldsMax;
                    fieldsMax.insert(fieldsMax.end(), 3, 1);
                    res = mpSumPointCloud->ExtractPointCloudWithFusion(extractPointCloud, &pointColorFieldsFused, 0, 12, 
                        &fieldsMin, &fieldsMax, &cloudIds);
                }
                else
                {
                    res = mpFusePointCloud->ExtractPointCloud(extractPointCloud, &pointColorFieldsFused);
                }
                if (res == GPP_API_IS_NOT_AVAILABLE)
                {
                    MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                    MagicCore::ToolKit::Get()->SetAppRunning(false);
                }
                if (res != GPP_NO_ERROR)
                {
                    GPPFREEPOINTER(extractPointCloud);
                    MessageBox(NULL, "点云抽取失败", "温馨提示", MB_OK);
                    mGlobalRegistrateProgress = -1;
                    mIsCommandInProgress = false;
                    return;
                }
                GPP::Int pointCountFused = extractPointCloud->GetPointCount();
                for (GPP::Int pid = 0; pid < pointCountFused; pid++)
                {
                    GPP::Int baseIndex = pid * 3;
                    extractPointCloud->SetPointColor(pid, GPP::Vector3(pointColorFieldsFused.at(baseIndex), pointColorFieldsFused.at(baseIndex + 1), pointColorFieldsFused.at(baseIndex + 2)));
                }
                extractPointCloud->SetHasColor(true);
                GPPFREEPOINTER(mpPointCloudRef);
                mpPointCloudRef = extractPointCloud;

            }
            else
            {
                GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
                GPP::ErrorCode res = GPP_NO_ERROR;
                if (isSum)
                {
                    res = mpSumPointCloud->ExtractPointCloud(extractPointCloud, NULL, &cloudIds);
                }
                else
                {
                    res = mpFusePointCloud->ExtractPointCloud(extractPointCloud, NULL);
                }
                if (res == GPP_API_IS_NOT_AVAILABLE)
                {
                    MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                    MagicCore::ToolKit::Get()->SetAppRunning(false);
                }
                if (res != GPP_NO_ERROR)
                {
                    GPPFREEPOINTER(extractPointCloud);
                    MessageBox(NULL, "点云抽取失败", "温馨提示", MB_OK);
                    mGlobalRegistrateProgress = -1;
                    mIsCommandInProgress = false;
                    return;
                }
                GPPFREEPOINTER(mpPointCloudRef);
                mpPointCloudRef = extractPointCloud;
                SetPointCloudColor(mpPointCloudRef, GPP::Vector3(0.09, 0.48627, 0.69));
            }
            if (0)//cloudIds.size() > 0 && mpPointCloudRef->HasNormal())
            {
                /*std::ofstream cloudIdOut("cloudId.txt");
                cloudIdOut << cloudIds.size() << std::endl;
                for (std::vector<int>::iterator citr = cloudIds.begin(); citr != cloudIds.end(); ++citr)
                {
                    cloudIdOut << *citr << " ";
                }
                cloudIdOut.close();*/

                std::vector<GPP::Real> isolation;
                GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateIsolation(mpPointCloudRef, &isolation, 9, &cloudIds);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "点云孤立项计算失败", "温馨提示", MB_OK);
                    return;
                }
                int pointCount = mpPointCloudRef->GetPointCount();
                std::vector<GPP::Int> deleteIndex;
                double cutValue = 0.0001;
                for (GPP::Int pid = 0; pid < pointCount; pid++)
                {
                    if (isolation[pid] < cutValue)
                    {
                        deleteIndex.push_back(pid);
                    }
                }
                res = GPP::DeletePointCloudElements(mpPointCloudRef, deleteIndex);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "点云去噪点失败", "温馨提示", MB_OK);
                    return;
                }
            }
            GPPFREEPOINTER(mpFusePointCloud);
            mGlobalRegistrateProgress = -1.0;
            ResetGlobalRegistrationData();
            mRefMarks.clear();
            mUpdateMarkRefRendering = true;
            mUpdatePointRefRendering = true;
            mGlobalRegistrateProgress = -1;
            mIsCommandInProgress = false;
            mUpdateUIInfo = true;
            mEnterPointShop = true;
        }
    }
#endif

    bool RegistrationApp::ImportPointCloudFrom()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
        if (mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云", "温馨提示", MB_OK);
            return false;
        }
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0PLY Files(*.ply)\0*.ply\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloudFrom);
                mpPointCloudFrom = pointCloud;
                mpPointCloudFrom->SetHasColor(mpPointCloudRef->HasColor());
                if (mpPointCloudFrom->HasColor() == false)
                {
                    SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0, 0.86, 0));
                }
                //InfoLog << "Import Point Cloud From: " << mpPointCloudFrom->GetPointCount() << " points" << std::endl;
                mUpdateUIInfo = true;
                InitViewTool();
                if (!mIsSeparateDisplay)
                {
                    SetSeparateDisplay(true);
                    UpdatePointCloudRefRendering();
                    UpdateMarkRefRendering();
                }
                UpdatePointCloudFromRendering();
                mFromMarks.clear();
                UpdateMarkFromRendering();

                GPPFREEPOINTER(mpPickToolFrom);
                mpPickToolFrom = new MagicCore::PickTool;
                mpPickToolFrom->SetPickParameter(MagicCore::PM_POINT, false, mpPointCloudFrom, NULL, "ModelNodeRight");

                mReversePatchNormalFrom = false;

                return true;
            }
            else
            {
                MessageBox(NULL, "点云导入失败, 目前支持导入格式：asc，obj", "温馨提示", MB_OK);
            }
        }
        return false;
    }

    void RegistrationApp::CalculateFromNormal(bool isDepthImage, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudFrom == NULL)
        {
            MessageBox(NULL, "请先导入需要对齐的点云", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = NORMAL_FROM;
            mIsDepthImageFrom = isDepthImage;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            int neighborCount = 9;
            if (isDepthImage)
            {
                neighborCount = 5;
            }
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculatePointCloudNormal(mpPointCloudFrom, isDepthImage, neighborCount);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云法线计算失败", "温馨提示", MB_OK);
                return;
            }
            mUpdatePointFromRendering = true;
        }
    }

    void RegistrationApp::FlipFromNormal()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudFrom == NULL)
        {
            MessageBox(NULL, "请先导入需要对齐的点云", "温馨提示", MB_OK);
            return;
        }
        else if (mpPointCloudFrom->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        GPP::Int pointCount = mpPointCloudFrom->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloudFrom->SetPointNormal(pid, mpPointCloudFrom->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudFromRendering();
    }

    void RegistrationApp::ReversePatchNormalFrom()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloudFrom->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        mReversePatchNormalFrom = !mReversePatchNormalFrom;
        if (mReversePatchNormalFrom)
        {
            MessageBox(NULL, "鼠标右键进入点云法线修复模式，点击点云反转法线方向，再次点击本按钮返回拾取模式", "温馨提示", MB_OK);
        }
    }

    void RegistrationApp::RemoveOutlierFrom(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudFrom == NULL)
        {
            MessageBox(NULL, "请先导入参考点云", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = OUTLIER_FROM;
            DoCommand(true);
        }
        else
        {
            GPP::Int pointCount = mpPointCloudFrom->GetPointCount();
            std::vector<GPP::Real> uniformity;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateUniformity(mpPointCloudFrom, &uniformity);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                mIsCommandInProgress = false;
                MessageBox(NULL, "点云去除飞点失败", "温馨提示", MB_OK);
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
            res = DeletePointCloudElements(mpPointCloudFrom, deleteIndex);
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                return;
            }
            mUpdatePointFromRendering = true;
            mUpdateUIInfo = true;
        }
    }

    bool RegistrationApp::IsCommandInProgress()
    {
        return mIsCommandInProgress;
    }

    void RegistrationApp::SwitchSeparateDisplay()
    {
        mIsSeparateDisplay = !mIsSeparateDisplay;
        UpdatePointCloudRefRendering();
        UpdatePointCloudFromRendering();
        UpdateMarkRefRendering();
        UpdateMarkFromRendering();
        UpdatePickToolNodeName();
    }

    void RegistrationApp::SetSeparateDisplay(bool isSeparate)
    {
        mIsSeparateDisplay = isSeparate;
        UpdatePickToolNodeName();
    }

    void RegistrationApp::UpdatePickToolNodeName()
    {
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

    void RegistrationApp::DeleteFromMark()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mFromMarks.size() > 0)
        {
            mFromMarks.pop_back();
            UpdateMarkFromRendering();
        }
    }

    void RegistrationApp::ImportFromMark()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        MessageBox(NULL, "点云标记点格式为：\nMark0_X Mark0_Y Mark0_Z\nMark1_X Mark1_Y Mark1_Z\n... ... ...", "温馨提示", MB_OK);
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
        
    void RegistrationApp::AlignMark(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云和需要对齐的点云", "温馨提示", MB_OK);
            return;
        }
        else if (mpPointCloudFrom->HasNormal() == false || mpPointCloudRef->HasNormal() == false)
        {
            MessageBox(NULL, "请先给参考点云和需要对齐的点云计算法线", "温馨提示", MB_OK);
            return;
        }
        if (mRefMarks.size() < 3 || mFromMarks.size() < 3)
        {
            MessageBox(NULL, "带标记的拼接需要3个及以上的标记点", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = ALIGN_MARK;
            DoCommand(true);
        }
        else
        {
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
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::RegistratePointCloud::AlignPointCloudByMark(mpPointCloudRef, marksRef, mpPointCloudFrom, marksFrom, &resultTransform);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "快速对齐失败", "温馨提示", MB_OK);
                return;
            }
            //Update mpPointCloudFrom
            GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
            for (GPP::Int pid = 0; pid < fromPointCount; pid++)
            {
                mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
                mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
            }
            SetSeparateDisplay(false);
            //Update from marks
            for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
            {
                (*markItr) = resultTransform.TransformPoint(*markItr);
            }
            mUpdateMarkFromRendering = true;
            mUpdateMarkRefRendering = true;
            mUpdatePointRefRendering = true;
            mUpdatePointFromRendering = true;
        }
    }

    void RegistrationApp::AlignFree(int maxSampleTripleCount, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云和需要对齐的点云", "温馨提示", MB_OK);
            return;
        }
        else if (mpPointCloudFrom->HasNormal() == false || mpPointCloudRef->HasNormal() == false)
        {
            MessageBox(NULL, "请先给参考点云和需要对齐的点云计算法线", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = ALIGN_FREE;
            mMaxSampleTripleCount = maxSampleTripleCount;
            DoCommand(true);
        }
        else
        {
            GPP::Matrix4x4 resultTransform;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::RegistratePointCloud::AlignPointCloud(mpPointCloudRef, mpPointCloudFrom, &resultTransform, 
                maxSampleTripleCount);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "精细对齐失败", "温馨提示", MB_OK);
                return;
            }
            //Update mpPointCloudFrom
            GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
            for (GPP::Int pid = 0; pid < fromPointCount; pid++)
            {
                mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
                mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
            }
            SetSeparateDisplay(false);
            //Update from marks
            for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
            {
                (*markItr) = resultTransform.TransformPoint(*markItr);
            }
            mUpdateMarkRefRendering = true;
            mUpdateMarkFromRendering = true;
            mUpdatePointRefRendering = true;
            mUpdatePointFromRendering = true;
        }
    }

    void RegistrationApp::AlignICP(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudFrom == NULL || mpPointCloudRef == NULL)
        {
            MessageBox(NULL, "请先导入参考点云和需要对齐的点云", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = ALIGN_ICP;
            DoCommand(true);
        }
        else
        {
            if (mRefMarks.size() > 0 || mFromMarks.size() > 0)
            {
                if (MessageBox(NULL, "需要删除所有标记点吗", "温馨提示", MB_YESNO) == IDYES)
                {
                    mRefMarks.clear();
                    mFromMarks.clear();
                    mUpdateMarkRefRendering = true;
                    mUpdateMarkFromRendering = true;
                }
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
            mIsCommandInProgress = true;
            bool hasNormalInfo = false;
            if (mpPointCloudRef->HasNormal() && mpPointCloudFrom->HasNormal())
            {
                hasNormalInfo = true;
            }
            GPP::ErrorCode res = GPP::RegistratePointCloud::ICPRegistrate(mpPointCloudRef, marksRef, 
                mpPointCloudFrom, marksFrom, &resultTransform, NULL, hasNormalInfo);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "ICP对齐失败", "温馨提示", MB_OK);
                return;
            }
            //Update mpPointCloudFrom
            GPP::Int fromPointCount = mpPointCloudFrom->GetPointCount();
            for (GPP::Int pid = 0; pid < fromPointCount; pid++)
            {
                mpPointCloudFrom->SetPointCoord(pid, resultTransform.TransformPoint(mpPointCloudFrom->GetPointCoord(pid)));
                mpPointCloudFrom->SetPointNormal(pid, resultTransform.RotateVector(mpPointCloudFrom->GetPointNormal(pid)));
            }
            SetSeparateDisplay(false);
            //Update from marks
            for (std::vector<GPP::Vector3>::iterator markItr = mFromMarks.begin(); markItr != mFromMarks.end(); ++markItr)
            {
                (*markItr) = resultTransform.TransformPoint(*markItr);
            }
            mUpdateMarkFromRendering = true;
            mUpdatePointRefRendering = true;
            mUpdateMarkRefRendering = true;
            mUpdatePointFromRendering = true;
        }
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

    bool RegistrationApp::IsCommandAvaliable(void)
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        return true;
    }

    void RegistrationApp::EnterPointShop()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpPointCloudRef == NULL)
        {
            AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        }
        GPP::PointCloud* copiedPointCloud = GPP::CopyPointCloud(mpPointCloudRef);
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop)
        {
            pointShop->SetPointCloud(copiedPointCloud, mObjCenterCoord, mScaleValue);
            copiedPointCloud = NULL;
        }
        else
        {
            GPPFREEPOINTER(copiedPointCloud);
        }
    }

    void RegistrationApp::ImportPointCloudList()
    {
        std::vector<std::string> fileNames;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0PLY Files(*.ply)\0*.ply\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
        if (MagicCore::ToolKit::MultiFileOpenDlg(fileNames, filterName))
        {
            if (fileNames.size() > 1)
            {
                for (std::vector<GPP::PointCloud*>::iterator pitr = mPointCloudList.begin(); pitr != mPointCloudList.end(); ++pitr)
                {
                    GPPFREEPOINTER(*pitr);
                }
                mPointCloudList.clear();
                double colorDelta = 0.067;
                bool hasColorInfo = false;
                for (int fileId = 0; fileId < fileNames.size(); fileId++)
                {
                    GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileNames.at(fileId));
                    if (pointCloud != NULL)
                    { 
                        if (mPointCloudList.empty())
                        {
                            pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                        }
                        else
                        {
                            pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                        }
                        if (fileId == 0 && pointCloud->HasColor())
                        {
                            hasColorInfo = true;
                        }
                        if (!hasColorInfo)
                        {
                            GPP::Vector3 color = MagicCore::ToolKit::ColorCoding((fileId % 6) * colorDelta + 0.4);
                            SetPointCloudColor(pointCloud, color);
                        }
                        pointCloud->SetHasColor(hasColorInfo);
                        mPointCloudList.push_back(pointCloud);
                    }
                }
                UpdatePointCloudListRendering();
                InitViewTool();
            }
        }
    }

#if DEBUGDUMPFILE
    void RegistrationApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        if (dumpInfo == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpDumpInfo);
        mpDumpInfo = dumpInfo;
        if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP ||
            mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTCLOUD ||
            mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTCLOUD_MARK)
        {
            if (mpDumpInfo->GetPointCloud(0) == NULL || mpDumpInfo->GetPointCloud(1) == NULL)
            {
                MessageBox(NULL, "Dump信息的点云为NULL", "温馨提示", MB_OK);
                return;
            }
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = CopyPointCloud(mpDumpInfo->GetPointCloud(0));
            if (mpPointCloudRef == NULL)
            {
                MessageBox(NULL, "mpPointCloudRef is NULL", "温馨提示", MB_OK);
                return;
            }
            mpPointCloudRef->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
            SetPointCloudColor(mpPointCloudRef, GPP::Vector3(0.86, 0, 0));
            GPPFREEPOINTER(mpPointCloudFrom);
            mpPointCloudFrom = CopyPointCloud(mpDumpInfo->GetPointCloud(1));
            if (mpPointCloudFrom == NULL)
            {
                MessageBox(NULL, "mpPointCloudFrom is NULL", "温馨提示", MB_OK);
                GPPFREEPOINTER(mpPointCloudRef);
                return;
            }
            mpPointCloudFrom->UnifyCoords(mScaleValue, mObjCenterCoord);
            SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0, 0.86, 0));
            if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP)
            {
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
            else if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTCLOUD)
            {
                UpdatePointCloudRefRendering();
                UpdatePointCloudFromRendering();
            }
            else if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTCLOUD_MARK)
            {
                MessageBox(NULL, "暂时没有支持，需要些代码", "温馨提示", MB_OK);
            }
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
            MessageBox(NULL, "dump执行失败", "温馨提示", MB_OK);
            return;
        }
        if (mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ICP || 
            mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTCLOUD ||
            mpDumpInfo->GetApiName() == GPP::POINT_REGISTRATION_ALIGNPOINTCLOUD_MARK)
        {
            if (mpDumpInfo->GetPointCloud(1) == NULL)
            {
                return;
            }
            GPPFREEPOINTER(mpPointCloudFrom);
            mpPointCloudFrom = GPP::CopyPointCloud(mpDumpInfo->GetPointCloud(1));
            if (mpPointCloudFrom == NULL)
            {
                MessageBox(NULL, "mpPointCloudFrom is NULL", "温馨提示", MB_OK);
                return;
            }
            mpPointCloudFrom->UnifyCoords(mScaleValue, mObjCenterCoord);
            SetPointCloudColor(mpPointCloudFrom, GPP::Vector3(0, 0.86, 0));
            UpdatePointCloudFromRendering();
        }
        else if (mpDumpInfo->GetApiName() == GPP::POINT_SUM_EXTRACT_FUSION)
        {
            GPPFREEPOINTER(mpPointCloudRef);
            mpPointCloudRef = GPP::CopyPointCloud(mpDumpInfo->GetPointCloud(0));
            mpPointCloudRef->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
            UpdatePointCloudRefRendering();
            GPPFREEPOINTER(mpPointCloudFrom);
            UpdatePointCloudFromRendering();
        }
        GPPFREEPOINTER(mpDumpInfo);
    }
#endif

    void RegistrationApp::UpdatePointCloudFromRendering()
    {
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
            if (mpPointCloudFrom)
            {
                if (mpPointCloudFrom->HasNormal())
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_Right_RegistrationApp", "CookTorrancePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
                }
                else
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_Right_RegistrationApp", "SimplePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
                }
            }
            else
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Right_RegistrationApp");
            }
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_Right_RegistrationApp");
            if (mpPointCloudFrom)
            {
                if (mpPointCloudFrom && mpPointCloudFrom->HasNormal())
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_RegistrationApp", "CookTorrancePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_CENTER);
                }
                else
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudFrom_RegistrationApp", "SimplePoint", mpPointCloudFrom, MagicCore::RenderSystem::MODEL_NODE_CENTER);
                }
            }
            else
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudFrom_RegistrationApp");
            }
        }
    }

    void RegistrationApp::UpdatePointCloudRefRendering()
    {
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
            if (mpPointCloudRef)
            {
                if (mpPointCloudRef->HasNormal())
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Left_RegistrationApp", "CookTorrancePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_LEFT);
                }
                else
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_Left_RegistrationApp", "SimplePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_LEFT);
                }
            }
            else
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Left_RegistrationApp");
            }   
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_Left_RegistrationApp");
            if (mpPointCloudRef)
            {
                if (mpPointCloudRef->HasNormal())
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_RegistrationApp", "CookTorrancePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_CENTER);
                }
                else
                {
                    MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloudRef_RegistrationApp", "SimplePoint", mpPointCloudRef, MagicCore::RenderSystem::MODEL_NODE_CENTER);
                }
            }
            else
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
            }
        }
    }

    void RegistrationApp::UpdatePointCloudListRendering()
    {
        //static bool isOn = true;
        bool isOn = true;
        if (isOn)
        {
            bool hasNormalInfo = true;
            for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
            {
                if ((*itr)->HasNormal() == false)
                {
                    hasNormalInfo = false;
                    break;
                }
            }
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudRef_RegistrationApp");
            if (hasNormalInfo)
            {
                MagicCore::RenderSystem::Get()->RenderPointCloudList("PointCloudList_RegistrationApp", "CookTorrancePoint", 
                    mPointCloudList, true, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloudList("PointCloudList_RegistrationApp", "SimplePoint", 
                    mPointCloudList, false, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudList_RegistrationApp");
        }
        isOn = !isOn;
    }

    void RegistrationApp::UpdateMarkRefRendering()
    {
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_RegistrationApp");
            MagicCore::RenderSystem::Get()->RenderPointList("RefMarks_Left_RegistrationApp", "SimplePoint_Large", GPP::Vector3(1, 0, 1), mRefMarks, MagicCore::RenderSystem::MODEL_NODE_LEFT);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("RefMarks_Left_RegistrationApp");
            MagicCore::RenderSystem::Get()->RenderPointList("RefMarks_RegistrationApp", "SimplePoint_Large", GPP::Vector3(1, 0, 1), mRefMarks, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
    }

    void RegistrationApp::UpdateMarkFromRendering()
    {
        if (mIsSeparateDisplay)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_RegistrationApp");
            MagicCore::RenderSystem::Get()->RenderPointList("FromMarks_Right_RegistrationApp", "SimplePoint_Large", GPP::Vector3(0, 1, 1), mFromMarks, MagicCore::RenderSystem::MODEL_NODE_RIGHT);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("FromMarks_Right_RegistrationApp");
            MagicCore::RenderSystem::Get()->RenderPointList("FromMarks_RegistrationApp", "SimplePoint_Large", GPP::Vector3(0, 1, 1), mFromMarks, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }     
    }

    void RegistrationApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }
}
