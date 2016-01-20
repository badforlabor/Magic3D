#pragma once
#include "AppBase.h"
#include "../Common/RenderSystem.h"
#include "Vector3.h"
#include <vector>
#include "Gpp.h"

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class MeshShopAppUI;

    class MeshShopApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0,
            EXPORTMESH,
            CONSOLIDATETOPOLOGY,
            CONSOLIDATEGEOMETRY,
            REMOVEISOLATEPART,
            REMOVEMESHNOISE,
            SMOOTHMESH,
            ENHANCEDETAIL,
            LOOPSUBDIVIDE,
            REFINE,
            SIMPLIFY,
            FILLHOLE
        };

    public:
        MeshShopApp();
        ~MeshShopApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        void DoCommand(bool isSubThread);

        void SwitchDisplayMode(void);
        bool ImportMesh(void);
        void ExportMesh(bool isSubThread = true);
        void ConsolidateTopology(bool isSubThread = true);
        void ReverseDirection(void);
        void RemoveMeshIsolatePart(bool isSubThread = true);
        void ConsolidateGeometry(bool isSubThread = true);
        void RemoveMeshNoise(bool isSubThread = true);
        void SmoothMesh(bool isSubThread = true);
        void EnhanceMeshDetail(bool isSubThread = true);
        void LoopSubdivide(bool isSubThread = true);
        void RefineMesh(int targetVertexCount, bool isSubThread = true);
        void SimplifyMesh(int targetVertexCount, bool isSubThread = true);
        void SampleMesh(void);
        void FindHole(bool isShowHole);
        void FillHole(bool isFillFlat, bool isSubThread = true);

        void SetMesh(GPP::TriMesh* triMesh, GPP::Vector3 objCenterCoord, GPP::Real scaleValue);
        int GetMeshVertexCount(void);
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
        void UpdateMeshRendering(void);
        void SetToShowHoleLoopVrtIds(const std::vector<std::vector<GPP::Int> >& toShowHoleLoopIds);
        void SetBoundarySeedIds(const std::vector<GPP::Int>& bounarySeedIds);
        void UpdateHoleRendering(void);

    private:
        MeshShopAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        MagicCore::ViewTool* mpViewTool;
        int mDisplayMode;
        GPP::DumpBase* mpDumpInfo;
        std::vector<std::vector<GPP::Int> > mShowHoleLoopIds;
        std::vector<GPP::Int>               mBoundarySeedIds;
        GPP::Int mTargetVertexCount;
        bool mIsFillFlat;
        CommandType mCommandType;
        bool mUpdateMeshRendering;
        bool mUpdateHoleRendering;
        bool mIsCommandInProgress;
    };
}
