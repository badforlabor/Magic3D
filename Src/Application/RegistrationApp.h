#pragma once
#include "AppBase.h"
#include "Vector3.h"
#include <vector>

namespace GPP
{
    class PointCloud;
    class DumpBase;
    class FusePointCloud;
}

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class RegistrationAppUI;
    class RegistrationApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0,
            ALIGN_FAST,
            ALIGN_PRECISE,
            ALIGN_ICP,
            NORMAL_REF,
            NORMAL_FROM,
            NORMAL_SMOOTH_REF,
            NORMAL_SMOOTH_FROM
        };

    public:
        enum MouseMode
        {
            MM_VIEW = 0,
            MM_PICK_REF,
            MM_PICK_FROM
        };

        RegistrationApp();
        ~RegistrationApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        void DoCommand(bool isSubThread);

        bool ImportPointCloudRef(void);
        
        void CalculateRefNormal(bool isDepthImage, bool isSubThread = true);
        void FlipRefNormal(void);
        void SmoothRefNormal(bool isSubThread = true);
        
        void SwitchRefControlState(void);
        void DeleteRefMark(void);
        void ImportRefMark(void);

        bool ImportPointCloudFrom(void);
        
        void CalculateFromNormal(bool isDepthImage, bool isSubThread = true);
        void FlipFromNormal(void);
        void SmoothFromNormal(bool isSubThread = true);
        
        void SwitchFromControlState(void);
        void DeleteFromMark(void);
        void ImportFromMark(void);

        void AlignFast(bool isSubThread = true);
        void AlignPrecise(bool isSubThread = true);
        void AlignICP(bool isSubThread = true);
        
        void FuseRef(void);

        void EnterPointShop(void);

        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
        void SwitchToViewMode(void);
        bool IsCommandInProgress(void);

    private:
        void InitViewTool(void);
        void InitPickTool(void);
        void UpdatePointCloudFromRendering(void);
        void UpdatePointCloudRefRendering(void);
        void UpdateMarkRefRendering(void);
        void UpdateMarkFromRendering(void);
        void SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color);
        bool IsCommandAvaliable(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        RegistrationAppUI* mpUI;
        MagicCore::ViewTool* mpViewTool;
        MagicCore::PickTool* mpPickTool;
        GPP::DumpBase* mpDumpInfo;
        GPP::PointCloud* mpPointCloudRef;
        GPP::PointCloud* mpPointCloudFrom;
        GPP::FusePointCloud* mpFusePointCloud;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        MouseMode mMouseMode;
        std::vector<GPP::Vector3> mRefMarks;
        std::vector<GPP::Vector3> mFromMarks;
        CommandType mCommandType;
        bool mIsCommandInProgress;
        bool mUpdatePointRefRendering;
        bool mUpdatePointFromRendering;
        bool mUpdateMarkRefRendering;
        bool mUpdateMarkFromRendering;
        bool mIsDepthImageRef;
        bool mIsDepthImageFrom;
    };
}
