#include "stdafx.h"
#include "MeshShopApp.h"
#include "MeshShopAppUI.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/RenderSystem.h"
#include "Mesh.h"
#include "Parser.h"
#include "ErrorCodes.h"
#include "ConsolidateMesh.h"
#include "SubdivideMesh.h"

namespace MagicApp
{
    MeshShopApp::MeshShopApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mpViewTool(NULL)
    {
    }

    MeshShopApp::~MeshShopApp()
    {
        if (mpUI != NULL)
        {
            delete mpUI;
            mpUI = NULL;
        }
        if (mpTriMesh != NULL)
        {
            delete mpTriMesh;
            mpTriMesh = NULL;
        }
        if (mpViewTool != NULL)
        {
            delete mpViewTool;
            mpViewTool = NULL;
        }
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
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        } 
    }

    void MeshShopApp::ClearData()
    {
        if (mpUI != NULL)
        {
            delete mpUI;
            mpUI = NULL;
        }
        if (mpTriMesh != NULL)
        {
            delete mpTriMesh;
            mpTriMesh = NULL;
        }
        if (mpViewTool != NULL)
        {
            delete mpViewTool;
            mpViewTool = NULL;
        }
    }

    bool MeshShopApp::ImportMesh()
    {
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0Color Files(*.cps)\0*.cps\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::TriMesh* triMesh = GPP::Parser::ImportTriMesh(fileName);
            if (triMesh != NULL)
            { 
                triMesh->UnifyCoords(2.0);
                triMesh->UpdateNormal();
                if (mpTriMesh != NULL)
                {
                    delete mpTriMesh;
                }
                mpTriMesh = triMesh;
                InfoLog << "Import Mesh,  vertex: " << mpTriMesh->GetVertexCount() << " triangles: " << triMesh->GetTriangleCount() << std::endl;
                InitViewTool();
                UpdateMeshRendering();
                return true;
            }
        }
        return false;
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

    void MeshShopApp::SmoothMesh()
    {
        if (mpTriMesh)
        {
            GPP::Int res = GPP::ConsolidateMesh::LaplaceSmooth(mpTriMesh, 0.25, 5, true);
            if (res == GPP_NO_ERROR)
            {
                UpdateMeshRendering();
            }
        }
    }

    void MeshShopApp::SubdivideMesh()
    {
        if (mpTriMesh)
        {
            GPP::Int res = GPP::SubdivideMesh::CCSubdivideMesh(mpTriMesh);
            if (res == GPP_NO_ERROR)
            {
                UpdateMeshRendering();
            }
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

    void MeshShopApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }
}
