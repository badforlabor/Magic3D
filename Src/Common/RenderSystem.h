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
}

namespace GPP
{
    class PointCloud;
    class TriMesh;
}

namespace MagicCore
{
    class RenderSystem
    {
    private:
        static RenderSystem* mpRenderSystem;
        RenderSystem(void);
    public:
        static RenderSystem* Get(void);
        void Init(void);
        void Update(void);

        Ogre::RenderWindow* GetRenderWindow(void);
        Ogre::SceneManager* GetSceneManager(void);
        Ogre::Camera*       GetMainCamera(void);
        void SetupCameraDefaultParameter();

        int GetRenderWindowWidth(void);
        int GetRenderWindowHeight(void);

        //Rendering tools
        void RenderPointCloud(std::string pointCloudName, std::string materialName, const GPP::PointCloud* pointCloud);
        void RenderPointList(std::string pointListName, std::string materialName, const GPP::Vector3& color, const std::vector<GPP::Vector3>& pointCoords);
        void RenderMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh);
        void RenderLineSegments(std::string lineName, std::string materialName, const std::vector<GPP::Vector3>& startCoords, const std::vector<GPP::Vector3>& endCoords);
        void HideRenderingObject(std::string objName);

        virtual ~RenderSystem(void);

    private:
        Ogre::Root*    mpRoot;
        Ogre::Camera*  mpMainCamera;
        Ogre::RenderWindow* mpRenderWindow;
        Ogre::SceneManager* mpSceneManager;
    };
}

