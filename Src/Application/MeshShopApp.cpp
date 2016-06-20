#include "stdafx.h"
#include <process.h>
#include "MeshShopApp.h"
#include "MeshShopAppUI.h"
#include "PointShopApp.h"
#include "ReliefApp.h"
#include "TextureApp.h"
#include "MeasureApp.h"
#include "AppManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#if DEBUGDUMPFILE
#include "DumpFillMeshHole.h"
#endif

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        MeshShopApp* app = (MeshShopApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    MeshShopApp::MeshShopApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mpViewTool(NULL),
        mDisplayMode(0),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mShowHoleLoopIds(),
        mBoundarySeedIds(),
        mTargetVertexCount(0),
        mFillHoleType(0),
        mCommandType(NONE),
        mUpdateMeshRendering(false),
        mUpdateHoleRendering(false),
        mIsCommandInProgress(false),
        mVertexSelectFlag(),
        mRightMouseType(MOVE),
        mMousePressdCoord(),
        mIgnoreBack(true),
        mFilterPositionWeight(1.0)
    {
    }

    MeshShopApp::~MeshShopApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
    }

    bool MeshShopApp::Enter()
    {
        InfoLog << "Enter MeshShopApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new MeshShopAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool MeshShopApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            int progressValue = int(GPP::GetApiProgress() * 100.0);
            mpUI->SetProgressbar(progressValue);
        }
        if (mUpdateMeshRendering)
        {
            UpdateMeshRendering();
            mUpdateMeshRendering = false;
        }
        if (mUpdateHoleRendering)
        {
            UpdateHoleRendering();
            mUpdateHoleRendering = false;
        }
        if (mUpdateBridgeRendering)
        {
            UpdateBridgeRendering();
            mUpdateBridgeRendering = false;
        }
        return true;
    }

    bool MeshShopApp::Exit()
    {
        InfoLog << "Exit MeshShopApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool MeshShopApp::MouseMoved( const OIS::MouseEvent &arg )
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
        else if ((mRightMouseType == SELECT_ADD || mRightMouseType == SELECT_DELETE || mRightMouseType == SELECT_BRIDGE) && arg.state.buttonDown(OIS::MB_Right))
        {
            UpdateRectangleRendering(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
        }

        return true;
    }

    bool MeshShopApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left || (id == OIS::MB_Right && mRightMouseType == MOVE)) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (arg.state.buttonDown(OIS::MB_Right) && mpTriMesh && (mRightMouseType == SELECT_ADD || mRightMouseType == SELECT_DELETE || mRightMouseType == SELECT_BRIDGE))
        {
            mMousePressdCoord[0] = arg.state.X.abs;
            mMousePressdCoord[1] = arg.state.Y.abs;
        }
        return true;
    }

    bool MeshShopApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left || (id == OIS::MB_Right && mRightMouseType == MOVE)) && mpViewTool != NULL)
        {
            mpViewTool->MouseReleased(); 
        }
        else if (id == OIS::MB_Right && mpTriMesh && (mRightMouseType == SELECT_ADD || mRightMouseType == SELECT_DELETE))
        {
            SelectControlPointByRectangle(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
            ClearRectangleRendering();
            UpdateMeshRendering();
        }
        else if (id == OIS::MB_Right && mpTriMesh && (mRightMouseType == SELECT_BRIDGE))
        {
            SelectControlPointByRectangle(mMousePressdCoord[0], mMousePressdCoord[1], arg.state.X.abs, arg.state.Y.abs);
            ClearRectangleRendering();
            DoBridgeEdges();
            UpdateMeshRendering();
            UpdateBridgeRendering();
        }

        return true;
    }

    bool MeshShopApp::KeyPressed( const OIS::KeyEvent &arg )
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
        else if (arg.key == OIS::KC_D)
        {
#if DEBUGDUMPFILE
            RunDumpInfo();
#endif
        }
        return true;
    }
    
    void MeshShopApp::WindowFocusChanged( Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
    }

    void MeshShopApp::DoCommand(bool isSubThread)
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
            case MagicApp::MeshShopApp::NONE:
                break;
            case MagicApp::MeshShopApp::EXPORTMESH:
                ExportMesh(false);
                break;
            case MagicApp::MeshShopApp::CONSOLIDATETOPOLOGY:
                ConsolidateTopology(false);
                break;
            case MagicApp::MeshShopApp::CONSOLIDATEGEOMETRY:
                ConsolidateGeometry(false);
                break;
            case MagicApp::MeshShopApp::REMOVEISOLATEPART:
                RemoveMeshIsolatePart(false);
                break;
            case MagicApp::MeshShopApp::REMOVEMESHNOISE:
                RemoveMeshNoise(mFilterPositionWeight, false);
                break;
            case MagicApp::MeshShopApp::SMOOTHMESH:
                SmoothMesh(mFilterPositionWeight, false);
                break;
            case MagicApp::MeshShopApp::ENHANCEDETAIL:
                EnhanceMeshDetail(false);
                break;
            case MagicApp::MeshShopApp::LOOPSUBDIVIDE:
                LoopSubdivide(false);
                break;
            case MagicApp::MeshShopApp::REFINE:
                RefineMesh(mTargetVertexCount, false);
                break;
            case MagicApp::MeshShopApp::SIMPLIFY:
                SimplifyMesh(mTargetVertexCount, false);
                break;
            case MagicApp::MeshShopApp::FILLHOLE:
                FillHole(mFillHoleType, false);
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
        }
    }

    void MeshShopApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("MeshShop_SimpleLight");
        light->setPosition(-5, 5, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
    }

    void MeshShopApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("MeshShop_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_MeshShop");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshShop_Holes");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshShop_HoleSeeds");
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        } 
    }

    void MeshShopApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        mShowHoleLoopIds.clear();
        mVertexSelectFlag.clear();
        mRightMouseType = MOVE;
    }

    bool MeshShopApp::IsCommandAvaliable()
    {
        if (mpTriMesh == NULL)
        {
            MessageBox(NULL, "请先导入网格", "温馨提示", MB_OK);
            return false;
        }
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        return true;
    }

    void MeshShopApp::ResetSelection()
    {
        if (mpTriMesh)
        {
            mVertexSelectFlag = std::vector<bool>(mpTriMesh->GetVertexCount(), 0);
        }
        else
        {
            mVertexSelectFlag.clear();
        }
    }

    void MeshShopApp::SelectControlPointByRectangle(int startCoordX, int startCoordY, int endCoordX, int endCoordY)
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
        GPP::Int vertexCount = mpTriMesh->GetVertexCount();
        GPP::Vector3 coord;
        GPP::Vector3 normal;
        bool ignoreBack = mIgnoreBack;
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            if (mRightMouseType == SELECT_ADD && mVertexSelectFlag.at(vid))
            {
                continue;
            }
            else if (mRightMouseType == SELECT_DELETE && mVertexSelectFlag.at(vid) == 0)
            {
                continue;
            }
            GPP::Vector3 coord = mpTriMesh->GetVertexCoord(vid);
            Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
            ogreCoord = wvpM * ogreCoord;
            if (ogreCoord.x > minX && ogreCoord.x < maxX && ogreCoord.y > minY && ogreCoord.y < maxY)
            {
                if (ignoreBack)
                {
                    normal = mpTriMesh->GetVertexNormal(vid);
                    Ogre::Vector4 ogreNormal(normal[0], normal[1], normal[2], 0);
                    ogreNormal = worldM * ogreNormal;
                    if (ogreNormal.z > 0)
                    {
                        if (mRightMouseType == SELECT_BRIDGE)
                        {
                            mVertexBridgeFlag.at(vid) = 1;
                        }
                        else if (mRightMouseType == SELECT_ADD)
                        {
                            mVertexSelectFlag.at(vid) = 1;
                        }
                        else
                        {
                            mVertexSelectFlag.at(vid) = 0;
                        }
                    }
                }
                else
                {
                    if (mRightMouseType == SELECT_BRIDGE)
                    {
                        mVertexBridgeFlag.at(vid) = 1;
                    }
                    else if (mRightMouseType == SELECT_ADD)
                    {
                        mVertexSelectFlag.at(vid) = 1;
                    }
                    else
                    {
                        mVertexSelectFlag.at(vid) = 0;
                    }
                }
            }
        }
    }

    void MeshShopApp::UpdateRectangleRendering(int startCoordX, int startCoordY, int endCoordX, int endCoordY)
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

    void MeshShopApp::ClearRectangleRendering()
    {
        MagicCore::RenderSystem::Get()->HideRenderingObject("PickRectangleObj");
    }

    void MeshShopApp::SwitchDisplayMode()
    {
        mDisplayMode++;
        if (mDisplayMode > 2)
        {
            mDisplayMode = 0;
        }
        if (mDisplayMode == 0)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
        }
        else if (mDisplayMode == 1)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
        }
        else if (mDisplayMode == 2)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_POINTS);
        }
    }

    bool MeshShopApp::ImportMesh()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
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
                InfoLog << "Import Mesh,  vertex: " << mpTriMesh->GetVertexCount() << " triangles: " << triMesh->GetTriangleCount() << std::endl;
                InitViewTool();
                ResetSelection();
                mRightMouseType = MOVE;
                mShowHoleLoopIds.clear();
                UpdateHoleRendering();
                UpdateMeshRendering();
                mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
                mpUI->ResetFillHole();
                return true;
            }
            else
            {
                mpUI->SetMeshInfo(0, 0);
                MessageBox(NULL, "网格导入失败", "温馨提示", MB_OK);
            }
        }
        return false;
    }

    void MeshShopApp::ExportMesh(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0PLY Files(*.ply)\0*.ply\0OFF Files(*.off)\0*.off\0";
        if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
        {
            size_t dotPos = fileName.rfind('.');
            if (dotPos == std::string::npos)
            {
                MessageBox(NULL, "请输入文件后缀名", "温馨提示", MB_OK);
                return;
            }
            std::string extName = fileName.substr(dotPos + 1);
            GPP::ErrorCode res = GPP_NO_ERROR;
            if (extName == "inc")
            {
                res = GPP::Parser::ExportTriMeshToPovray(fileName, mpTriMesh, NULL);
                std::vector<GPP::Vector3> lineSegments;
                GPP::Int vertexIds[3] = {-1};
                int faceCount = mpTriMesh->GetTriangleCount();
                for (int fid = 0; fid < faceCount; fid++)
                {
                    mpTriMesh->GetTriangleVertexIds(fid, vertexIds);
                    lineSegments.push_back(mpTriMesh->GetVertexCoord(vertexIds[0]));
                    lineSegments.push_back(mpTriMesh->GetVertexCoord(vertexIds[1]));
                    lineSegments.push_back(mpTriMesh->GetVertexCoord(vertexIds[1]));
                    lineSegments.push_back(mpTriMesh->GetVertexCoord(vertexIds[2]));
                    lineSegments.push_back(mpTriMesh->GetVertexCoord(vertexIds[2]));
                    lineSegments.push_back(mpTriMesh->GetVertexCoord(vertexIds[0]));
                }
                res = GPP::Parser::ExportLineSegmentToPovray("edge.inc", lineSegments, 0.0025, GPP::Vector3(0.09, 0.48627, 0.69));
            }
            else
            {
                mpTriMesh->UnifyCoords(1.0 / mScaleValue, mObjCenterCoord * (-mScaleValue));
                res = GPP::Parser::ExportTriMesh(fileName, mpTriMesh);
                mpTriMesh->UnifyCoords(mScaleValue, mObjCenterCoord);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格导出失败", "温馨提示", MB_OK);
            }
        }
    }

    void MeshShopApp::SetMesh(GPP::TriMesh* triMesh, GPP::Vector3 objCenterCoord, GPP::Real scaleValue)
    {
        if (!mpTriMesh)
        {
            delete mpTriMesh;
        }
        mpTriMesh = triMesh;
        mObjCenterCoord = objCenterCoord;
        mScaleValue = scaleValue;
        ResetSelection();
        mRightMouseType = MOVE;
        InitViewTool();
        UpdateMeshRendering();
        if (mpTriMesh)
        {
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
        }
    }

    static void CollectTriMeshVerticesColorFields(const GPP::TriMesh* triMesh, std::vector<GPP::Real> *vertexColorFields)
    {
        if (triMesh == NULL || vertexColorFields == NULL)
        {
            return;
        }
        GPP::Int verticeSize = triMesh->GetVertexCount();
        GPP::Int dim = 3;
        vertexColorFields->resize(verticeSize * dim, 0.0);
        for (GPP::Int vid = 0; vid < verticeSize; ++vid)
        {
            GPP::Vector3 color = triMesh->GetVertexColor(vid);
            GPP::Int offset = vid * 3;
            vertexColorFields->at(offset) = color[0];
            vertexColorFields->at(offset+1) = color[1];
            vertexColorFields->at(offset+2) = color[2];
        }
    }

    static GPP::Vector3 TrimVector(const GPP::Vector3& vec, GPP::Real minValue, GPP::Real maxValue)
    {
        GPP::Vector3 out(vec);
        for (GPP::Int ii = 0; ii < 3; ++ii)
        {
            if (out[ii] < minValue)
            {
                out[ii] = minValue;
            }
            else if (out[ii] > maxValue)
            {
                out[ii] = maxValue;
            }
        }
        return out;
    }

    static void UpdateTriMeshVertexColors(GPP::TriMesh *triMesh, GPP::Int oldTriMeshVertexSize, const std::vector<GPP::Real>& insertedVertexFields)
    {
        if (triMesh == NULL || insertedVertexFields.empty())
        {
            return;
        }
        GPP::Int verticeSize = triMesh->GetVertexCount();
        GPP::Int insertedVertexSize = (triMesh->GetVertexCount() - oldTriMeshVertexSize);
        if (insertedVertexFields.size() != insertedVertexSize * 3)
        {
            return;
        }
        for (GPP::Int vid = 0; vid < insertedVertexSize; ++vid)
        {
            GPP::Int newVertexId = vid + oldTriMeshVertexSize;
            GPP::Vector3 color(insertedVertexFields[vid * 3 + 0], insertedVertexFields[vid * 3 + 1], insertedVertexFields[vid * 3 + 2]);
            triMesh->SetVertexColor(newVertexId, TrimVector(color, 0.0, 1.0));
        }
    }

#if DEBUGDUMPFILE
    void MeshShopApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
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
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = CopyTriMesh(mpDumpInfo->GetTriMesh());
        InitViewTool();
        if (mpDumpInfo->GetApiName() == GPP::ApiName::MESH_FILLHOLE_FILL)
        {
            GPP::DumpFillMeshHole* dumpDetail = dynamic_cast<GPP::DumpFillMeshHole*>(mpDumpInfo);
            if (dumpDetail)
            {
                SetBoundarySeedIds(dumpDetail->GetBoundarySeedIds());
                UpdateHoleRendering();
            }
        }
        ResetSelection();
        UpdateMeshRendering();
    }

    void MeshShopApp::RunDumpInfo()
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

        //Copy result
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = CopyTriMesh(mpDumpInfo->GetTriMesh());
        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();

        // update hole rendering
        if (mpDumpInfo->GetApiName() == GPP::ApiName::MESH_FILLHOLE_FIND)
        {
            GPP::DumpFindMeshHole* dumpDetails = dynamic_cast<GPP::DumpFindMeshHole*>(mpDumpInfo);
            if (dumpDetails)
            {
                SetToShowHoleLoopVrtIds(dumpDetails->GetBoundaryLoopVrtIds());
                SetBoundarySeedIds(std::vector<GPP::Int>());
                UpdateHoleRendering();
            }
        }
        else if (mpDumpInfo->GetApiName() == GPP::ApiName::MESH_FILLHOLE_FILL)
        {
            GPP::DumpFillMeshHole* dumpDetails = dynamic_cast<GPP::DumpFillMeshHole*>(mpDumpInfo);
            if (dumpDetails)
            {
                SetToShowHoleLoopVrtIds(std::vector<std::vector<GPP::Int> >());
                SetBoundarySeedIds(std::vector<GPP::Int>());
                UpdateHoleRendering();
            }
        }
        ResetSelection();
        UpdateMeshRendering();
        GPPFREEPOINTER(mpDumpInfo);
        
    }
#endif

    bool MeshShopApp::IsCommandInProgress(void)
    {
        return mIsCommandInProgress;
    }

    void MeshShopApp::ConsolidateTopology(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = CONSOLIDATETOPOLOGY;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidateMesh::MakeTriMeshManifold(mpTriMesh);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "拓扑修复失败", "温馨提示", MB_OK);
                return;
            }
            ResetSelection();
            bool isManifold = GPP::ConsolidateMesh::_IsTriMeshManifold(mpTriMesh);
            if (!isManifold)
            {
                MessageBox(NULL, "拓扑修复后，网格仍然是非流形结构", "温馨提示", MB_OK);
                return;
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering= true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
            mpUI->ResetFillHole();
            FindHole(false);
        }
    }

    void MeshShopApp::ReverseDirection()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::Int faceCount = mpTriMesh->GetTriangleCount();
        GPP::Int vertexIds[3];
        for (GPP::Int fid = 0; fid < faceCount; fid++)
        {
            mpTriMesh->GetTriangleVertexIds(fid, vertexIds);
            mpTriMesh->SetTriangleVertexIds(fid, vertexIds[1], vertexIds[0], vertexIds[2]);
        }
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::RemoveMeshIsolatePart(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = REMOVEISOLATEPART;
            DoCommand(true);
        }
        else
        {
            GPP::Int vertexCount = mpTriMesh->GetVertexCount();
            if (vertexCount < 1)
            {
                MessageBox(NULL, "网格顶点个数小于1，操作失败", "温馨提示", MB_OK);
                return;
            }
            std::vector<GPP::Real> isolation;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidateMesh::CalculateIsolation(mpTriMesh, &isolation);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格光顺失败", "温馨提示", MB_OK);
                return;
            }
            ResetSelection();
            GPP::Real cutValue = 0.10;
            std::vector<GPP::Int> deleteIndex;
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                if (isolation[vid] < cutValue)
                {
                    deleteIndex.push_back(vid);
                }
            }
            DebugLog << "MeshShopApp::RemoveOutlier deleteIndex size=" << deleteIndex.size() << std::endl;
            res = GPP::DeleteTriMeshVertices(mpTriMesh, deleteIndex);
            if (res != GPP_NO_ERROR)
            {
                return;
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
            mpUI->ResetFillHole();
            FindHole(false);
        }
    }

    void MeshShopApp::ConsolidateGeometry(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = CONSOLIDATEGEOMETRY;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::ConsolidateMesh::ConsolidateGeometry(mpTriMesh, GPP::ONE_RADIAN * 5.0, GPP::REAL_TOL, GPP::ONE_RADIAN * 170.0);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "几何修复失败", "温馨提示", MB_OK);
                return;
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
        }
    }

    void MeshShopApp::RemoveMeshNoise(double positionWeight, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = REMOVEMESHNOISE;
            mFilterPositionWeight = positionWeight;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            bool isVertexSelected = false;
            for (std::vector<bool>::iterator itr = mVertexSelectFlag.begin(); itr != mVertexSelectFlag.end(); ++itr)
            {
                if (*itr)
                {
                    isVertexSelected = true;
                    break;
                }
            }
            GPP::ErrorCode res = GPP_NO_ERROR;
            if (isVertexSelected)
            {
                GPP::SubTriMesh subTriMesh(mpTriMesh, mVertexSelectFlag, GPP::SubTriMesh::BUILD_SUBTRIMESH_TYPE_BY_VERTICES);
                res = GPP::ConsolidateMesh::RemoveGeometryNoise(&subTriMesh, 70.0 * GPP::ONE_RADIAN, positionWeight);
            }
            else
            {
                res = GPP::ConsolidateMesh::RemoveGeometryNoise(mpTriMesh, 70.0 * GPP::ONE_RADIAN, positionWeight);
            }
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格光顺失败", "温馨提示", MB_OK);
                return;
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
        }
    }

    void MeshShopApp::SmoothMesh(double positionWeight, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = SMOOTHMESH;
            mFilterPositionWeight = positionWeight;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            bool isVertexSelected = false;
            for (std::vector<bool>::iterator itr = mVertexSelectFlag.begin(); itr != mVertexSelectFlag.end(); ++itr)
            {
                if (*itr)
                {
                    isVertexSelected = true;
                    break;
                }
            }
            GPP::ErrorCode res = GPP_NO_ERROR;
            if (isVertexSelected)
            {
                GPP::SubTriMesh subTriMesh(mpTriMesh, mVertexSelectFlag, GPP::SubTriMesh::BUILD_SUBTRIMESH_TYPE_BY_VERTICES);
                res = GPP::FilterMesh::LaplaceSmooth(&subTriMesh, true, positionWeight);
            }
            else
            {
                res = GPP::FilterMesh::LaplaceSmooth(mpTriMesh, true, positionWeight);
            }
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格光顺失败", "温馨提示", MB_OK);
                return;
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
        }
    }

    void MeshShopApp::EnhanceMeshDetail(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = ENHANCEDETAIL;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;
            bool isVertexSelected = false;
            for (std::vector<bool>::iterator itr = mVertexSelectFlag.begin(); itr != mVertexSelectFlag.end(); ++itr)
            {
                if (*itr)
                {
                    isVertexSelected = true;
                    break;
                }
            }
            GPP::ErrorCode res = GPP_NO_ERROR;
            if (isVertexSelected)
            {
                GPP::SubTriMesh subTriMesh(mpTriMesh, mVertexSelectFlag, GPP::SubTriMesh::BUILD_SUBTRIMESH_TYPE_BY_VERTICES);
                res = GPP::FilterMesh::EnhanceDetail(&subTriMesh);
            }
            else
            {
                res = GPP::FilterMesh::EnhanceDetail(mpTriMesh);
            }
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "几何细节增强失败", "温馨提示", MB_OK);
                return;
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
        }
    }

    void MeshShopApp::LoopSubdivide(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mCommandType = LOOPSUBDIVIDE;
            DoCommand(true);
        }
        else
        {
            GPP::Int vertexCount = mpTriMesh->GetVertexCount();
            if (vertexCount >= 250000)
            {
                if (MessageBox(NULL, "细分后网格顶点数量可能会大于1M，是否继续", "温馨提示", MB_OKCANCEL) != IDOK)
                {
                    return;
                }
            }
            std::vector<GPP::Real> vertexFields(vertexCount * 3);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 color = mpTriMesh->GetVertexColor(vid);
                GPP::Int baseId = vid * 3;
                vertexFields.at(baseId) = color[0];
                vertexFields.at(baseId + 1) = color[1];
                vertexFields.at(baseId + 2) = color[2];
            }
            std::vector<GPP::Real> insertedVertexFields;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::SubdivideMesh::LoopSubdivideMesh(mpTriMesh, &vertexFields, &insertedVertexFields);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "Loop细分失败", "温馨提示", MB_OK);
                return;
            }
            ResetSelection();
            GPP::Int insertedVertexCount = mpTriMesh->GetVertexCount() - vertexCount;
            for (GPP::Int vid = 0; vid < insertedVertexCount; vid++)
            {
                GPP::Int baseId = vid * 3;
                mpTriMesh->SetVertexColor(vertexCount + vid, GPP::Vector3(insertedVertexFields.at(baseId), insertedVertexFields.at(baseId + 1), insertedVertexFields.at(baseId + 2)));
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
        }
    }

    void MeshShopApp::RefineMesh(int targetVertexCount, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mTargetVertexCount = targetVertexCount;
            mCommandType = REFINE;
            DoCommand(true);
        }
        else
        {
            if (targetVertexCount > 1000000)
            {
                if (MessageBox(NULL, "加密后网格顶点数量会大于1M，是否继续", "温馨提示", MB_OKCANCEL) != IDOK)
                {
                    return;
                }
            }
            GPP::Int vertexCount = mpTriMesh->GetVertexCount();
            std::vector<GPP::Real> vertexFields(vertexCount * 3);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 color = mpTriMesh->GetVertexColor(vid);
                GPP::Int baseId = vid * 3;
                vertexFields.at(baseId) = color[0];
                vertexFields.at(baseId + 1) = color[1];
                vertexFields.at(baseId + 2) = color[2];
            }
            std::vector<GPP::Real> insertedVertexFields;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::SubdivideMesh::DensifyMesh(mpTriMesh, targetVertexCount, &vertexFields, &insertedVertexFields);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格加密失败", "温馨提示", MB_OK);
                return;
            }
            ResetSelection();
            GPP::Int insertedVertexCount = mpTriMesh->GetVertexCount() - vertexCount;
            for (GPP::Int vid = 0; vid < insertedVertexCount; vid++)
            {
                GPP::Int baseId = vid * 3;
                mpTriMesh->SetVertexColor(vertexCount + vid, GPP::Vector3(insertedVertexFields.at(baseId), insertedVertexFields.at(baseId + 1), insertedVertexFields.at(baseId + 2)));
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
        }
    }

    void MeshShopApp::SimplifyMesh(int targetVertexCount, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mTargetVertexCount = targetVertexCount;
            mCommandType = SIMPLIFY;
            DoCommand(true);
        }
        else
        {
            if (GPP::ConsolidateMesh::_IsTriMeshManifold(mpTriMesh) == false)
            {
                MessageBox(NULL, "警告：网格有非流形结构，请先拓扑修复，否则程序会出错", "温馨提示", MB_OK);
                return;
            }
            std::vector<GPP::Real> vertexFields;
            CollectTriMeshVerticesColorFields(mpTriMesh, &vertexFields);
            std::vector<GPP::Real> simplifiedVertexFields;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::SimplifyMesh::QuadricSimplify(mpTriMesh, targetVertexCount, false, &vertexFields, &simplifiedVertexFields);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格简化失败", "温馨提示", MB_OK);
                return;
            }
            ResetSelection();
            GPP::Int vertexCount = mpTriMesh->GetVertexCount();
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Int baseIndex = vid * 3;
                mpTriMesh->SetVertexColor(vid, GPP::Vector3(simplifiedVertexFields.at(baseIndex), simplifiedVertexFields.at(baseIndex + 1), simplifiedVertexFields.at(baseIndex + 2)));
            }
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
            mpUI->ResetFillHole();
            FindHole(false);
        }
    }

    void MeshShopApp::SampleMesh()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return;
        }
        if (mpTriMesh == NULL)
        {
            AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
            return;
        }
        GPP::Int vertexCount = mpTriMesh->GetVertexCount();
        if (vertexCount < 1)
        {
            AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
            return;
        }
        GPP::PointCloud* pointCloud = new GPP::PointCloud;
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            pointCloud->InsertPoint(mpTriMesh->GetVertexCoord(vid), mpTriMesh->GetVertexNormal(vid));
            pointCloud->SetPointColor(vid, mpTriMesh->GetVertexColor(vid));
        }
        pointCloud->SetHasNormal(true);
        pointCloud->SetHasColor(true);
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop)
        {
            pointShop->SetPointCloud(pointCloud, mObjCenterCoord, mScaleValue);
        }
        else
        {
            delete pointCloud;
            pointCloud = NULL;
        }
    }

    void MeshShopApp::EnterReliefApp()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return;
        }
        if (mpTriMesh == NULL)
        {
            AppManager::Get()->EnterApp(new ReliefApp, "ReliefApp");
            return;
        }
        GPP::TriMesh* copyMesh = GPP::CopyTriMesh(mpTriMesh);
        AppManager::Get()->EnterApp(new ReliefApp, "ReliefApp");
        ReliefApp* reliefApp = dynamic_cast<ReliefApp*>(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp)
        {
            reliefApp->SetMesh(copyMesh, mObjCenterCoord, mScaleValue);
            copyMesh = NULL;
        }
        else
        {
            GPPFREEPOINTER(copyMesh);
        }
    }

    void MeshShopApp::EnterTextureApp()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return;
        }
        if (mpTriMesh == NULL)
        {
            AppManager::Get()->EnterApp(new TextureApp, "TextureApp");
            return;
        }
        GPP::TriMesh* copyMesh = GPP::CopyTriMesh(mpTriMesh);
        AppManager::Get()->EnterApp(new TextureApp, "TextureApp");
        TextureApp* textureApp = dynamic_cast<TextureApp*>(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp)
        {
            textureApp->SetMesh(copyMesh, mObjCenterCoord, mScaleValue);
            copyMesh = NULL;
        }
        else
        {
            GPPFREEPOINTER(copyMesh);
        }
    }

    void MeshShopApp::EnterMeasureApp()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return;
        }
        if (mpTriMesh == NULL)
        {
            AppManager::Get()->EnterApp(new MeasureApp, "MeasureApp");
            return;
        }
        GPP::TriMesh* copyMesh = GPP::CopyTriMesh(mpTriMesh);
        AppManager::Get()->EnterApp(new MeasureApp, "MeasureApp");
        MeasureApp* measureApp = dynamic_cast<MeasureApp*>(AppManager::Get()->GetApp("MeasureApp"));
        if (measureApp)
        {
            measureApp->SetMesh(copyMesh, mObjCenterCoord, mScaleValue);
            copyMesh = NULL;
        }
        else
        {
            GPPFREEPOINTER(copyMesh);
        }
    }

    void MeshShopApp::ResetBridgeTags()
    {
        if (mpTriMesh)
        {
            mVertexBridgeFlag = std::vector<bool>(mpTriMesh->GetVertexCount(), 0);
        }
        else
        {
            mVertexBridgeFlag.clear();
        }

    }

    void MeshShopApp::FindHole(bool isShowHoles)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        std::vector<std::vector<GPP::Int> > holeIds;
        if (!isShowHoles)
        {
            // clear the holes
            mRightMouseType = MOVE; 
            ResetBridgeTags();
            mBridgeEdgeVertices.clear();
            SetToShowHoleLoopVrtIds(holeIds);
            SetBoundarySeedIds(std::vector<GPP::Int>());
            UpdateBridgeRendering();
            UpdateHoleRendering();
            return;
        }
        GPP::ErrorCode res = GPP::FillMeshHole::FindHoles(mpTriMesh, &holeIds);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        SetToShowHoleLoopVrtIds(holeIds);
        SetBoundarySeedIds(std::vector<GPP::Int>());

        mpTriMesh->UpdateNormal();
        UpdateHoleRendering();
        UpdateMeshRendering();
    }

    void MeshShopApp::FillHole(int type, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (isSubThread)
        {
            mFillHoleType = type;
            mCommandType = FILLHOLE;
            DoCommand(true);
        }
        else
        {
            mIsCommandInProgress = true;

            std::vector<GPP::Real> vertexScaleFields, outputScaleFields;
            CollectTriMeshVerticesColorFields(mpTriMesh, &vertexScaleFields);
            GPP::Int oldTriMeshVertexSize = mpTriMesh->GetVertexCount();
            
            bool isHoleSelected = false;
            for (GPP::Int vLoop = 0; vLoop < mShowHoleLoopIds.size(); ++vLoop)
            {
                if (isHoleSelected)
                {
                    break;
                }
                for (std::vector<GPP::Int>::iterator loopItr = mShowHoleLoopIds.at(vLoop).begin(); loopItr != mShowHoleLoopIds.at(vLoop).end(); ++loopItr)
                {
                    if (mVertexSelectFlag.at(*loopItr))
                    {
                        isHoleSelected = true;
                        break;
                    }
                }
            }
            std::vector<GPP::Int> holeSeeds;
            if (isHoleSelected)
            {
                for (GPP::Int vLoop = 0; vLoop < mShowHoleLoopIds.size(); ++vLoop)
                {
                    for (std::vector<GPP::Int>::iterator loopItr = mShowHoleLoopIds.at(vLoop).begin(); loopItr != mShowHoleLoopIds.at(vLoop).end(); ++loopItr)
                    {
                        if (mVertexSelectFlag.at(*loopItr))
                        {
                            holeSeeds.push_back(*loopItr);
                            break;
                        }
                    }
                }
            }
            else
            {
                for (GPP::Int vLoop = 0; vLoop < mShowHoleLoopIds.size(); ++vLoop)
                {
                    if (mShowHoleLoopIds.at(vLoop).size() > 0)
                    {
                        holeSeeds.push_back(mShowHoleLoopIds.at(vLoop).at(0));
                    }
                }
            }
            GPP::ErrorCode res = GPP::FillMeshHole::FillHoles(mpTriMesh, &holeSeeds, GPP::FillMeshHoleType(mFillHoleType),
                &vertexScaleFields, &outputScaleFields);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格补洞失败", "温馨提示", MB_OK);
                return;
            }
            ResetSelection();
            UpdateTriMeshVertexColors(mpTriMesh, oldTriMeshVertexSize, outputScaleFields);
            SetToShowHoleLoopVrtIds(std::vector<std::vector<GPP::Int> >());
            SetBoundarySeedIds(std::vector<GPP::Int>());
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
            mUpdateHoleRendering = true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
            mpUI->ResetFillHole();
        }
    }

    void MeshShopApp::DoBridgeEdges()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        
        if (mBridgeEdgeVertices.size() == 0 || mBridgeEdgeVertices.size() == 2)
        {
            for (int holeLoopId = 0; holeLoopId < mShowHoleLoopIds.size(); ++holeLoopId)
            {
                bool isHoleSelected = false;
                std::vector<bool> isSelectedTags(mShowHoleLoopIds.at(holeLoopId).size(), false);
                for (int vid = 0; vid < mShowHoleLoopIds.at(holeLoopId).size(); ++vid)
                {
                    GPP::Int vertexId = mShowHoleLoopIds.at(holeLoopId).at(vid);
                    if (mVertexBridgeFlag.at(vertexId))
                    {
                        isSelectedTags.at(vid) = true;
                        isHoleSelected = true;
                    }
                }
                if (isHoleSelected)
                {
                    // find the "most center" id
                    int allCount = isSelectedTags.size();
                    int longestStartId = -1, longestEndId = -1;
                    int tmpStartId = -1, tmpEndId = -1;
                    int longestSize = 0;
                    int currentId = 0;
                    bool bFind = false;
                    while (currentId != isSelectedTags.size() - 2)
                    {
                        if (!isSelectedTags.at(currentId) && isSelectedTags.at(currentId + 1))
                        {
                            bFind = true;
                            break;
                        }
                        currentId++;
                    }
                    if (!bFind)
                    {
                        mBridgeEdgeVertices.push_back(mShowHoleLoopIds.at(holeLoopId).at(0));
                        mBridgeEdgeVertices.push_back(mShowHoleLoopIds.at(holeLoopId).at(1));
                        break;
                    }

                    //
                    currentId ++;
                    int startId = currentId;
                    do 
                    {
                        if (!isSelectedTags.at(currentId))
                        {
                            currentId ++;
                            if (currentId >= allCount)
                            {
                                currentId = 0;
                            }
                            continue;
                        }

                        tmpStartId = currentId;
                        do 
                        {
                            int nextId = currentId + 1;
                            if (nextId >= allCount)
                            {
                                nextId = 0;
                            }
                            if (!isSelectedTags.at(nextId))
                            {
                                int count = (nextId - tmpStartId + allCount) % allCount;
                                if (count > longestSize)
                                {
                                    longestStartId = tmpStartId;
                                    longestEndId = currentId;
                                    longestSize = count;
                                }
                                break;
                            }

                            currentId ++;
                            if (currentId >= allCount)
                            {
                                currentId = 0;
                            }
                        } while (currentId != tmpStartId);
                        currentId ++;
                        if (currentId >= allCount)
                        {
                            currentId = 0;
                        }
                    } while (currentId != startId);
                    if (longestSize <= 2)
                    {
                        mBridgeEdgeVertices.push_back(mShowHoleLoopIds.at(holeLoopId).at(longestStartId));
                        mBridgeEdgeVertices.push_back(mShowHoleLoopIds.at(holeLoopId).at((longestStartId + 1) % allCount));
                    }
                    else
                    {
                        int centerId = (longestStartId + longestEndId) / 2;
                        if (longestStartId > longestEndId)
                        {
                            centerId = (longestStartId + longestEndId + allCount) / 2;
                            centerId %= allCount;
                        }
                        int nextId = (centerId + 1) % allCount;
                        mBridgeEdgeVertices.push_back(mShowHoleLoopIds.at(holeLoopId).at(centerId));
                        mBridgeEdgeVertices.push_back(mShowHoleLoopIds.at(holeLoopId).at(nextId));
                    }
                    break;
                }
            }
            mUpdateBridgeRendering = true;
        }
        else
        {
            ResetBridgeTags();
        }

        if (mBridgeEdgeVertices.size() == 2)
        {
            ResetBridgeTags();
        }
        else if (mBridgeEdgeVertices.size() == 4)
        {
            GPP::Int edgeVertex1[2] = {mBridgeEdgeVertices[0], mBridgeEdgeVertices[1]};
            GPP::Int edgeVertex2[2] = {mBridgeEdgeVertices[2], mBridgeEdgeVertices[3]};
            std::vector<GPP::Real> vertexScaleFields, outputScaleFields;
            CollectTriMeshVerticesColorFields(mpTriMesh, &vertexScaleFields);
            GPP::Int oldTriMeshVertexSize = mpTriMesh->GetVertexCount();

            GPP::ErrorCode res = GPP::FillMeshHole::BridgeEdges(mpTriMesh, edgeVertex1, edgeVertex2, &vertexScaleFields, &outputScaleFields);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }

            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格搭桥失败", "温馨提示", MB_OK);
            }

            UpdateTriMeshVertexColors(mpTriMesh, oldTriMeshVertexSize, outputScaleFields);
            ResetSelection();
            FindHole(false);
            mpTriMesh->UpdateNormal();
            mUpdateMeshRendering = true;
            mUpdateHoleRendering = true;
            mUpdateBridgeRendering = true;
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
            mpUI->ResetFillHole();
            mRightMouseType = MOVE;
        }
    }

    void MeshShopApp::BridgeEdges(bool isSubThread)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        mRightMouseType = SELECT_BRIDGE;
        ResetBridgeTags();
        if (mBridgeEdgeVertices.empty())
        {
            MessageBox(NULL, "进入搭桥模式: 请使用鼠标右键依次框选需要进行搭桥的两对边", "温馨提示", MB_OK);
        }
        else
        {
            MessageBox(NULL, "搭桥模式: 请使用鼠标右键再选择一条边进行搭桥", "温馨提示", MB_OK);
        }
    }

    void MeshShopApp::SelectByRectangle()
    {
        mRightMouseType = SELECT_ADD;
        ResetBridgeTags();
        mBridgeEdgeVertices.clear();
        UpdateBridgeRendering();
    }

    void MeshShopApp::EraseByRectangle()
    {
        mRightMouseType = SELECT_DELETE;
        ResetBridgeTags();
        mBridgeEdgeVertices.clear();
        UpdateBridgeRendering();
    }
 
    void MeshShopApp::DeleteSelections()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        std::vector<GPP::Int> deleteIndex;
        GPP::Int vertexCount = mpTriMesh->GetVertexCount();
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            if (mVertexSelectFlag.at(vid))
            {
                deleteIndex.push_back(vid);
            }
        }
        GPP::ErrorCode res = GPP::DeleteTriMeshVertices(mpTriMesh, deleteIndex);
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "删点失败", "温馨提示", MB_OK);
            return;
        }
        if (GPP::ConsolidateMesh::_IsTriMeshManifold(mpTriMesh) == false)
        {
            if (MessageBox(NULL, "警告：删除面片后的网格有非流形结构，是否需要修复？", "温馨提示", MB_OKCANCEL) == IDOK)
            {
                GPP::ErrorCode res = GPP::ConsolidateMesh::MakeTriMeshManifold(mpTriMesh);
                if (res == GPP_API_IS_NOT_AVAILABLE)
                {
                    MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                    MagicCore::ToolKit::Get()->SetAppRunning(false);
                }
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "拓扑修复失败", "温馨提示", MB_OK);
                    return;
                }
            }
        }
        ResetSelection();
        mpTriMesh->UpdateNormal();
        mUpdateMeshRendering = true;
        mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
        mpUI->ResetFillHole();
        FindHole(false);
    }

    void MeshShopApp::IgnoreBack(bool ignore)
    {
        mIgnoreBack = ignore;
    }

    void MeshShopApp::MoveModel(void)
    {
        mRightMouseType = MOVE;
    }

    int MeshShopApp::GetMeshVertexCount()
    {
        if (mpTriMesh != NULL)
        {
            return mpTriMesh->GetVertexCount();
        }
        else
        {
            return 0;
        }
    }

    void MeshShopApp::UpdateMeshRendering()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        GPP::Vector3 selectColor(1, 0, 0);
        MagicCore::RenderSystem::Get()->RenderMesh("Mesh_MeshShop", "CookTorrance", mpTriMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER,
            &mVertexSelectFlag, &selectColor);
    }

    void MeshShopApp::UpdateHoleRendering()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        // reset the render object
        MagicCore::RenderSystem::Get()->RenderPolyline("MeshShop_Holes", "SimpleLine", GPP::Vector3(1.0,0.0,0.0), std::vector<GPP::Vector3>(), true);
        // append each polyline to the render object
        for (GPP::Int vLoop = 0; vLoop < mShowHoleLoopIds.size(); ++vLoop)
        {
            int vSize = mShowHoleLoopIds.at(vLoop).size();
            std::vector<GPP::Vector3> positions(vSize + 1);
            for (GPP::Int vId = 0; vId < vSize; ++vId)
            {
                positions.at(vId) = mpTriMesh->GetVertexCoord(mShowHoleLoopIds[vLoop][vId]);
            }
            positions.at(vSize) = mpTriMesh->GetVertexCoord(mShowHoleLoopIds[vLoop][0]);
            MagicCore::RenderSystem::Get()->RenderPolyline("MeshShop_Holes", "SimpleLine", GPP::Vector3(1.0,0.0,0.0), positions, false);
        }
        // show seeds
        GPP::Int boundarySeedSize = mBoundarySeedIds.size();
        std::vector<GPP::Vector3> holeSeeds(boundarySeedSize, GPP::Vector3());
        for (GPP::Int vId = 0; vId < boundarySeedSize; ++vId)
        {
            holeSeeds.at(vId) = mpTriMesh->GetVertexCoord(mBoundarySeedIds[vId]);
        }
        MagicCore::RenderSystem::Get()->RenderPointList("MeshShop_HoleSeeds", "SimplePoint", GPP::Vector3(1.0, 0.0, 0.0), holeSeeds);
    }

    void MeshShopApp::UpdateBridgeRendering()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        MagicCore::RenderSystem::Get()->RenderPointList("MeshShop_Bridge", "CookTorrancePointLarge", GPP::Vector3(0.0, 1.0, 1.0), std::vector<GPP::Vector3>());
        std::vector<GPP::Vector3> bridgeVertices;
        if (mBridgeEdgeVertices.size() < 2)
        {
            return;
        }
        for (int vid = 0; vid < mBridgeEdgeVertices.size(); ++vid)
        {
            bridgeVertices.push_back(mpTriMesh->GetVertexCoord(mBridgeEdgeVertices.at(vid)));
        }
        bridgeVertices.push_back(bridgeVertices.front());
        MagicCore::RenderSystem::Get()->RenderPointList("MeshShop_Bridge", "CookTorrancePointLarge", GPP::Vector3(0.0, 1.0, 1.0), bridgeVertices);
    }

    void MeshShopApp::SetToShowHoleLoopVrtIds(const std::vector<std::vector<GPP::Int> >& toShowHoleLoopVrtIds)
    {
        if (mpTriMesh == NULL || toShowHoleLoopVrtIds.empty())
        {
            mShowHoleLoopIds.clear();
            return;
        }

        mShowHoleLoopIds = toShowHoleLoopVrtIds;
    }

    void MeshShopApp::SetBoundarySeedIds(const std::vector<GPP::Int>& holeSeedIds)
    {
        mBoundarySeedIds = holeSeedIds;
    }

    void MeshShopApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }
}
