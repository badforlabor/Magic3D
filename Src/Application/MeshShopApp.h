#pragma once
#include "AppBase.h"

namespace GPP
{
    class TriMesh;
}

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class MeshShopAppUI;
    class MeshShopApp : public AppBase
    {
    public:
        MeshShopApp();
        ~MeshShopApp();

        virtual bool Enter(void);
        virtual bool Update(float timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        bool ImportMesh(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);

    private:
        void InitViewTool(void);
        void UpdateMeshRendering(void);

    private:
        MeshShopAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        MagicCore::ViewTool* mpViewTool;
    };
}
