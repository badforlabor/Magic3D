#pragma once
#include "AppBase.h"
#include "GPP.h"
#include <vector>

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
            ALIGN_MARK,
            ALIGN_FREE,
            ALIGN_ICP,
            NORMAL_REF,
            NORMAL_FROM,
            OUTLIER_REF,
            OUTLIER_FROM,
            GLOBAL_REGISTRATE
        };

    public:

        RegistrationApp();
        ~RegistrationApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        void DoCommand(bool isSubThread);

        bool ImportPointCloudRef(void);
        
        void CalculateRefNormal(bool isDepthImage, bool isSubThread = true);
        void FlipRefNormal(void);
        void RemoveOutlierRef(bool isSubThread = true);
        
        void DeleteRefMark(void);
        void ImportRefMark(void);

        bool ImportPointCloudFrom(void);
        void TransformPointCloudFrom(void);
        
        void CalculateFromNormal(bool isDepthImage, bool isSubThread = true);
        void FlipFromNormal(void);
        void RemoveOutlierFrom(bool isSubThread = true);
        
        void DeleteFromMark(void);
        void ImportFromMark(void);

        void AlignMark(bool isSubThread = true);
        void AlignFree(bool isSubThread = true);
        void AlignICP(bool isSubThread = true);
        
        void FuseRef(void);

        void GlobalRegistrate(bool isInSequence, bool isSubThread = true);

        void EnterPointShop(void);

        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
        bool IsCommandInProgress(void);

        void SwitchSeparateDisplay(void);
        void SetSeparateDisplay(bool isSeparate);
        void UpdatePickToolNodeName(void);

    private:
        void InitViewTool(void);
        void UpdatePointCloudFromRendering(void);
        void UpdatePointCloudRefRendering(void);
        void UpdateMarkRefRendering(void);
        void UpdateMarkFromRendering(void);
        void UpdatePointCloudListRendering(void);
        void SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color);
        bool IsCommandAvaliable(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        void ResetGlobalRegistrationData(void);

    private:
        RegistrationAppUI* mpUI;
        MagicCore::ViewTool* mpViewTool;
        bool mIsSeparateDisplay;
        MagicCore::PickTool* mpPickToolRef;
        MagicCore::PickTool* mpPickToolFrom;
        GPP::Matrix4x4 mTransformFrom;
        GPP::Matrix4x4 mTransformFromAccumulate;
        GPP::DumpBase* mpDumpInfo;
        GPP::PointCloud* mpPointCloudRef;
        GPP::PointCloud* mpPointCloudFrom;
        GPP::FusePointCloud* mpFusePointCloud;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
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
        std::vector<GPP::PointCloud*> mPointCloudList;
        std::vector<std::vector<GPP::Vector3> > mMarkList;
        double mGlobalRegistrateProgress;
        bool mEnterPointShop;
        bool mUpdateUIInfo;
        bool mPointCloudInSequence;
    };
}
