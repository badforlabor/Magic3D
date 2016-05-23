#include "stdafx.h"
#include <process.h>
#include "PointShopApp.h"
#include "PointShopAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
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
        mpPickTool(NULL),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mCommandType(NONE),
        mUpdatePointCloudRendering(false),
        mIsCommandInProgress(false),
        mpTriMesh(NULL),
        mIsDepthImage(0),
        mReconstructionQuality(4),
        mPointSelectFlag(),
        mRightMouseType(MOVE),
        mMousePressdCoord(),
        mIgnoreBack(true)
    {
    }

    PointShopApp::~PointShopApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpPointCloud);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
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

    void PointShopApp::SelectControlPointByRectangle(int startCoordX, int startCoordY, int endCoordX, int endCoordY)
    {
        GPP::Vector2 pos0(startCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - startCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        GPP::Vector2 pos1(endCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - endCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        double minX = (pos0[0] < pos1[0]) ? pos0[0] : pos1[0];
        double maxX = (pos0[0] > pos1[0]) ? pos0[0] : pos1[0];
        double minY = (pos0[1] < pos1[1]) ? pos0[1] : pos1[1];
        double maxY = (pos0[1] > pos1[1]) ? pos0[1] : pos1[1];
        Ogre::Matrix4 worldM = MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->_getFullTransform();
        Ogre::Matrix4 viewM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getViewMatrix();
        Ogre::Matrix4 projM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getProjectionMatrix();
        Ogre::Matrix4 wvpM   = projM * viewM * worldM;
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        GPP::Vector3 coord;
        GPP::Vector3 normal;
        bool ignoreBack = mpPointCloud->HasNormal() && mIgnoreBack;
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            if (mRightMouseType == SELECT_ADD && mPointSelectFlag.at(pid))
            {
                continue;
            }
            else if (mRightMouseType == SELECT_DELETE && mPointSelectFlag.at(pid) == 0)
            {
                continue;
            }
            GPP::Vector3 coord = mpPointCloud->GetPointCoord(pid);
            Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
            ogreCoord = wvpM * ogreCoord;
            if (ogreCoord.x > minX && ogreCoord.x < maxX && ogreCoord.y > minY && ogreCoord.y < maxY)
            {
                if (ignoreBack)
                {
                    normal = mpPointCloud->GetPointNormal(pid);
                    Ogre::Vector4 ogreNormal(normal[0], normal[1], normal[2], 0);
                    ogreNormal = worldM * ogreNormal;
                    if (ogreNormal.z > 0)
                    {
                        if (mRightMouseType == SELECT_ADD)
                        {
                            mPointSelectFlag.at(pid) = 1;
                        }
                        else
                        {
                            mPointSelectFlag.at(pid) = 0;
                        }
                    }
                }
                else
                {
                    if (mRightMouseType == SELECT_ADD)
                    {
                        mPointSelectFlag.at(pid) = 1;
                    }
                    else
                    {
                        mPointSelectFlag.at(pid) = 0;
                    }
                }
            }
        }
    }

    void PointShopApp::UpdateRectangleRendering(int startCoordX, int startCoordY, int endCoordX, int endCoordY)
    {
        Ogre::ManualObject* pMObj = NULL;
        Ogre::SceneManager* pSceneMgr = MagicCore::RenderSystem::Get()->GetSceneManager();
        if (pSceneMgr->hasManualObject("PickRectangleObj"))
        {
            pMObj = pSceneMgr->getManualObject("PickRectangleObj");
            pMObj->clear();
        }
        else
        {
            pMObj = pSceneMgr->createManualObject("PickRectangleObj");
            pMObj->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
            pMObj->setUseIdentityProjection(true);
            pMObj->setUseIdentityView(true);
            if (pSceneMgr->hasSceneNode("PickRectangleNode"))
            {
                pSceneMgr->getSceneNode("PickRectangleNode")->attachObject(pMObj);
            }
            else
            {
                pSceneMgr->getRootSceneNode()->createChildSceneNode("PickRectangleNode")->attachObject(pMObj);
            }
        }
        GPP::Vector2 pos0(startCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - startCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        GPP::Vector2 pos1(endCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - endCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        pMObj->begin("SimpleLine", Ogre::RenderOperation::OT_LINE_STRIP);
        pMObj->position(pos0[0], pos0[1], -1);
        pMObj->colour(1, 0, 0);
        pMObj->position(pos0[0], pos1[1], -1);
        pMObj->colour(1, 0, 0);
        pMObj->position(pos1[0], pos1[1], -1);
        pMObj->colour(1, 0, 0);
        pMObj->position(pos1[0], pos0[1], -1);
        pMObj->colour(1, 0, 0);
        pMObj->position(pos0[0], pos0[1], -1);
        pMObj->colour(1, 0, 0);
        pMObj->end();
    }

    void PointShopApp::ClearRectangleRendering()
    {
        MagicCore::RenderSystem::Get()->HideRenderingObject("PickRectangleObj");
    }

    bool PointShopApp::MouseMoved( const OIS::MouseEvent &arg )
    {
        if (arg.state.buttonDown(OIS::MB_Middle) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_MIDDLE_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Right) && mRightMouseType == MOVE && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_RIGHT_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_LEFT_DOWN);
        } 
        else if ((mRightMouseType == SELECT_ADD || mRightMouseType == SELECT_DELETE) && arg.state.buttonDown(OIS::MB_Right))
        {
            UpdateRectangleRendering(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
        }

        return true;
    }

    bool PointShopApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left || (id == OIS::MB_Right && mRightMouseType == MOVE)) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (mRightMouseType == REVERSE_NORMAL && arg.state.buttonDown(OIS::MB_Right) && mpPickTool)
        {
            mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (arg.state.buttonDown(OIS::MB_Right) && mpPointCloud && (mRightMouseType == SELECT_ADD || mRightMouseType == SELECT_DELETE))
        {
            mMousePressdCoord[0] = arg.state.X.abs;
            mMousePressdCoord[1] = arg.state.Y.abs;
        }
        return true;
    }

    bool PointShopApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left || (id == OIS::MB_Right && mRightMouseType == MOVE)) && mpViewTool != NULL)
        {
            mpViewTool->MouseReleased(); 
        }
        else if (id == OIS::MB_Right && mRightMouseType == REVERSE_NORMAL && mpPickTool)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickPointId();
            mpPickTool->ClearPickedIds();
            if (pickedId != -1)
            {
                GPP::ErrorCode res = GPP::ConsolidatePointCloud::ReversePatchNormal(mpPointCloud, pickedId);
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
                UpdatePointCloudRendering();
            }
        }
        else if (id == OIS::MB_Right && mpPointCloud && (mRightMouseType == SELECT_ADD || mRightMouseType == SELECT_DELETE))
        {
            SelectControlPointByRectangle(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
            ClearRectangleRendering();
            UpdatePointCloudRendering();
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
                GPP::ConsolidatePointCloud::ConsolidateRawScanData(mpPointCloud, 512, 424, false, true);
                UpdatePointCloudRendering();
                mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
            }
        }
        else if (arg.key == OIS::KC_N)
        {
            if (mpPointCloud)
            {
                mpPointCloud->SetHasNormal(false);
                UpdatePointCloudRendering();
            }
        }
        else if (arg.key == OIS::KC_C)
        {
            if (mpPointCloud)
            {
                mpPointCloud->SetHasColor(false);
                int pointCount = mpPointCloud->GetPointCount();
                for (int pid = 0; pid < pointCount; pid++)
                {
                    mpPointCloud->SetPointColor(pid, GPP::Vector3(0.09, 0.48627, 0.69));
                }
                UpdatePointCloudRendering();
            }
        }
        else if (arg.key == OIS::KC_A)
        {
            mRightMouseType = SELECT_ADD;
        }
        else if (arg.key == OIS::KC_S)
        {
            mRightMouseType = SELECT_DELETE;
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
        InitViewTool();
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
        GPPFREEPOINTER(mpPickTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        mPointSelectFlag.clear();
        mRightMouseType = MOVE;
    }

    void PointShopApp::ResetSelection()
    {
        mPointSelectFlag = std::vector<bool>(mpPointCloud->GetPointCount(), 0);
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
                ReconstructMesh(mReconstructionQuality, false);
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
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        std::string fileName;
        char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
            if (pointCloud != NULL)
            { 
                pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                GPPFREEPOINTER(mpPointCloud);
                mpPointCloud = pointCloud;
                InfoLog << "Import Point Cloud: " << mpPointCloud->GetPointCount() << " points" << std::endl;
                mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
                UpdatePickTool();
                ResetSelection();
                mRightMouseType = MOVE;
                UpdatePointCloudRendering();
                return true;
            }
            else
            {
                mpUI->SetPointCloudInfo(0);
                MessageBox(NULL, "点云导入失败, 目前支持导入格式：asc，obj", "温馨提示", MB_OK);
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
        char filterName[] = "Support format(*.obj, *.ply, *.asc, *.gpc)\0*.*\0";
        if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
        {
            size_t dotPos = fileName.rfind('.');
            if (dotPos == std::string::npos)
            {
                MessageBox(NULL, "请输入文件后缀名", "温馨提示", MB_OK);
                return;
            }
            mpPointCloud->UnifyCoords(1.0 / mScaleValue, mObjCenterCoord * (-mScaleValue));
            GPP::ErrorCode res = GPP::Parser::ExportPointCloud(fileName, mpPointCloud);
            mpPointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "导出点云失败", "温馨提示", MB_OK);
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
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
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
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云法线光滑失败", "温馨提示", MB_OK);
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
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
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
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云坐标光滑失败", "温馨提示", MB_OK);
                return;
            }
            mIsCommandInProgress = true;
            res = GPP::ConsolidatePointCloud::SmoothGeometryByNormal(mpPointCloud);
            mIsCommandInProgress = false;
            mUpdatePointCloudRendering = true;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云坐标光滑失败", "温馨提示", MB_OK);
                return;
            }
        }
    }

    void PointShopApp::SelectByRectangle()
    {
        mRightMouseType = SELECT_ADD;
    }

    void PointShopApp::EraseByRectangle()
    {
        mRightMouseType = SELECT_DELETE;
    }
 
    void PointShopApp::DeleteSelections()
    {
        if (mpPointCloud == NULL)
        {
            return;
        }
        std::vector<GPP::Int> deleteIndex;
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            if (mPointSelectFlag.at(pid))
            {
                deleteIndex.push_back(pid);
            }
        }
        GPP::ErrorCode res = DeletePointCloudElements(mpPointCloud, deleteIndex);
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "删点失败", "温馨提示", MB_OK);
            return;
        }
        ResetSelection();
        mUpdatePointCloudRendering = true;
        mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
    }

    void PointShopApp::IgnoreBack(bool ignore)
    {
        mIgnoreBack = ignore;
    }

    void PointShopApp::MoveModel(void)
    {
        mRightMouseType = MOVE;
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
                MessageBox(NULL, "点云的点个数小于1，操作失败", "温馨提示", MB_OK);
                return;
            }
            std::vector<GPP::Real> uniformity;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateUniformity(mpPointCloud, &uniformity);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
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
            res = DeletePointCloudElements(mpPointCloud, deleteIndex);
            if (res != GPP_NO_ERROR)
            {
                return;
            }
            ResetSelection();
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
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
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
                MessageBox(NULL, "点云的点个数小于1，操作失败", "温馨提示", MB_OK);
                return;
            }
            std::vector<GPP::Real> isolation;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidatePointCloud::CalculateIsolation(mpPointCloud, &isolation);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云去除孤立项失败", "温馨提示", MB_OK);
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
            ResetSelection();
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
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "点云采样失败", "温馨提示", MB_OK);
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
        samplePointCloud->SetHasColor(mpPointCloud->HasColor());
        delete mpPointCloud;
        mpPointCloud = samplePointCloud;
        ResetSelection();
        mUpdatePointCloudRendering = true;
        GPPFREEARRAY(sampleIndex);
        mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
    }

    void PointShopApp::SimplifyPointCloud(int resolution)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (resolution > 10000 || resolution < 1)
        {
            MessageBox(NULL, "采样分辨率区间为[1, 10000]", "温馨提示", MB_OK);
            return;
        }
        GPP::PointCloudPointList pointList(mpPointCloud);
        GPP::Vector3 bboxMin, bboxMax;
        GPP::ErrorCode res = GPP::CalculatePointListBoundingBox(&pointList, bboxMin, bboxMax);
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "点云包围盒计算失败", "温馨提示", MB_OK);
            return;
        }
        GPP::Vector3 delta(0.1, 0.1, 0.1);
        bboxMin -= delta;
        bboxMax += delta;
		double epsilon = (bboxMax - bboxMin).Length() / double(resolution);
		if (epsilon < 1.0e-15)
		{
			epsilon = 1.0e-15;
		}
		int resolutionX = int((bboxMax[0] - bboxMin[0]) / epsilon) + 1;
		int resolutionY = int((bboxMax[1] - bboxMin[1]) / epsilon) + 1;
		int resolutionZ = int((bboxMax[2] - bboxMin[2]) / epsilon) + 1;
        GPP::FusePointCloud fusePointCloud(resolutionX, resolutionY, resolutionZ, bboxMin, bboxMax, mpPointCloud->HasNormal());
        std::vector<GPP::Real> fields;
        if (mpPointCloud->HasColor())
        {
            GPP::Int pointCount = mpPointCloud->GetPointCount();
            fields.reserve(pointCount * 3);
            for (GPP::Int pid = 0; pid < pointCount; pid++)
            {
                GPP::Vector3 color = mpPointCloud->GetPointColor(pid);
                fields.push_back(color[0]);
                fields.push_back(color[1]);
                fields.push_back(color[2]);
            }
        }
        res = fusePointCloud.UpdateFuseFunction(mpPointCloud, NULL, &fields);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "点云简化失败", "温馨提示", MB_OK);
            return;
        }
        GPP::PointCloud* extractPointCloud = new GPP::PointCloud;
        if (mpPointCloud->HasColor())
        {
            std::vector<GPP::Real> extractFields;
            res = fusePointCloud.ExtractPointCloud(extractPointCloud, &extractFields);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云简化失败", "温馨提示", MB_OK);
                return;
            }
            GPP::Int pointCount = extractPointCloud->GetPointCount();
            for (GPP::Int pid = 0; pid < pointCount; pid++)
            {
                extractPointCloud->SetPointColor(pid, GPP::Vector3(extractFields.at(pid * 3), 
                    extractFields.at(pid * 3 + 1), extractFields.at(pid * 3 + 2)));
            }
        }
        else
        {
            res = fusePointCloud.ExtractPointCloud(extractPointCloud, NULL);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云简化失败", "温馨提示", MB_OK);
                return;
            }
        }
        //extractPointCloud->SetHasNormal(mpPointCloud->HasNormal());
        extractPointCloud->SetHasColor(mpPointCloud->HasColor());
        GPPFREEPOINTER(mpPointCloud);
        mpPointCloud = extractPointCloud;
        ResetSelection();
        UpdatePickTool();
        UpdatePointCloudRendering();
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

    void PointShopApp::FlipPointCloudNormal()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        GPP::Int pointCount = mpPointCloud->GetPointCount();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            mpPointCloud->SetPointNormal(pid, mpPointCloud->GetPointNormal(pid) * -1.0);
        }
        UpdatePointCloudRendering();
    }

    void PointShopApp::ReversePatchNormal()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        if (mRightMouseType == REVERSE_NORMAL)
        {
            mRightMouseType = MOVE;
        }
        else
        {
            mRightMouseType = REVERSE_NORMAL;
        }
        if (mRightMouseType == REVERSE_NORMAL)
        {
            MessageBox(NULL, "鼠标右键进入点云反转法线模式，再次点击本按钮返回平移模式", "温馨提示", MB_OK);
        }
    }

    void PointShopApp::ReconstructMesh(int quality, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        else if (mpPointCloud->HasNormal() == false)
        {
            MessageBox(NULL, "请先给点云计算法向量", "温馨提示", MB_OK);
            return;
        }
        else if (quality < 0 || quality > 6)
        {
            MessageBox(NULL, "三角化质量参数范围[0, 6]，数字越大，质量越好，速度越慢", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = RECONSTRUCTION;
            mReconstructionQuality = quality;
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
            GPP::ErrorCode res = GPP::ReconstructMesh::Reconstruct(mpPointCloud, triMesh, quality, false, &pointColorFields, &vertexColorField);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "点云三角化失败", "温馨提示", MB_OK);
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
        GPPFREEPOINTER(mpPointCloud);
        mpPointCloud = pointCloud;
        mObjCenterCoord = objCenterCoord;
        mScaleValue = scaleValue;
        if (!mpPointCloud)
        {
            MessageBox(NULL, "错误：点云为NULL", "温馨提示", MB_OK);
            return;
        }
        mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
        UpdatePickTool();
        ResetSelection();
        mRightMouseType = MOVE;
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
        UpdatePointCloudRendering();
        if (mpPointCloud)
        {
            mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
        }
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
            MessageBox(NULL, "Dump Run Failed", "温馨提示", MB_OK);
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
            if (mpPointCloud)
            {
                mpUI->SetPointCloudInfo(mpPointCloud->GetPointCount());
            }
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
        GPP::Vector3 selectColor(1, 0, 0);
        if (mpPointCloud->HasNormal())
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloud_PointShop", "CookTorrancePoint", mpPointCloud, MagicCore::RenderSystem::MODEL_NODE_CENTER,
                &mPointSelectFlag, &selectColor);
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloud_PointShop", "SimplePoint", mpPointCloud, MagicCore::RenderSystem::MODEL_NODE_CENTER,
                &mPointSelectFlag, &selectColor);
        }
    }

    bool PointShopApp::IsCommandAvaliable()
    {
        if (mpPointCloud == NULL)
        {
            MessageBox(NULL, "请先导入点云", "温馨提示", MB_OK);
            return false;
        }
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
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

    void PointShopApp::UpdatePickTool()
    {
        GPPFREEPOINTER(mpPickTool);
        mpPickTool = new MagicCore::PickTool;
        mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, mpPointCloud, NULL, "ModelNode");
    }
}
