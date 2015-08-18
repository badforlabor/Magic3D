#pragma once
#include "AppBase.h"

namespace GPP
{
    class PointCloud;
}

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class PointShopAppUI;
    class PointShopApp : public AppBase
    {
    public:

        PointShopApp();
        ~PointShopApp();

        virtual bool Enter(void);
        virtual bool Update(float timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        bool ImportPointCloud(void);
        void SmoothPointCloud(void);

    private:
        void InitViewTool(void);
        void UpdatePointCloudRendering(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);

    private:
        PointShopAppUI* mpUI;
        GPP::PointCloud* mpPointCloud;
        MagicCore::ViewTool* mpViewTool;
    };
}
