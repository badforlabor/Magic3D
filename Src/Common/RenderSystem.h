#pragma once
#include <string>

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
        void RenderMesh(std::string meshName, std::string materialName, const GPP::TriMesh* mesh);
        void HideRenderingObject(std::string objName);

        virtual ~RenderSystem(void);

    private:
        Ogre::Root*    mpRoot;
        Ogre::Camera*  mpMainCamera;
        Ogre::RenderWindow* mpRenderWindow;
        Ogre::SceneManager* mpSceneManager;
    };
}

