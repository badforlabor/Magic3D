#pragma once
#include <string>
#include <vector>
#include "Vector3.h"

namespace Ogre
{
    class RenderWindow;
    class SceneManager;
    class Camera;
    class Root;
    class ManualObject;
    class Viewport;
}

namespace GPP
{
    class PointCloud;
    class TriMesh;
    struct Obb;
}

namespace MagicCore
{
    class RenderSystem
    {
    private:
        static RenderSystem* mpRenderSystem;
        RenderSystem(void);
    public:
        enum ModelNodeType
        {
            MODEL_NODE_CENTER = 0,
            MODEL_NODE_LEFT,
            MODEL_NODE_RIGHT
        };

        static RenderSystem* Get(void);
        void Init(void);
        void Update(void);

        Ogre::RenderWindow* GetRenderWindow(void);
        Ogre::SceneManager* GetSceneManager(void);
        Ogre::Camera*       GetMainCamera(void);
        void SetupCameraDefaultParameter();
        void SetBackgroundColor(double r, double g, double b);

        int GetRenderWindowWidth(void);
        int GetRenderWindowHeight(void);

        //Rendering tools
        void RenderPointCloud(std::string pointCloudName, std::string materialName, const GPP::PointCloud* pointCloud, 
            ModelNodeType nodeType = MODEL_NODE_CENTER, std::vector<bool>* selectFlags = NULL, GPP::Vector3* selectColor = NULL);
        void RenderPointCloudList(std::string pointCloudListName, std::string materialName, const std::vector<GPP::PointCloud*>& pointCloudList, bool hasNormal, ModelNodeType nodeType = MODEL_NODE_CENTER);
        void RenderPointList(std::string pointListName, std::string materialName, const GPP::Vector3& color, const std::vector<GPP::Vector3>& pointCoords, ModelNodeType nodeType = MODEL_NODE_CENTER);
        void RenderMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh, 
            ModelNodeType nodeType = MODEL_NODE_CENTER, std::vector<bool>* selectFlags = NULL, GPP::Vector3* selectColor = NULL, bool isFlat = false);
        void RenderTextureMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh, ModelNodeType nodeType = MODEL_NODE_CENTER);
        void RenderUVMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh, ModelNodeType nodeType = MODEL_NODE_CENTER);
        void RenderLineSegments(std::string lineName, std::string materialName, const std::vector<GPP::Vector3>& startCoords, const std::vector<GPP::Vector3>& endCoords);
        void RenderPolyline(std::string polylineName, std::string materialName, const GPP::Vector3& color, const std::vector<GPP::Vector3>& polylineCoords, bool appendNewPolyline = false, ModelNodeType nodeType = MODEL_NODE_CENTER);
        void RenderOBB(std::string obbName, std::string materialName, const GPP::Vector3& color, const GPP::Obb& obb, bool appendNewObb = false, ModelNodeType nodeType = MODEL_NODE_CENTER);
        void HideRenderingObject(std::string objName);

        void ResertAllSceneNode(void);

        virtual ~RenderSystem(void);

    private:
        void AttachManualObjectToSceneNode(ModelNodeType nodeType, Ogre::ManualObject* manualObj);

    private:
        Ogre::Root*    mpRoot;
        Ogre::Camera*  mpMainCamera;
        Ogre::RenderWindow* mpRenderWindow;
        Ogre::SceneManager* mpSceneManager;
        Ogre::Viewport* mpViewport;
    };
}

