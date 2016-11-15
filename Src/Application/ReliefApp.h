#pragma once
#include "AppBase.h"
#include <vector>
#include "opencv2/opencv.hpp"
#include "GPP.h"

namespace GPP
{
    class TriMesh;
    class DumpBase;
    class PointCloud;
}

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class ReliefAppUI;
    class ReliefApp : public AppBase
    {
        enum DisplayMode
        {
            TRIMESH = 0,
            RELIEF,
            POINTCLOUD
        };

    public:
        ReliefApp();
        ~ReliefApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        void SwitchDisplayMode(void);
        bool ImportModel(void);
        void GenerateRelief(double compressRatio, int resolution);
        void ExportReliefMesh(void);
        void CaptureDepthPointCloud(int scanResolution, int imageResolution, const char* shadeName);
        void SavePointCloud(void);
        void SaveDepthPointCloud(const char* pointCloudName);
        void RotateView(double axisX, double axisY, double axisZ, double angle);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

        void InitViewTool(void);
        void UpdateModelRendering(void);
        GPP::TriMesh* GenerateTriMeshFromHeightField(const std::vector<GPP::Real>& heightField, int resolutionX, int resolutionY);
        void RunScript();

    private:
        ReliefAppUI* mpUI;
        MagicCore::ViewTool* mpViewTool;
#if DEBUGDUMPFILE
        GPP::DumpBase* mpDumpInfo;
#endif
        GPP::TriMesh* mpReliefMesh;
        DisplayMode mDisplayMode;
        GPP::PointCloud* mpDepthPointCloud;
        cv::Mat mDepthImage;
        int mScanResolution;
        int mImageResolution;
        int mImageStartId;
        int mImageSegCount;
    };
}
