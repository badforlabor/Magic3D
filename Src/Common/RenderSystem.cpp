#include "stdafx.h"
#include "RenderSystem.h"
#include "../Common/LogSystem.h"
#include "MagicListener.h"
#include "PointCloud.h"
#include "TriMesh.h"

namespace MagicCore
{
    RenderSystem* RenderSystem::mpRenderSystem = NULL;

    RenderSystem::RenderSystem(void) : 
        mpRoot(NULL), 
        mpMainCamera(NULL), 
        mpRenderWindow(NULL), 
        mpSceneManager(NULL)
    {
    }

    RenderSystem* RenderSystem::Get()
    {
        if (mpRenderSystem == NULL)
        {
            mpRenderSystem = new RenderSystem;
        }
        return mpRenderSystem;
    }

    void RenderSystem::Init()
    {
        InfoLog << "RenderSystem init...." << std::endl;
        mpRoot = new Ogre::Root();
        bool hasConfig = false;
        if (mpRoot->restoreConfig())
        {
            hasConfig = true;
        }
        else if (mpRoot->showConfigDialog())
        {
            hasConfig = true;
        }

        if (hasConfig)
        {
            // initialise system according to user options.
            mpRenderWindow = mpRoot->initialise(true, "Magic3D");
            // Create the scene manager
            mpSceneManager = mpRoot->createSceneManager(Ogre::ST_GENERIC, "MainSceneManager");
            // Create and initialise the camera
            mpMainCamera = mpSceneManager->createCamera("MainCamera");
            SetupCameraDefaultParameter();
            // Create a viewport covering whole window
            Ogre::Viewport* viewPort = mpRenderWindow->addViewport(mpMainCamera);
            viewPort->setBackgroundColour(Ogre::ColourValue(0.8705882352941176, 0.8705882352941176, 0.8705882352941176));
            // Update the camera aspect ratio to that of the viewport
            mpMainCamera->setAspectRatio(Ogre::Real(viewPort->getActualWidth()) / Ogre::Real(viewPort->getActualHeight()));

            mpRoot->addFrameListener(MagicListener::Get());
            Ogre::WindowEventUtilities::addWindowEventListener(mpRenderWindow, MagicListener::Get());

            //get supported syntax information
            const Ogre::GpuProgramManager::SyntaxCodes &syntaxCodes = Ogre::GpuProgramManager::getSingleton().getSupportedSyntax();
            for (Ogre::GpuProgramManager::SyntaxCodes::const_iterator iter = syntaxCodes.begin();iter != syntaxCodes.end();++iter)
            {
                InfoLog << "supported syntax : " << (*iter) << std::endl;
            }
        }
    }

    void RenderSystem::SetupCameraDefaultParameter()
    {
        if (mpMainCamera != NULL)
        {
            mpMainCamera->setProjectionType(Ogre::PT_PERSPECTIVE);
            mpMainCamera->setPosition(0, 0, 4);
            mpMainCamera->lookAt(0, 0, 0);
            mpMainCamera->setNearClipDistance(0.05);
            mpMainCamera->setFarClipDistance(0);
            mpMainCamera->setAspectRatio((Ogre::Real)mpRenderWindow->getWidth() / (Ogre::Real)mpRenderWindow->getHeight());
        }
    }

    void RenderSystem::Update()
    {
        mpRoot->renderOneFrame();
    }

    Ogre::RenderWindow* RenderSystem::GetRenderWindow()
    {
        return mpRenderWindow;
    }

    Ogre::SceneManager* RenderSystem::GetSceneManager()
    {
        return mpSceneManager;
    }

    int RenderSystem::GetRenderWindowWidth()
    {
        if (mpRenderWindow != NULL)
        {
            return mpRenderWindow->getWidth();
        }
        else
        {
            return 0;
        }
    }
    
    int RenderSystem::GetRenderWindowHeight()
    {
        if (mpRenderWindow != NULL)
        {
            return mpRenderWindow->getHeight();
        }
        else
        {
            return 0;
        }
    }

    Ogre::Camera* RenderSystem::GetMainCamera()
    {
        return mpMainCamera;
    }

    void RenderSystem::RenderPointCloud(std::string pointCloudName, std::string materialName, const GPP::PointCloud* pointCloud, ModelNodeType nodeType)
    {
        if (mpSceneManager == NULL)
        {
            InfoLog << "Error: RenderSystem::mpSceneMagager is NULL when RenderPoingCloud" << std::endl;
            return;
        }
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(pointCloudName))
        {
            manualObj = mpSceneManager->getManualObject(pointCloudName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(pointCloudName);
            AttachManualObjectToSceneNode(nodeType, manualObj);
        }
        if (pointCloud == NULL)
        {
            return;
        }
        if (pointCloud->HasNormal())
        {
            int pointCount = pointCloud->GetPointCount();
            manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
            for (int pid = 0; pid < pointCount; pid++)
            {
                GPP::Vector3 coord = pointCloud->GetPointCoord(pid);
                GPP::Vector3 normal = pointCloud->GetPointNormal(pid);
                GPP::Vector3 color = pointCloud->GetPointColor(pid);
                manualObj->position(coord[0], coord[1], coord[2]);
                manualObj->normal(normal[0], normal[1], normal[2]);
                manualObj->colour(color[0], color[1], color[2]);
            }
            manualObj->end();
        }
        else
        {
            int pointCount = pointCloud->GetPointCount();
            manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
            for (int pid = 0; pid < pointCount; pid++)
            {
                GPP::Vector3 coord = pointCloud->GetPointCoord(pid);
                GPP::Vector3 color = pointCloud->GetPointColor(pid);
                manualObj->position(coord[0], coord[1], coord[2]);
                manualObj->colour(color[0], color[1], color[2]);
            }
            manualObj->end();
        }
    }

    void RenderSystem::RenderPointCloudList(std::string pointCloudListName, std::string materialName, 
        const std::vector<GPP::PointCloud*>& pointCloudList, bool hasNormal, ModelNodeType nodeType)
    {
        if (mpSceneManager == NULL)
        {
            InfoLog << "Error: RenderSystem::mpSceneMagager is NULL when RenderPointCloudList" << std::endl;
            return;
        }
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(pointCloudListName))
        {
            manualObj = mpSceneManager->getManualObject(pointCloudListName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(pointCloudListName);
            AttachManualObjectToSceneNode(nodeType, manualObj);
        }
        if (pointCloudList.empty())
        {
            return;
        }
        for (std::vector<GPP::PointCloud*>::const_iterator pItr = pointCloudList.begin(); pItr != pointCloudList.end(); ++pItr)
        {
            if ((*pItr) == NULL)
            {
                continue;
            }
            int pointCount = (*pItr)->GetPointCount();
            if (hasNormal)
            {
                manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
                for (int pid = 0; pid < pointCount; pid++)
                {
                    GPP::Vector3 coord = (*pItr)->GetPointCoord(pid);
                    GPP::Vector3 normal = (*pItr)->GetPointNormal(pid);
                    GPP::Vector3 color = (*pItr)->GetPointColor(pid);
                    manualObj->position(coord[0], coord[1], coord[2]);
                    manualObj->normal(normal[0], normal[1], normal[2]);
                    manualObj->colour(color[0], color[1], color[2]);
                }
                manualObj->end();
            }
            else
            {
                manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
                for (int pid = 0; pid < pointCount; pid++)
                {
                    GPP::Vector3 coord = (*pItr)->GetPointCoord(pid);
                    GPP::Vector3 color = (*pItr)->GetPointColor(pid);
                    manualObj->position(coord[0], coord[1], coord[2]);
                    manualObj->colour(color[0], color[1], color[2]);
                }
                manualObj->end();
            }
        }
    }

    void RenderSystem::RenderPointList(std::string pointListName, std::string materialName, const GPP::Vector3& color, 
        const std::vector<GPP::Vector3>& pointCoords, ModelNodeType nodeType)
    {
        if (mpSceneManager == NULL)
        {
            InfoLog << "Error: RenderSystem::mpSceneMagager is NULL when RenderPointList" << std::endl;
            return;
        }
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(pointListName))
        {
            manualObj = mpSceneManager->getManualObject(pointListName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(pointListName);
            AttachManualObjectToSceneNode(nodeType, manualObj);
        }
        manualObj->begin(materialName, Ogre::RenderOperation::OT_POINT_LIST);
        for (std::vector<GPP::Vector3>::const_iterator itr = pointCoords.begin(); itr != pointCoords.end(); ++itr)
        {
            manualObj->position((*itr)[0], (*itr)[1], (*itr)[2]);
            manualObj->colour(color[0], color[1], color[2]);
        }
        manualObj->end();
    }

    void RenderSystem::RenderMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh, ModelNodeType nodeType)
    {
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(meshName))
        {
            manualObj = mpSceneManager->getManualObject(meshName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(meshName);
            AttachManualObjectToSceneNode(nodeType, manualObj);
        }
        if (mesh == NULL)
        {
            return;
        }
        manualObj->begin(materialName, Ogre::RenderOperation::OT_TRIANGLE_LIST);
        int vertexCount = mesh->GetVertexCount();
        for (int vid = 0; vid < vertexCount; vid++)
        {
            GPP::Vector3 coord = mesh->GetVertexCoord(vid);
            GPP::Vector3 normal = mesh->GetVertexNormal(vid);
            GPP::Vector3 color = mesh->GetVertexColor(vid);
            manualObj->position(coord[0], coord[1], coord[2]);
            manualObj->normal(normal[0], normal[1], normal[2]);
            manualObj->colour(color[0], color[1], color[2]);
        }
        int triangleCount = mesh->GetTriangleCount();
        for (int fid = 0; fid < triangleCount; fid++)
        {
            int vertexIds[3];
            mesh->GetTriangleVertexIds(fid, vertexIds);
            manualObj->triangle(vertexIds[0], vertexIds[1], vertexIds[2]);
        }
        manualObj->end();
    }

    /*void RenderSystem::RenderLineSegments(std::string lineName, std::string materialName, const std::vector<GPP::Vector3>& startCoords, const std::vector<GPP::Vector3>& endCoords)
    {
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(lineName))
        {
            manualObj = mpSceneManager->getManualObject(lineName);
            manualObj->clear();
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(lineName);
            if (mpSceneManager->hasSceneNode("ModelNode"))
            {
                mpSceneManager->getSceneNode("ModelNode")->attachObject(manualObj);
            }
            else
            {
                mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNode")->attachObject(manualObj);
            }
        }
        manualObj->begin(materialName, Ogre::RenderOperation::OT_LINE_LIST);
        int lineCount = startCoords.size();
        if (lineCount > endCoords.size())
        {
            lineCount = endCoords.size();
        }
        for (int lineId = 0; lineId < lineCount; lineId++)
        {
            GPP::Vector3 start = startCoords.at(lineId);
            GPP::Vector3 end = endCoords.at(lineId);
            manualObj->position(start[0], start[1], start[2]);
            manualObj->position(end[0], end[1], end[2]);
        }
        manualObj->end();
    }*/

    void RenderSystem::RenderPolyline(std::string lineName, std::string materialName, const GPP::Vector3& color, 
        const std::vector<GPP::Vector3>& polylineCoords, bool appendNewPolyline, ModelNodeType nodeType)
    {
        Ogre::ManualObject* manualObj = NULL;
        if (mpSceneManager->hasManualObject(lineName))
        {
            manualObj = mpSceneManager->getManualObject(lineName);
            if (appendNewPolyline)
            {
                manualObj->clear();
            }
        }
        else
        {
            manualObj = mpSceneManager->createManualObject(lineName);
            AttachManualObjectToSceneNode(nodeType, manualObj);
        }
        manualObj->begin(materialName, Ogre::RenderOperation::OT_LINE_STRIP);
        int pointSize = polylineCoords.size();
        for (int pointId = 0; pointId < pointSize; ++pointId)
        {
            GPP::Vector3 pt = polylineCoords.at(pointId);
            manualObj->position(pt[0], pt[1], pt[2]);
            manualObj->colour(color[0], color[1], color[2]);
        }
        manualObj->end();
    }

    void RenderSystem::HideRenderingObject(std::string objName)
    {
        if (mpSceneManager != NULL)
        {
            if (mpSceneManager->hasManualObject(objName))
            {
                mpSceneManager->destroyManualObject(objName);
            }
        }
    }
    
    void RenderSystem::ResertAllSceneNode()
    {
        if (mpSceneManager->hasSceneNode("ModelNode") == false)
        {
            mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNode");
        }
        else
        {
            mpSceneManager->getSceneNode("ModelNode")->resetToInitialState();
        }
        if (mpSceneManager->hasSceneNode("ModelNodeLeft") == false)
        {
            mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNodeLeft", Ogre::Vector3(-1, 0, 0));
        }
        else
        {
            mpSceneManager->getSceneNode("ModelNodeLeft")->resetToInitialState();
            mpSceneManager->getSceneNode("ModelNodeLeft")->translate(Ogre::Vector3(-1, 0, 0));
        }
        if (mpSceneManager->hasSceneNode("ModelNodeRight") == false)
        {
            mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNodeRight", Ogre::Vector3(1, 0, 0));
        }
        else
        {
            mpSceneManager->getSceneNode("ModelNodeRight")->resetToInitialState();
            mpSceneManager->getSceneNode("ModelNodeRight")->translate(Ogre::Vector3(1, 0, 0));
        }
    }

    RenderSystem::~RenderSystem(void)
    {
    }

    void RenderSystem::AttachManualObjectToSceneNode(ModelNodeType nodeType, Ogre::ManualObject* manualObj)
    {
        switch (nodeType)
        {
        case MagicCore::RenderSystem::MODEL_NODE_CENTER:
            if (mpSceneManager->hasSceneNode("ModelNode"))
            {
                mpSceneManager->getSceneNode("ModelNode")->attachObject(manualObj);
            }
            else
            {
                mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNode")->attachObject(manualObj);
            }
            break;
        case MagicCore::RenderSystem::MODEL_NODE_LEFT:
            if (mpSceneManager->hasSceneNode("ModelNodeLeft"))
            {
                mpSceneManager->getSceneNode("ModelNodeLeft")->attachObject(manualObj);
            }
            else
            {
                mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNodeLeft", Ogre::Vector3(-1, 0, 0))->attachObject(manualObj);
            }
            break;
        case MagicCore::RenderSystem::MODEL_NODE_RIGHT:
            if (mpSceneManager->hasSceneNode("ModelNodeRight"))
            {
                mpSceneManager->getSceneNode("ModelNodeRight")->attachObject(manualObj);
            }
            else
            {
                mpSceneManager->getRootSceneNode()->createChildSceneNode("ModelNodeRight", Ogre::Vector3(1, 0, 0))->attachObject(manualObj);
            }
            break;
        default:
            break;
        }
    }
}
