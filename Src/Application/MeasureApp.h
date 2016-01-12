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
            GEODESICS_EXACT
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

        void DoCommand(bool isSubThread);

        bool ImportModelRef(void);
        void DeleteMeshMarkRef(void);
        void ComputeApproximateGeodesics(bool isSubThread = true);
        void ComputeExactGeodesics(bool isSubThread = true);

        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
        bool IsCommandInProgress(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        bool IsCommandAvaliable(void);

    private:
        void InitViewTool(void);
        void UpdateModelRendering(void);
        void UpdateMarkRendering(void);

    private:
        MeasureAppUI* mpUI;
        GPP::TriMesh* mpTriMeshRef;
        GPP::PointCloud* mpPointCloudRef;
        GPP::TriMesh* mpTriMeshFrom;
        GPP::PointCloud* mpPointCloudFrom;
        MagicCore::ViewTool* mpViewTool;
        MagicCore::PickTool* mpPickTool;
        GPP::DumpBase* mpDumpInfo;
        std::vector<GPP::Int> mMeshRefMarkIds;
        std::vector<GPP::Vector3> mMarkPoints;
        CommandType mCommandType;
        bool mIsCommandInProgress;
        bool mUpdateModelRendering;
        bool mUpdateMarkRendering;
    };
}
