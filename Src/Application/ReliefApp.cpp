#include "stdafx.h"
#include "ReliefApp.h"
#include "ReliefAppUI.h"
#include "AppManager.h"
#include "MeshShopApp.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/RenderSystem.h"
#include "Gpp.h"

namespace MagicApp
{
    ReliefApp::ReliefApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mpViewTool(NULL),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mHeightField(),
        mResolution(650)
    {
    }

    ReliefApp::~ReliefApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
    }

    bool ReliefApp::Enter()
    {
        InfoLog << "Enter ReliefApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new ReliefAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool ReliefApp::Update(double timeElapsed)
    {
        return true;
    }

    bool ReliefApp::Exit()
    {
        InfoLog << "Exit ReliefApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool ReliefApp::MouseMoved( const OIS::MouseEvent &arg )
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

    bool ReliefApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool ReliefApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {        
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        return  true;
    }

    bool ReliefApp::KeyPressed( const OIS::KeyEvent &arg )
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

    void ReliefApp::WindowFocusChanged(Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
    }

    void ReliefApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
        Ogre::Light* light = sceneManager->createLight("ReliefApp_SimpleLight");
        light->setPosition(-7.5, 7.5, 20);
        light->setDiffuseColour(0.7, 0.7, 0.7);
        light->setSpecularColour(0.1, 0.1, 0.1);
    }

    void ReliefApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("ReliefApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_ReliefApp");
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        } 
    }

    void ReliefApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpViewTool);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        mHeightField.clear();
    }

#if DEBUGDUMPFILE
    void ReliefApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        if (dumpInfo == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpDumpInfo);
        mpDumpInfo = dumpInfo;
        GPP::TriMesh* triMesh = mpDumpInfo->GetTriMesh();
        if (triMesh == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = triMesh;
        mpTriMesh->UpdateNormal();
        InitViewTool();
        UpdateModelRendering();
    }

    void ReliefApp::RunDumpInfo()
    {
        if (mpDumpInfo == NULL)
        {
            return;
        }
        GPP::ErrorCode res = mpDumpInfo->Run();
        if (res != GPP_NO_ERROR)
        {
            MagicCore::ToolKit::Get()->SetAppRunning(false);
            return;
        }
        //Copy result
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = mpDumpInfo->GetTriMesh();
        if (mpTriMesh != NULL)
        {
            mpTriMesh->UnifyCoords(2.0);
            mpTriMesh->UpdateNormal();
        }
        UpdateModelRendering();
        GPPFREEPOINTER(mpDumpInfo);
    }
#endif

    bool ReliefApp::ImportModel()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::TriMesh* triMesh = GPP::Parser::ImportTriMesh(fileName);
            if (triMesh != NULL)
            { 
                triMesh->UnifyCoords(2.5);
                triMesh->UpdateNormal();
                GPPFREEPOINTER(mpTriMesh);
                mpTriMesh = triMesh;
                InfoLog << "Import Mesh,  vertex: " << mpTriMesh->GetVertexCount() << " triangles: " << mpTriMesh->GetTriangleCount() << std::endl;
                InitViewTool();
                UpdateModelRendering();
                mHeightField.clear();
                return true;
            }
            else
            {
                MessageBox(NULL, "网格导入失败", "温馨提示", MB_OK);
            }
        }
        return false;
    }

    void ReliefApp::GenerateRelief(double compressRatio)
    {
        //GPPDebug << "Compress ratio: " << compressRatio << std::endl;
        if (mpTriMesh == NULL && mHeightField.empty())
        {
            MessageBox(NULL, "请先导入网格", "温馨提示", MB_OK);
            return;
        }
        if (mHeightField.empty())
        {
            //Get depth data
            Ogre::TexturePtr depthTex = Ogre::TextureManager::getSingleton().createManual(  
                "DepthTexture",      // name   
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,  
                Ogre::TEX_TYPE_2D,   // type   
                mResolution,  // width   
                mResolution,  // height   
                0,                   // number of mipmaps   
                //Ogre::PF_B8G8R8A8,   // pixel format
                Ogre::PF_FLOAT32_R,
                Ogre::TU_RENDERTARGET
                ); 
            Ogre::RenderTarget* target = depthTex->getBuffer()->getRenderTarget();
            Ogre::Camera* orthCam = MagicCore::RenderSystem::Get()->GetMainCamera();
            orthCam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
            orthCam->setOrthoWindow(3, 3);
            orthCam->setPosition(0, 0, 3);
            orthCam->lookAt(0, 0, 0);
            orthCam->setAspectRatio(1.0);
            orthCam->setNearClipDistance(0.5);
            orthCam->setFarClipDistance(5);
            Ogre::Viewport* viewport = target->addViewport(orthCam);
            viewport->setDimensions(0, 0, 1, 1);
            MagicCore::RenderSystem::Get()->RenderMesh("Mesh_ReliefApp", "Depth", mpTriMesh);
            MagicCore::RenderSystem::Get()->Update();
            Ogre::Image img;
            depthTex->convertToImage(img);
            mHeightField = std::vector<double>(mResolution * mResolution);
            for(int x = 0; x < mResolution; x++)  
            {  
                int baseIndex = x * mResolution;
                for(int y = 0; y < mResolution; y++)  
                {
                    mHeightField.at(baseIndex + y) = (img.getColourAt(x, mResolution - 1 - y, 0))[1];
                }
            }
            Ogre::TextureManager::getSingleton().remove("DepthTexture");
            MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        }
        std::vector<GPP::Real> compressedHeightField = mHeightField;
        GPP::ErrorCode res = GPP::FilterMesh::CompressHeightField(&compressedHeightField, mResolution, mResolution, compressRatio);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "浮雕生成失败", "温馨提示", MB_OK);
            return;
        }
        GPP::TriMesh* reliefMesh = GenerateTriMeshFromHeightField(compressedHeightField, mResolution, mResolution);
        if (reliefMesh != NULL)
        {
            GPPFREEPOINTER(mpTriMesh);
            mpTriMesh = reliefMesh;
            mpTriMesh->UnifyCoords(2.0);
            mpTriMesh->UpdateNormal();
        }
        else
        {
            MessageBox(NULL, "浮雕生成失败", "温馨提示", MB_OK);
        }
        UpdateModelRendering();
    }

    void ReliefApp::EnterMeshTool()
    {
        if (mpTriMesh == NULL)
        {
            AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
            return;
        }
        GPP::TriMesh* copiedTriMesh = GPP::CopyTriMesh(mpTriMesh);
        AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop)
        {
            meshShop->SetMesh(copiedTriMesh, GPP::Vector3(1.0, 1.0, 1.0), 1.0);
        }
        else
        {
            GPPFREEPOINTER(copiedTriMesh);
        }
    }

    void ReliefApp::SetMesh(GPP::TriMesh* triMesh, GPP::Vector3 objCenterCoord, GPP::Real scaleValue)
    {
        if (triMesh == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = triMesh;
        mpTriMesh->UnifyCoords(2.5);
        mpTriMesh->UpdateNormal();
        InitViewTool();
        UpdateModelRendering();
        mHeightField.clear();
    }

    void ReliefApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    void ReliefApp::UpdateModelRendering()
    {
        if (mpTriMesh != NULL)
        {
            MagicCore::RenderSystem::Get()->RenderMesh("Mesh_ReliefApp", "CookTorranceRough", mpTriMesh);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_ReliefApp");
        }
    }

    GPP::TriMesh* ReliefApp::GenerateTriMeshFromHeightField(const std::vector<GPP::Real>& heightField, int resolutionX, int resolutionY)
    {
        GPP::TriMesh* triMesh = new GPP::TriMesh;
        double scaleValue = 1.0;
        double minX = -1.0 * scaleValue;
        double maxX = 1.0 * scaleValue;
        double minY = -1.0 * scaleValue;
        double maxY = 1.0 * scaleValue;
        double deltaX = (maxX - minX) / resolutionX;
        double deltaY = (maxY - minY) / resolutionY;
        for (int xid = 0; xid < resolutionX; xid++)
        {
            for (int yid = 0; yid < resolutionY; yid++)
            {
                int baseIndex = xid * resolutionY;
                triMesh->InsertVertex(GPP::Vector3(minX + deltaX * xid, minY + deltaY * yid, heightField.at(baseIndex + yid)));
            }
        }
        for (int xid = 0; xid < resolutionX - 1; xid++)
        {
            for (int yid = 0; yid < resolutionY - 1; yid++)
            {
                int index = xid * resolutionY + yid;
                int indexRight = index + resolutionY;
                int indexDown = index + 1;
                int indexDiag = indexRight + 1;
                triMesh->InsertTriangle(index, indexRight, indexDiag);
                triMesh->InsertTriangle(index, indexDiag, indexDown);
            }
        }
        return triMesh;
    }
}
