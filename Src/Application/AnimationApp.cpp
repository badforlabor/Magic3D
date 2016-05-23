#include "stdafx.h"
#include <process.h>
#include "AnimationApp.h"
#include "AnimationAppUI.h"
#include "AppManager.h"
#include "MeshShopApp.h"
#include "PointShopApp.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "../Common/RenderSystem.h"
#include "GPP.h"

namespace MagicApp
{
    AnimationApp::AnimationApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mpPointCloud(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mpViewTool(NULL),
        mDeformation(NULL),
        mControlIds(),
        mIsDeformationInitialised(false),
        mPickControlId(-1),
        mPickTargetCoord(),
        mMousePressdCoord(),
        mControlFixFlags(),
        mRightMouseType(DEFORM),
        mAddSelection(true),
        mFirstAlert(true),
        mTargetControlCoords(),
        mTargetControlIds()
    {
    }

    AnimationApp::~AnimationApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpPointCloud);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mDeformation);
    }

    void AnimationApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        ClearMeshData();
        ClearPointCloudData();
        mRightMouseType = DEFORM;
        mDeformation->Clear();
        std::vector<GPP::Int>().swap(mControlIds);
        std::vector<std::set<int> >().swap(mControlNeighbors);
        std::vector<bool>().swap(mControlFixFlags);
        std::vector<GPP::Vector3>().swap(mTargetControlCoords);
        std::vector<int>().swap(mTargetControlIds);
    }

    void AnimationApp::ClearPointCloudData()
    {
        GPPFREEPOINTER(mpPointCloud);
        mIsDeformationInitialised = false;
        mPickControlId = -1;
        mTargetControlCoords.clear();
        mTargetControlIds.clear();
    }
    
    void AnimationApp::ClearMeshData()
    {
        GPPFREEPOINTER(mpTriMesh);
        mIsDeformationInitialised = false;
        mPickControlId = -1;
        mTargetControlCoords.clear();
        mTargetControlIds.clear();
    }

    bool AnimationApp::Enter()
    {
        InfoLog << "Enter AnimationApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new AnimationAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool AnimationApp::Update(double timeElapsed)
    {
        return true;
    }

    bool AnimationApp::Exit()
    {
        InfoLog << "Exit AnimationApp" << std::endl; 
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        ShutdownScene();
        return true;
    }

    void AnimationApp::PickControlPoint(int mouseCoordX, int mouseCoordY)
    {
        GPP::Vector2 mouseCoord(mouseCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - mouseCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode") == false || mControlIds.empty())
        {
            return;
        }
        double pointSizeSquared = 0.01 * 0.01;
        Ogre::Matrix4 worldM = MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->_getFullTransform();
        Ogre::Matrix4 viewM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getViewMatrix();
        Ogre::Matrix4 projM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getProjectionMatrix();
        Ogre::Matrix4 wvpM   = projM * viewM * worldM;
        double minZ = 1.0e10;
        mPickControlId = -1;
        GPP::Int pointCount = mControlIds.size();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            if (mControlFixFlags.at(pid))
            {
                continue;
            }
            GPP::Vector3 coord;
            if (mpTriMesh)
            {
                coord = mpTriMesh->GetVertexCoord(mControlIds.at(pid));
            }
            else
            {
                coord = mpPointCloud->GetPointCoord(mControlIds.at(pid));
            }
            Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
            ogreCoord = wvpM * ogreCoord;
            GPP::Vector2 screenCoord(ogreCoord.x, ogreCoord.y);
            if ((screenCoord - mouseCoord).LengthSquared() < pointSizeSquared)
            {
                if (ogreCoord.z < minZ)
                {
                    minZ = ogreCoord.z;
                    mPickControlId = pid;
                }
            }
        }
        if (mPickControlId != -1)
        {
            InfoLog << "mControlNeighbors.at(mPickControlId).size = " << mControlNeighbors.at(mPickControlId).size() << std::endl;
            /*std::vector<GPP::Vector3> startCoords, endCoords;
            if (mpTriMesh)
            {
                for (std::set<int>::iterator itr = mControlNeighbors.at(mPickControlId).begin(); itr != mControlNeighbors.at(mPickControlId).end(); ++itr)
                {
                    startCoords.push_back(mpTriMesh->GetVertexCoord(mControlIds.at(mPickControlId)));
                    endCoords.push_back(mpTriMesh->GetVertexCoord(mControlIds.at(*itr)));
                }
            }
            else
            {
                for (std::set<int>::iterator itr = mControlNeighbors.at(mPickControlId).begin(); itr != mControlNeighbors.at(mPickControlId).end(); ++itr)
                {
                    startCoords.push_back(mpPointCloud->GetPointCoord(mControlIds.at(mPickControlId)));
                    endCoords.push_back(mpPointCloud->GetPointCoord(mControlIds.at(*itr)));
                }
            }
            MagicCore::RenderSystem::Get()->RenderLineSegments("Test", "Simple_Line", startCoords, endCoords);*/
            if (mpTriMesh)
            {
                mPickTargetCoord = mpTriMesh->GetVertexCoord(mControlIds.at(mPickControlId));
            }
            else
            {
                mPickTargetCoord = mpPointCloud->GetPointCoord(mControlIds.at(mPickControlId));
            }
        }
    }

    void AnimationApp::DragControlPoint(int mouseCoordX, int mouseCoordY, bool mouseReleased)
    {
        if (mPickControlId == -1)
        {
            return;
        }
        GPP::Vector2 mouseCoord(mouseCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - mouseCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        Ogre::Ray ray = MagicCore::RenderSystem::Get()->GetMainCamera()->getCameraToViewportRay((mouseCoord[0] + 1.0) / 2.0, (1.0 - mouseCoord[1]) / 2.0);
        Ogre::Vector3 originCoord = ray.getOrigin();
        Ogre::Vector3 dir = ray.getDirection();
        Ogre::Matrix4 worldM = MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->_getFullTransform();
        Ogre::Vector3 ogrePickCoord(mPickTargetCoord[0], mPickTargetCoord[1], mPickTargetCoord[2]);
        ogrePickCoord = worldM * ogrePickCoord;
        double t = dir.dotProduct(ogrePickCoord - originCoord);
        Ogre::Vector3 targetCoord = originCoord + dir * t;
        Ogre::Matrix4 worldMInverse = worldM.inverse();
        targetCoord = worldMInverse * targetCoord;
        mPickTargetCoord = GPP::Vector3(targetCoord[0], targetCoord[1], targetCoord[2]);
        if (mouseReleased)
        {
            int targetIndex = -1;
            for (int tid = 0; tid < mTargetControlIds.size(); tid++)
            {
                if (mTargetControlIds.at(tid) == mPickControlId)
                {
                    targetIndex = tid;
                }
            }
            if (targetIndex == -1)
            {
                mTargetControlIds.push_back(mPickControlId);
                mTargetControlCoords.push_back(mPickTargetCoord);
            }
            else
            {
                mTargetControlCoords.at(targetIndex) = mPickTargetCoord;
            }
        }
        UpdateControlRendering();
    }

    void AnimationApp::UpdateDeformation(int mouseCoordX, int mouseCoordY)
    {
        if (mPickControlId == -1)
        {
            return;
        }
        GPP::Vector2 mouseCoord(mouseCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - mouseCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
        Ogre::Ray ray = MagicCore::RenderSystem::Get()->GetMainCamera()->getCameraToViewportRay((mouseCoord[0] + 1.0) / 2.0, (1.0 - mouseCoord[1]) / 2.0);
        Ogre::Vector3 originCoord = ray.getOrigin();
        Ogre::Vector3 dir = ray.getDirection();
        Ogre::Matrix4 worldM = MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->_getFullTransform();
        Ogre::Vector3 ogrePickCoord(mPickTargetCoord[0], mPickTargetCoord[1], mPickTargetCoord[2]);
        ogrePickCoord = worldM * ogrePickCoord;
        double t = dir.dotProduct(ogrePickCoord - originCoord);
        Ogre::Vector3 targetCoord = originCoord + dir * t;
        Ogre::Matrix4 worldMInverse = worldM.inverse();
        targetCoord = worldMInverse * targetCoord;
        mPickTargetCoord = GPP::Vector3(targetCoord[0], targetCoord[1], targetCoord[2]);
        std::vector<int> controlIndex;
        controlIndex.push_back(mPickControlId);
        std::vector<GPP::Vector3> targetCoords;
        targetCoords.push_back(mPickTargetCoord);  
        GPP::ErrorCode res = mDeformation->Deform(controlIndex, targetCoords, mControlFixFlags);
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "三维模型变形失败", "温馨提示", MB_OK);
            mPickControlId = -1;
            return;
        }
        UpdateModelRendering();
        UpdateControlRendering();
    }

    void AnimationApp::SelectControlPointByRectangle(int startCoordX, int startCoordY, int endCoordX, int endCoordY)
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
        GPP::Int pointCount = mControlIds.size();
        for (GPP::Int pid = 0; pid < pointCount; pid++)
        {
            GPP::Vector3 coord;
            if (mpTriMesh)
            {
                coord = mpTriMesh->GetVertexCoord(mControlIds.at(pid));
            }
            else
            {
                coord = mpPointCloud->GetPointCoord(mControlIds.at(pid));
            }
            Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
            ogreCoord = wvpM * ogreCoord;
            if (ogreCoord.x > minX && ogreCoord.x < maxX && ogreCoord.y > minY && ogreCoord.y < maxY)
            {
                mControlFixFlags.at(pid) = (!mAddSelection);
            }
        }
    }

    void AnimationApp::UpdateRectangleRendering(int startCoordX, int startCoordY, int endCoordX, int endCoordY)
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
        pMObj->colour(0.09, 0.48627, 0.69);
        pMObj->position(pos0[0], pos1[1], -1);
        pMObj->colour(0.09, 0.48627, 0.69);
        pMObj->position(pos1[0], pos1[1], -1);
        pMObj->colour(0.09, 0.48627, 0.69);
        pMObj->position(pos1[0], pos0[1], -1);
        pMObj->colour(0.09, 0.48627, 0.69);
        pMObj->position(pos0[0], pos0[1], -1);
        pMObj->colour(0.09, 0.48627, 0.69);
        pMObj->end();
    }

    void AnimationApp::ClearRectangleRendering()
    {
        MagicCore::RenderSystem::Get()->HideRenderingObject("PickRectangleObj");
    }

    bool AnimationApp::MouseMoved( const OIS::MouseEvent &arg )
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
        else if (arg.state.buttonDown(OIS::MB_Right) && mIsDeformationInitialised)
        {
            if (mRightMouseType == SELECT)
            {
                UpdateRectangleRendering(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
            }
            else if (mRightMouseType == DEFORM && mPickControlId >= 0)
            {
                //DragControlPoint(arg.state.X.abs, arg.state.Y.abs);
                UpdateDeformation(arg.state.X.abs, arg.state.Y.abs);
            }
            else if (mRightMouseType == MOVE && mPickControlId >= 0)
            {
                DragControlPoint(arg.state.X.abs, arg.state.Y.abs, false);
            }
        }
        
        return true;
    }

    bool AnimationApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        if ((id == OIS::MB_Right) && mIsDeformationInitialised)
        {
            if (mRightMouseType == SELECT)
            {
                mMousePressdCoord[0] = arg.state.X.abs;
                mMousePressdCoord[1] = arg.state.Y.abs;
            }
            else
            {
                PickControlPoint(arg.state.X.abs, arg.state.Y.abs);
            }
        }
        return true;
    }

    bool AnimationApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        if ((id == OIS::MB_Right) && mIsDeformationInitialised)
        {
            if (mRightMouseType == SELECT)
            {
                SelectControlPointByRectangle(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
                ClearRectangleRendering();
                UpdateControlRendering();
            }
            else if (mRightMouseType == DEFORM && mPickControlId >= 0)
            {
                UpdateDeformation(arg.state.X.abs, arg.state.Y.abs);
                mPickControlId = -1;
            }
            else if (mRightMouseType == MOVE && mPickControlId >= 0)
            {
                DragControlPoint(arg.state.X.abs, arg.state.Y.abs, true);
                mPickControlId = -1;
            }
        }
        return true;
    }

    bool AnimationApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_V && mpTriMesh !=NULL)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_POINTS);
        }
        else if (arg.key == OIS::KC_E && mpTriMesh !=NULL)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
        }
        else if (arg.key == OIS::KC_F && mpTriMesh !=NULL)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
        }
        return true;
    }

    bool AnimationApp::ImportModel()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0ASC Files(*.asc)\0*.asc\0Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
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
                    triMesh->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                    triMesh->UpdateNormal();
                    GPPFREEPOINTER(mpTriMesh);
                    mpTriMesh = triMesh;
                    
                    // Clear Data
                    ClearPointCloudData();
                    mDeformation->Clear();
                    mControlIds.clear();

                    // Update
                    UpdateModelRendering();
                    UpdateControlRendering();

                    return true;
                }
            }
            else if (extName == std::string("asc") || extName == std::string("gpc"))
            {
                GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileName);
                if (pointCloud != NULL)
                {
                    pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                    GPPFREEPOINTER(mpPointCloud);
                    mpPointCloud = pointCloud;
                    
                    // Clear Data
                    ClearMeshData();
                    mDeformation->Clear();
                    mControlIds.clear();

                    // Update
                    UpdateModelRendering();
                    UpdateControlRendering();

                    return true;
                }
            }
        }
        return false;
    }

    void AnimationApp::InitDeformation(int controlPointCount)
    {
        if (mpPointCloud == NULL && mpTriMesh == NULL)
        {
            MessageBox(NULL, "请先导入点云或者网格模型", "温馨提示", MB_OK);
            return;
        }
        GPP::ErrorCode res = GPP_NO_ERROR;
        if (mpPointCloud)
        {
            res = mDeformation->Init(mpPointCloud, controlPointCount);
        }
        else if (mpTriMesh)
        {
            res = mDeformation->Init(mpTriMesh, controlPointCount);
        }
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "变形初始化失败", "温馨提示", MB_OK);
            return;
        }
        mIsDeformationInitialised = true;
        mDeformation->GetControlIds(mControlIds);
        mControlNeighbors = mDeformation->GetControlNeighbors();
        mControlFixFlags = std::vector<bool>(mControlIds.size(), 1);
        UpdateControlRendering();
    }

    void AnimationApp::SelectFreeControlPoint()
    {
        mRightMouseType = SELECT;
        mAddSelection = true;
        if (mFirstAlert)
        {
            mFirstAlert = false;
            MessageBox(NULL, "鼠标右键进入框选控制点模式(+)，点击选择按钮返回移动控制点模式", "温馨提示", MB_OK);
        }
    }
        
    void AnimationApp::ClearFreeControlPoint()
    {
        mRightMouseType = SELECT;
        mAddSelection = false;
        if (mFirstAlert)
        {
            mFirstAlert = false;
            MessageBox(NULL, "鼠标右键进入框选控制点模式(-)，点击选择按钮返回移动控制点模式", "温馨提示", MB_OK);
        }
    }

    void AnimationApp::DeformControlPoint()
    {
        mRightMouseType = DEFORM;
        mTargetControlCoords.clear();
        mTargetControlIds.clear();
    }

    void AnimationApp::MoveControlPoint()
    {
        mRightMouseType = MOVE;
        if (mTargetControlCoords.size() > 0)
        {
            GPP::ErrorCode res = mDeformation->Deform(mTargetControlIds, mTargetControlCoords, mControlFixFlags);
            mTargetControlIds.clear();
            mTargetControlCoords.clear();
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "三维模型变形失败", "温馨提示", MB_OK);
                return;
            }
            UpdateModelRendering();
            UpdateControlRendering();
        }
    }
        
    void AnimationApp::AppJump()
    {
        if (mpTriMesh)
        {
            GPP::TriMesh* copiedTriMesh = GPP::CopyTriMesh(mpTriMesh);
            AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
            MeshShopApp* meshShop = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
            if (meshShop)
            {
                meshShop->SetMesh(copiedTriMesh, mObjCenterCoord, mScaleValue);
            }
            else
            {
                GPPFREEPOINTER(copiedTriMesh);
            }
        }
        else if (mpPointCloud)
        {
            GPP::PointCloud* copiedPointCloud = GPP::CopyPointCloud(mpPointCloud);
            AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
            PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
            if (pointShop)
            {
                pointShop->SetPointCloud(copiedPointCloud, mObjCenterCoord, mScaleValue);
            }
            else
            {
                GPPFREEPOINTER(copiedPointCloud);
            }
        }
    }

    void AnimationApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("AnimationApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);

        InitViewTool();

        mDeformation = new GPP::DeformPointList;
    }

    void AnimationApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("AnimationApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        }
        MagicCore::RenderSystem::Get()->HideRenderingObject("Model_AnimationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("ControlPoint_Free_AnimationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("ControlPoint_Fix_AnimationApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("ControlPoint_Target_AnimationApp");

        GPPFREEPOINTER(mDeformation);
    }

    void AnimationApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    void AnimationApp::UpdateModelRendering()
    {
        if (mpTriMesh)
        {
            MagicCore::RenderSystem::Get()->RenderMesh("Model_AnimationApp", "CookTorrance", mpTriMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else if (mpPointCloud)
        {
            if (mpPointCloud->HasNormal())
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("Model_AnimationApp", "CookTorrancePoint", mpPointCloud);
            }
            else
            {
                MagicCore::RenderSystem::Get()->RenderPointCloud("Model_AnimationApp", "SimplePoint", mpPointCloud);
            }
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("Model_AnimationApp");
        }
    }

    void AnimationApp::UpdateControlRendering()
    {
        if (mControlIds.empty())
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("ControlPoint_Free_AnimationApp");
            MagicCore::RenderSystem::Get()->HideRenderingObject("ControlPoint_Fix_AnimationApp");
        }
        else
        {
            std::vector<GPP::Vector3> fixCoords;
            std::vector<GPP::Vector3> freeCoords;
            std::vector<GPP::Vector3> targetCoords;
            if (mpTriMesh)
            {
                for (int cid = 0; cid < mControlIds.size(); cid++)
                {
                    if (mControlFixFlags.at(cid))
                    {
                        fixCoords.push_back(mpTriMesh->GetVertexCoord(mControlIds.at(cid)));
                    }
                    else
                    {
                        freeCoords.push_back(mpTriMesh->GetVertexCoord(mControlIds.at(cid)));
                    }
                }
                if (mPickControlId != -1)
                {
                    freeCoords.push_back(mPickTargetCoord);
                }
            }
            else if (mpPointCloud)
            {
                for (int cid = 0; cid < mControlIds.size(); cid++)
                {
                    if (mControlFixFlags.at(cid))
                    {
                        fixCoords.push_back(mpPointCloud->GetPointCoord(mControlIds.at(cid)));
                    }
                    else
                    {
                        freeCoords.push_back(mpPointCloud->GetPointCoord(mControlIds.at(cid)));
                    }
                }
                if (mPickControlId != -1)
                {
                    freeCoords.push_back(mPickTargetCoord);
                }
            }            
            MagicCore::RenderSystem::Get()->RenderPointList("ControlPoint_Free_AnimationApp", "SimplePoint_Large", GPP::Vector3(0, 0, 1), freeCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->RenderPointList("ControlPoint_Fix_AnimationApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), fixCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            if (mTargetControlCoords.size() > 0)
            {
                MagicCore::RenderSystem::Get()->RenderPointList("ControlPoint_Target_AnimationApp", "SimplePoint_Large", GPP::Vector3(0, 1, 0), mTargetControlCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            }
            else
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("ControlPoint_Target_AnimationApp");
            }
        }
    }
}
