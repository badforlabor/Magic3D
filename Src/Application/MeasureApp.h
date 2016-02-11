#pragma once
#include "AppBase.h"
#include <vector>
#include "GPP.h"

namespace GPP
{
    class TriMesh;
    class PointCloud;
    class DumpBase;
}

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class MeasureAppUI;
    class MeasureApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0,
            GEODESICS_APPROXIMATE,
            GEODESICS_EXACT,
            DEVIATION
        };

    public:
        MeasureApp();
        ~MeasureApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        void DoCommand(bool isSubThread);

        bool ImportModelRef(void);

        void DeleteMeshMarkRef(void);
        void ComputeApproximateGeodesics(bool isSubThread = true);
        void ComputeExactGeodesics(bool isSubThread = true);

        void MeasureRefArea(void);
        void MeasureRefVolume(void);

        bool ImportModelFrom(void);
        void ComputeDeviation(bool isSubThread = true);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif
        bool IsCommandInProgress(void);

        void SwitchSeparateDisplay(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        void ClearModelFromData(void);
        bool IsCommandAvaliable(void);

        void InitViewTool(void);
        void UpdateModelRefRendering(void);
        void UpdateMarkRefRendering(void);
        void UpdateModelFromRendering(void);
        void UpdateMarkFromRendering(void);

        void SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color);
        void SetMeshColor(GPP::TriMesh* triMesh, const GPP::Vector3& color);

    private:
        MeasureAppUI* mpUI;
        GPP::TriMesh* mpTriMeshRef;
        GPP::PointCloud* mpPointCloudRef;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        GPP::TriMesh* mpTriMeshFrom;
        GPP::PointCloud* mpPointCloudFrom;
        MagicCore::ViewTool* mpViewTool;
        bool mIsSeparateDisplay;
        MagicCore::PickTool* mpPickToolRef;
        MagicCore::PickTool* mpPickToolFrom;
#if DEBUGDUMPFILE
        GPP::DumpBase* mpDumpInfo;
#endif
        std::vector<GPP::Int> mRefMarkIds;
        std::vector<GPP::Vector3> mRefMarkPoints;
        std::vector<GPP::Int> mFromMarkIds;
        std::vector<GPP::Vector3> mFromMarkPoints;
        CommandType mCommandType;
        bool mIsCommandInProgress;
        bool mUpdateModelRefRendering;
        bool mUpdateMarkRefRendering;
        bool mUpdateModelFromRendering;
        bool mUpdateMarkFromRendering;
    };
}
