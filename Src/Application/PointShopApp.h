#pragma once
#include "AppBase.h"

namespace GPP
{
    class PointCloud;
    class DumpBase;
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
        void ExportPointCloud(void);
        void SmoothPointCloudNormal(void);
        void RemovePointCloudOutlier(void);
        void SamplePointCloud(int targetPointCount);
        void CalculatePointCloudNormal(void);
        void FlipPointCloudNormal(void);
        void ReconstructMesh(void);

        void SetPointCloud(GPP::PointCloud* pointCloud);
        int GetPointCount(void);
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);

    private:
        void InitViewTool(void);
        void UpdatePointCloudRendering(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        PointShopAppUI* mpUI;
        GPP::PointCloud* mpPointCloud;
        MagicCore::ViewTool* mpViewTool;
        GPP::DumpBase* mpDumpInfo;
    };
}
