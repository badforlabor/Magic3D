#pragma once
#include "AppBase.h"
#include "Gpp.h"
#if DEBUGDUMPFILE
#include "DumpBase.h"
#endif

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class PointShopAppUI;
    class PointShopApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0,
            EXPORT,
            NORMALCALCULATION,
            NORMALSMOOTH,
            OUTLIER,
            ISOLATE,
            COORDSMOOTH,
            RECONSTRUCTION
        };

        enum RightMouseType
        {
            MOVE = 0,
            SELECT_ADD,
            SELECT_DELETE,
            REVERSE_NORMAL,
        };

    public:
        PointShopApp();
        ~PointShopApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        void DoCommand(bool isSubThread);

        bool ImportPointCloud(void);
        void ExportPointCloud(bool isSubThread = true);
        
        void SmoothPointCloudNormal(bool isSubThread = true);
        
        void SamplePointCloud(int targetPointCount);

        void SimplifyPointCloud(int resolution);

        void CalculatePointCloudNormal(bool isDepthImage, bool isSubThread = true);
        void FlipPointCloudNormal(void);
        void ReversePatchNormal(void);
        void ReconstructMesh(int quality, bool isSubThread = true);

        void RemovePointCloudOutlier(bool isSubThread = true);
        void RemoveIsolatePart(bool isSubThread = true);
        void SmoothPointCloudByNormal(bool isSubThread = true);

        void SelectByRectangle(void);
        void EraseByRectangle(void);
        void DeleteSelections(void);
        void IgnoreBack(bool ignore);
        void MoveModel(void);

        void SetPointCloud(GPP::PointCloud* pointCloud, GPP::Vector3 objCenterCoord, GPP::Real scaleValue);
        int GetPointCount(void);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif
        bool IsCommandInProgress(void);

    private:
        void InitViewTool(void);
        void UpdatePickTool(void);
        void UpdatePointCloudRendering(void);
        bool IsCommandAvaliable(void);
        void SelectControlPointByRectangle(int startCoordX, int startCoordY, int endCoordX, int endCoordY);
        void UpdateRectangleRendering(int startCoordX, int startCoordY, int endCoordX, int endCoordY);
        void ClearRectangleRendering(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        void ResetSelection(void);

    private:
        PointShopAppUI* mpUI;
        GPP::PointCloud* mpPointCloud;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        MagicCore::ViewTool* mpViewTool;
        MagicCore::PickTool* mpPickTool;
#if DEBUGDUMPFILE
        GPP::DumpBase* mpDumpInfo;
#endif
        CommandType mCommandType;
        bool mUpdatePointCloudRendering;
        bool mIsCommandInProgress;
        GPP::TriMesh* mpTriMesh;
        bool mIsDepthImage;
        int mReconstructionQuality;
        std::vector<bool> mPointSelectFlag;
        RightMouseType mRightMouseType;
        GPP::Vector2 mMousePressdCoord;
        bool mIgnoreBack;
    };
}
