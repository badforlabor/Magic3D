#include "stdafx.h"
#include "MeshShopApp.h"
#include "MeshShopAppUI.h"
#include "PointShopApp.h"
#include "AppManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/RenderSystem.h"
#include "GPP.h"
#include "DumpInfo.h"
#include "DumpFillMeshHole.h"

namespace MagicApp
{
    MeshShopApp::MeshShopApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mpViewTool(NULL),
        mpDumpInfo(NULL)
    {
    }

    MeshShopApp::~MeshShopApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpDumpInfo);
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

    bool MeshShopApp::Update(float timeElapsed)
    {
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

    bool MeshShopApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool MeshShopApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        return  true;
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
            RunDumpInfo();
        }
        return true;
    }

    void MeshShopApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("MeshShop_SimpleLight");
        light->setPosition(0, 0, 20);
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
        GPPFREEPOINTER(mpDumpInfo);
        mShowHoleLoopIds.clear();
    }

    bool MeshShopApp::ImportMesh()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::TriMesh* triMesh = GPP::Parser::ImportTriMesh(fileName);
            if (triMesh != NULL)
            { 
                mShowHoleLoopIds.clear();
                triMesh->UnifyCoords(2.0);
                triMesh->UpdateNormal();
                if (mpTriMesh != NULL)
                {
                    delete mpTriMesh;
                }
                mpTriMesh = triMesh;
                InfoLog << "Import Mesh,  vertex: " << mpTriMesh->GetVertexCount() << " triangles: " << triMesh->GetTriangleCount() << std::endl;
                InitViewTool();
                UpdateHoleRendering();
                UpdateMeshRendering();
                return true;
            }
        }
        return false;
    }

    void MeshShopApp::ExportMesh()
    {
        if (mpTriMesh != NULL)
        {
            std::string fileName;
            char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0";
            if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
            {
                GPP::Parser::ExportTriMesh(fileName, mpTriMesh);
            }
        }
    }

    void MeshShopApp::SetMesh(GPP::TriMesh* triMesh)
    {
        if (!mpTriMesh)
        {
            delete mpTriMesh;
        }
        mpTriMesh = triMesh;
        InitViewTool();
        UpdateMeshRendering();
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

    static void UpdateTriMeshVertexColors(GPP::TriMesh *triMesh, const std::vector<GPP::Real>& newVertexColors)
    {
        if (triMesh == NULL || newVertexColors.empty())
        {
            return;
        }
        GPP::Int verticeSize = triMesh->GetVertexCount();
        for (GPP::Int vid = 0; vid < verticeSize; ++vid)
        {
            GPP::Vector3 color(newVertexColors[vid * 3 + 0], newVertexColors[vid * 3 + 1], newVertexColors[vid * 3 + 2]);
            triMesh->SetVertexColor(vid, color);
        }
    }

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
        mpTriMesh->UpdateNormal();
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
#if STOPFAILEDCOMMAND
            MagicCore::ToolKit::Get()->SetAppRunning(false);
#endif
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
        UpdateMeshRendering();
        GPPFREEPOINTER(mpDumpInfo);
        
    }

    void MeshShopApp::ConsolidateTopology()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::ErrorCode res = GPP::ConsolidateMesh::MakeTriMeshManifold(mpTriMesh);
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
#if STOPFAILEDCOMMAND
        bool isManifold = GPP::ConsolidateMesh::IsTriMeshManifold(mpTriMesh);
        if (!isManifold)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
            return;
        }
#endif
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::ReverseDirection()
    {
        if (mpTriMesh == NULL)
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

    void MeshShopApp::ConsolidateGeometry()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::ErrorCode res = GPP::ConsolidateMesh::ConsolidateGeometry(mpTriMesh, GPP::ONE_RADIAN * 5.0, GPP::REAL_TOL, 0.0174532925199433 * 170.0);
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
        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::SmoothMesh()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::ErrorCode res = GPP::FilterMesh::LaplaceSmooth(mpTriMesh);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::EnhanceMeshDetail()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::ErrorCode res = GPP::FilterMesh::EnhanceDetail(mpTriMesh);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::LoopSubdivide()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
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
        GPP::ErrorCode res = GPP::SubdivideMesh::LoopSubdivideMesh(mpTriMesh, &vertexFields, &insertedVertexFields);
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
        GPP::Int insertedVertexCount = mpTriMesh->GetVertexCount() - vertexCount;
        for (GPP::Int vid = 0; vid < insertedVertexCount; vid++)
        {
            GPP::Int baseId = vid * 3;
            mpTriMesh->SetVertexColor(vertexCount + vid, GPP::Vector3(insertedVertexFields.at(baseId), insertedVertexFields.at(baseId + 1), insertedVertexFields.at(baseId + 2)));
        }
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::CCSubdivide()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::ErrorCode res = GPP::SubdivideMesh::CCSubdivideMesh(mpTriMesh);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::RefineMesh(int targetVertexCount)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
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
        GPP::ErrorCode res = GPP::SubdivideMesh::RefineMesh(mpTriMesh, targetVertexCount, &vertexFields, &insertedVertexFields);
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
        GPP::Int insertedVertexCount = mpTriMesh->GetVertexCount() - vertexCount;
        for (GPP::Int vid = 0; vid < insertedVertexCount; vid++)
        {
            GPP::Int baseId = vid * 3;
            mpTriMesh->SetVertexColor(vertexCount + vid, GPP::Vector3(insertedVertexFields.at(baseId), insertedVertexFields.at(baseId + 1), insertedVertexFields.at(baseId + 2)));
        }
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::SimplifyMesh(int targetVertexCount)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        }
        GPP::ErrorCode res = GPP::SimplifyMesh::QuadricSimplify(mpTriMesh, targetVertexCount);
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
        mpTriMesh->UpdateNormal();
        UpdateMeshRendering();
    }

    void MeshShopApp::SampleMesh()
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        GPP::Int vertexCount = mpTriMesh->GetVertexCount();
        if (vertexCount < 3)
        {
            return;
        }
        GPP::PointCloud* pointCloud = new GPP::PointCloud;
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            pointCloud->InsertPoint(mpTriMesh->GetVertexCoord(vid), mpTriMesh->GetVertexNormal(vid));
        }
        pointCloud->SetHasNormal(true);
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop)
        {
            pointShop->SetPointCloud(pointCloud);
        }
        else
        {
            delete pointCloud;
            pointCloud = NULL;
        }
    }

    void MeshShopApp::FindHole(bool isShowHoles)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        } 
        std::vector<std::vector<GPP::Int> > holeIds;
        if (!isShowHoles)
        {
            // clear the holes
            SetToShowHoleLoopVrtIds(holeIds);
            SetBoundarySeedIds(std::vector<GPP::Int>());
            UpdateHoleRendering();
            UpdateMeshRendering();
            return;
        }

        GPP::ErrorCode res = GPP::FillMeshHole::FindHoles(mpTriMesh, &holeIds);
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
        SetToShowHoleLoopVrtIds(holeIds);
        SetBoundarySeedIds(std::vector<GPP::Int>());

        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();
        UpdateHoleRendering();
        UpdateMeshRendering();
    }

    void MeshShopApp::FillHole(bool isFillFlat)
    {
        if (mpTriMesh == NULL)
        {
            return;
        }
        if (mpTriMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
        {
            mpTriMesh->FuseVertex();
        } 

        std::vector<GPP::Int> holeSeeds;
        for (GPP::Int vLoop = 0; vLoop < mShowHoleLoopIds.size(); ++vLoop)
        {
            holeSeeds.insert(holeSeeds.end(), mShowHoleLoopIds[vLoop].begin(), mShowHoleLoopIds[vLoop].end());
        }

        std::vector<GPP::Real> vertexScaleFields, outputScaleFields;
        CollectTriMeshVerticesColorFields(mpTriMesh, &vertexScaleFields);

        GPP::ErrorCode res = GPP::FillMeshHole::FillHoles(mpTriMesh, &holeSeeds, isFillFlat ? GPP::FILL_MESH_HOLE_FLAT : GPP::FILL_MESH_HOLE_SMOOTH,
            &vertexScaleFields, &outputScaleFields);
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

        UpdateTriMeshVertexColors(mpTriMesh, outputScaleFields);
        SetToShowHoleLoopVrtIds(std::vector<std::vector<GPP::Int> >());
        SetBoundarySeedIds(std::vector<GPP::Int>());
        mpTriMesh->UnifyCoords(2.0);
        mpTriMesh->UpdateNormal();
        UpdateHoleRendering();
        UpdateMeshRendering();
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
        MagicCore::RenderSystem::Get()->RenderMesh("Mesh_MeshShop", "CookTorrance", mpTriMesh);
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
