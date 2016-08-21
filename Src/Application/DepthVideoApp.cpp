#include "stdafx.h"
#include <process.h>
#include "DepthVideoApp.h"
#include "DepthVideoAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "../Common/ViewTool.h"
#include "AppManager.h"

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        DepthVideoApp* app = (DepthVideoApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    DepthVideoApp::DepthVideoApp() :
        mpUI(NULL),
        mpViewTool(NULL),
        mCommandType(CT_NONE),
        mPointCloudList(),
        mObjCenterCoord(),
        mScaleValue(0),
        mSelectCloudIndex(0),
        mIsCommandInProgress(false),
        mUpdatePointCloudListRendering(false),
        mUpdateUIScrollBar(false),
        mProgressValue(-1),
        mGroupSize(0)
    {
    }

    DepthVideoApp::~DepthVideoApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
    }

    bool DepthVideoApp::Enter(void)
    {
        InfoLog << "Enter DepthVideoApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new DepthVideoAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool DepthVideoApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            if (mProgressValue >= 0)
            {
                mpUI->SetProgressbar(mProgressValue);
            }
            else
            {
                int progressValue = int(GPP::GetApiProgress() * 100.0);
                mpUI->SetProgressbar(progressValue);
            }
        }
        if (mUpdatePointCloudListRendering)
        {
            UpdatePointCloudListRendering();
            mUpdatePointCloudListRendering = false;
        }
        if (mUpdateUIScrollBar)
        {
            mUpdateUIScrollBar = false;
            mpUI->SetScrollRange(mPointCloudList.size());
        }
        return true;
    }

    bool DepthVideoApp::Exit(void)
    {
        InfoLog << "Exit DepthVideoApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool DepthVideoApp::MouseMoved( const OIS::MouseEvent &arg )
    {
        if (arg.state.buttonDown(OIS::MB_Middle) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_MIDDLE_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && arg.state.buttonDown(OIS::MB_Right) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_LEFT_DOWN);
        } 
        else if (arg.state.buttonDown(OIS::MB_Right) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_RIGHT_DOWN);
        }
        
        return true;
    }

    bool DepthVideoApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left || id == OIS::MB_Right) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool DepthVideoApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left || id == OIS::MB_Right) && mpViewTool != NULL)
        {
            mpViewTool->MouseReleased(); 
        }

        return true;
    }

    bool DepthVideoApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        return true;
    }

    void DepthVideoApp::WindowFocusChanged(Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
    }

    void DepthVideoApp::SetupScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
        Ogre::Light* light = sceneManager->createLight("DepthVideoApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
        InitViewTool();
    }

    void DepthVideoApp::ShutdownScene(void)
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("DepthVideoApp_SimpleLight");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloud_DepthVideoApp");
        /*MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        }*/
    }

    void DepthVideoApp::ClearData(void)
    {
        ClearPointCloudList();
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        mSelectCloudIndex = 0;
        mProgressValue = -1;
    }

    void DepthVideoApp::DoCommand(bool isSubThread)
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
            case MagicApp::DepthVideoApp::CT_NONE:
                break;
            case MagicApp::DepthVideoApp::CT_IMPORT_POINTCLOUD:
                ImportPointCloud(false);
                break;
            case MagicApp::DepthVideoApp::CT_ALIGN_POINTCLOUD:
                AlignPointCloudList(mGroupSize, false);
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
            mProgressValue = -1;
        }
    }

    void DepthVideoApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    bool DepthVideoApp::IsCommandAvaliable()
    {
        return true;
    }

    void DepthVideoApp::ClearPointCloudList()
    {
        for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
        {
            GPPFREEPOINTER(*itr);
        }
        mPointCloudList.clear();
    }

    void DepthVideoApp::UpdatePointCloudListRendering()
    {
        if (mPointCloudList.size() == 0)
        {
            return;
        }
        if (mPointCloudList.at(mSelectCloudIndex)->HasNormal())
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloud_DepthVideoApp", "CookTorrancePoint", mPointCloudList.at(mSelectCloudIndex), 
                MagicCore::RenderSystem::MODEL_NODE_CENTER, NULL, NULL);
        }
        else
        {
            MagicCore::RenderSystem::Get()->RenderPointCloud("PointCloud_DepthVideoApp", "SimplePoint", mPointCloudList.at(mSelectCloudIndex), 
                MagicCore::RenderSystem::MODEL_NODE_CENTER, NULL, NULL);
        }
    }

    void DepthVideoApp::ImportPointCloud(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = CT_IMPORT_POINTCLOUD;
            DoCommand(true);
        }
        else
        {
            std::vector<std::string> fileNames;
            char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0PLY Files(*.ply)\0*.ply\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
            if (MagicCore::ToolKit::MultiFileOpenDlg(fileNames, filterName))
            {
                if (fileNames.size() == 0)
                {
                    return;
                }
                mIsCommandInProgress = true;
                mSelectCloudIndex = 0;
                ClearPointCloudList();
                mPointCloudList.reserve(fileNames.size());
                for (int fileId = 0; fileId < fileNames.size(); fileId++)
                {
                    GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileNames.at(fileId));
                    if (pointCloud == NULL)
                    { 
                        continue;
                    }
                    if (mPointCloudList.empty())
                    {
                        pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                    }
                    else
                    {
                        pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                    }
                    mPointCloudList.push_back(pointCloud);
                    if (mPointCloudList.size() == 1)
                    {
                        mUpdatePointCloudListRendering = true;
                    }
                    mUpdateUIScrollBar = true;
                    mProgressValue = int(fileId * 100.0 / fileNames.size());
                }
                mIsCommandInProgress = false;
            }
        }
    }

    void DepthVideoApp::AlignPointCloudList(int groupSize, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = CT_ALIGN_POINTCLOUD;
            mGroupSize = groupSize;
            DoCommand(true);
        }
        else
        {
            std::vector<std::string> fileNames;
            char filterName[] = "ASC Files(*.asc)\0*.asc\0OBJ Files(*.obj)\0*.obj\0PLY Files(*.ply)\0*.ply\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
            if (MagicCore::ToolKit::MultiFileOpenDlg(fileNames, filterName))
            {
                int fileCount = fileNames.size();
                InfoLog << "fileCount = " << fileCount << std::endl;
                if (fileCount < 2)
                {
                    return;
                }

                //int groupSize = 45;
                int groupCount = fileCount / groupSize;
                if (fileCount % groupSize > 5)
                {
                    groupCount++;
                }
                for (int groupId = 0; groupId < groupCount; groupId++)
                {
                    ClearPointCloudList();
                    std::vector<GPP::Matrix4x4> initTransformList;
                    initTransformList.reserve(fileCount);
                    GPP::PointCloud* lastPointCloud = NULL;
                    GPP::Matrix4x4 transformAcc;
                    transformAcc.InitIdentityTransform();
                    int startFileId = groupId * groupSize;
                    int endFileId = startFileId + groupSize;
                    endFileId = endFileId > fileCount ? fileCount : endFileId;
                    for (int depthId = startFileId; depthId < endFileId; depthId++)
                    {
                        GPP::PointCloud* curPointCloud = GPP::Parser::ImportPointCloud(fileNames.at(depthId));
                        if (curPointCloud == NULL || curPointCloud->GetPointCount() < 10000)
                        {
                            InfoLog << "Point Cloud " << depthId << " Import failed" << std::endl;
                            continue;
                        }
                        GPP::PointCloud* originPointCloud = GPP::CopyPointCloud(curPointCloud); 
                        // Align point cloud
                        if (lastPointCloud != NULL)
                        {
                            int curPointCount = curPointCloud->GetPointCount();
                            for (int pid = 0; pid < curPointCount; pid++)
                            {
                                curPointCloud->SetPointCoord(pid, transformAcc.TransformPoint(curPointCloud->GetPointCoord(pid)));
                                curPointCloud->SetPointNormal(pid, transformAcc.RotateVector(curPointCloud->GetPointNormal(pid)));
                            }

                            GPP::Matrix4x4 transformLocal;
                            transformLocal.InitIdentityTransform();
                            GPP::ErrorCode res = GPP::RegistratePointCloud::AlignPointCloud(lastPointCloud, curPointCloud, &transformLocal, 
                                1000);
                            if (res != GPP_NO_ERROR)
                            {
                                InfoLog << "Point Cloud " << depthId << " AlignPointCloud failed" << std::endl;
                                GPPFREEPOINTER(curPointCloud);
                                GPPFREEPOINTER(originPointCloud);
                                continue;
                            }
                            else
                            {
                                for (int pid = 0; pid < curPointCount; pid++)
                                {
                                    curPointCloud->SetPointCoord(pid, transformLocal.TransformPoint(curPointCloud->GetPointCoord(pid)));
                                    curPointCloud->SetPointNormal(pid, transformLocal.RotateVector(curPointCloud->GetPointNormal(pid)));
                                }
                                GPP::Matrix4x4 icpLocal;
                                icpLocal.InitIdentityTransform();
                                res = GPP::RegistratePointCloud::ICPRegistrate(lastPointCloud, NULL, curPointCloud, NULL, 
                                    &icpLocal, NULL, true);
                                if (res == GPP_NO_ERROR)
                                {
                                    for (int pid = 0; pid < curPointCount; pid++)
                                    {
                                        curPointCloud->SetPointCoord(pid, icpLocal.TransformPoint(curPointCloud->GetPointCoord(pid)));
                                        curPointCloud->SetPointNormal(pid, icpLocal.RotateVector(curPointCloud->GetPointNormal(pid)));
                                    }
                                    transformLocal = icpLocal * transformLocal;
                                }
                                else
                                {
                                    InfoLog << "Point Cloud " << depthId << " ICPRegistrate failed" << std::endl;
                                    GPPFREEPOINTER(curPointCloud);
                                    GPPFREEPOINTER(originPointCloud);
                                    continue;
                                }
                                transformAcc = transformLocal * transformAcc;
                            }
                        }  
                        GPPFREEPOINTER(lastPointCloud);
                        lastPointCloud = curPointCloud;

                        mPointCloudList.push_back(originPointCloud);
                        initTransformList.push_back(transformAcc);
                        // Export point cloud
                        std::string inputModelName = fileNames.at(depthId);
                        size_t dotPos = inputModelName.rfind('.');
                        std::stringstream dumpStream;
                        dumpStream << inputModelName.substr(0, dotPos) << "_align.asc";
                        std::string outputModelName;
                        dumpStream >> outputModelName;
                        GPP::Parser::ExportPointCloud(outputModelName, curPointCloud);

                        mProgressValue = int(depthId * 100.0 / fileCount);
                        mUpdateUIScrollBar = true;
                        mUpdatePointCloudListRendering = true;
                    }
                    GPPFREEPOINTER(lastPointCloud);
                    if (mPointCloudList.size() < 2)
                    {
                        MessageBox(NULL, "初始拼接失败", "温馨提示", MB_OK);
                        return;
                    }
                    //mProgressValue = -1;
                    // Global registrate
                    std::vector<GPP::IPointCloud*> pointCloudList;
                    for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
                    {
                        pointCloudList.push_back(*itr);
                    }
                    std::vector<GPP::Matrix4x4> resultTransform;
                    GPP::ErrorCode res = GPP::RegistratePointCloud::GlobalRegistrate(&pointCloudList, 10, &resultTransform, 
                        &initTransformList, true, 0);
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "全局注册失败", "温馨提示", MB_OK);
                        return;
                    }

                    int cloudCount = mPointCloudList.size();   
                    // Fuse point cloud
                    GPP::Vector3 bboxMin, bboxMax;
                    res = GPP::CalculatePointCloudListBoundingBox(pointCloudList, &resultTransform, bboxMin, bboxMax);
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "包围盒计算失败", "温馨提示", MB_OK);
                        return;
                    }
                    GPP::Vector3 deltaVec(0.1, 0.1, 0.1);
                    bboxMin -= deltaVec;
                    bboxMax += deltaVec;
                    GPP::PointCloudPointList pointList(pointCloudList.at(0));
                    double epsilon = 0;
                    res = GPP::CalculatePointListDensity(&pointList, 4, epsilon);
                    int resolutionX = int((bboxMax[0] - bboxMin[0]) / epsilon) + 1;
		            int resolutionY = int((bboxMax[1] - bboxMin[1]) / epsilon) + 1;
		            int resolutionZ = int((bboxMax[2] - bboxMin[2]) / epsilon) + 1;
                    InfoLog << "epsilon=" << epsilon << " resX=" << resolutionX << " resY=" << resolutionY << " resZ=" << resolutionZ << std::endl;
                    GPP::SignedDistanceFunction sdf(resolutionX, resolutionY, resolutionZ, bboxMin, bboxMax);
                    for (int cid = 0; cid < cloudCount; cid++)
                    {
                        //mProgressValue = int(cid * 100.0 / cloudCount);
                        res = sdf.UpdateFunction(pointCloudList.at(cid), &(resultTransform.at(cid)));
                        if (res != GPP_NO_ERROR)
                        {
                            MessageBox(NULL, "点云融合失败", "温馨提示", MB_OK);
                            return;
                        }
                    }
                    GPP::PointCloud* fusedPointCloud = new GPP::PointCloud;
                    res = sdf.ExtractPointCloud(fusedPointCloud);
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "点云萃取失败", "温馨提示", MB_OK);
                        return;
                    }
                    mPointCloudList.push_back(fusedPointCloud);
                    mUpdateUIScrollBar = true;
                    mUpdatePointCloudListRendering = true;
                    // save result
                    std::stringstream outputStream;
                    outputStream << "fuse_res_" << groupId << ".asc";
                    std::string outputModelName;
                    outputStream >> outputModelName;
                    res = GPP::Parser::ExportPointCloud(outputModelName, fusedPointCloud);
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "导出点云失败", "温馨提示", MB_OK);
                    }
                }
            }
            else
            {
                InfoLog << "Open file failed" << std::endl;
            }
        }
    }

    void DepthVideoApp::SetPointCloudIndex(int index)
    {
        if (index >= mPointCloudList.size())
        {
            return;
        }
        mSelectCloudIndex = index;
        mUpdatePointCloudListRendering = true;
    }

    bool DepthVideoApp::IsCommandInProgress(void)
    {
        return mIsCommandInProgress;
    }
}
