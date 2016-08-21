#pragma once
#include "AppBase.h"
#include "GPP.h"
#include <vector>
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
            GLOBAL_REGISTRATE,
            GLOBAL_FUSE,
            FUSE_COLOR
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
        void ReversePatchNormalRef(void);
        void RemoveOutlierRef(bool isSubThread = true);
        
        void DeleteRefMark(void);
        void ImportRefMark(void);

        bool ImportPointCloudFrom(void);
        
        void CalculateFromNormal(bool isDepthImage, bool isSubThread = true);
        void FlipFromNormal(void);
        void ReversePatchNormalFrom(void);
        void RemoveOutlierFrom(bool isSubThread = true);
        
        void DeleteFromMark(void);
        void ImportFromMark(void);

        void AlignMark(bool isSubThread = true);
        void AlignFree(int maxSampleTripleCount, bool isSubThread = true);
        void AlignICP(bool isSubThread = true);
        
        void FuseRef(void);

        void GlobalRegistrate(int maxIterationCount, bool isSubThread = true);
        void GlobalFuse(bool isSubThread = true);

        void EnterPointShop(void);

        void ImportPointCloudList(void);
        void ImportMarkList(void);
        void ImportPointCloudColor(GPP::PointCloud* pointCloud, std::string fileName);

        void FusePointCloudColor(bool needBlend, bool isSubThread = true);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif
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
        void UpdateMarkListRendering(void);
        void UpdatePointCloudListRendering(void);
        void SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color);
        bool IsCommandAvaliable(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        void ResetGlobalRegistrationData(void);
        void ClearPairwiseRegistrationData(void);
        void ClearAuxiliaryData(void);

    private:
        RegistrationAppUI* mpUI;
        MagicCore::ViewTool* mpViewTool;
        bool mIsSeparateDisplay;
        MagicCore::PickTool* mpPickToolRef;
        MagicCore::PickTool* mpPickToolFrom;
#if DEBUGDUMPFILE
        GPP::DumpBase* mpDumpInfo;
#endif
        GPP::PointCloud* mpPointCloudRef;
        GPP::PointCloud* mpPointCloudFrom;
        GPP::SumPointCloud* mpSumPointCloud;
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
        bool mUpdatePointCloudListRendering;
        bool mUpdateMarkListRendering;
        bool mIsDepthImageRef;
        bool mIsDepthImageFrom;
        std::vector<GPP::PointCloud*> mPointCloudList;
        std::vector<std::vector<GPP::Vector3> > mMarkList;
        double mGlobalRegistrateProgress;
        bool mEnterPointShop;
        bool mUpdateUIInfo;
        bool mReversePatchNormalRef;
        bool mReversePatchNormalFrom;
        int mMaxGlobalIterationCount;
        bool mSaveGlobalRegistrateResult;
        int mMaxSampleTripleCount;
        std::vector<std::string> mPointCloudFiles;
        std::vector<std::vector<GPP::ImageColorId> > mImageColorIdList;
        std::vector<GPP::ImageColorId> mImageColorIds;
        std::vector<std::string> mTextureImageFiles;
        std::vector<int> mCloudIds;
        std::vector<std::vector<int> > mColorList;
        std::vector<int> mColorIds;
        bool mNeedBlendColor;
    };
}
