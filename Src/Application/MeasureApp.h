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
    public:
        enum MouseMode
        {
            MM_VIEW = 0,
            MM_PICK_MESH_REF
        };

        MeasureApp();
        ~MeasureApp();

        virtual bool Enter(void);
        virtual bool Update(float timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        bool ImportModelRef(void);
        void SwitchMeshRefControlState(void);
        void DeleteMeshMarkRef(void);
        void ComputeApproximateGeodesics(void);

        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
        void SwitchToViewMode(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        void InitViewTool(void);
        void InitPickTool(void);
        void UpdateModelRendering(void);
        void UpdateMarkRendering(void);

    private:
        MeasureAppUI* mpUI;
        MouseMode mMouseMode;
        GPP::TriMesh* mpTriMeshRef;
        GPP::PointCloud* mpPointCloudRef;
        GPP::TriMesh* mpTriMeshFrom;
        GPP::PointCloud* mpPointCloudFrom;
        MagicCore::ViewTool* mpViewTool;
        MagicCore::PickTool* mpPickTool;
        GPP::DumpBase* mpDumpInfo;
        std::vector<GPP::Int> mMeshRefMarkIds;
        std::vector<GPP::Vector3> mMarkPoints;
    };
}
