#pragma once
#include "AppBase.h"
#include "Vector3.h"
#include <vector>

namespace GPP
{
    class TriMesh;
    class PointCloud;
    class DumpBase;
}

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class MeshShopAppUI;
    class MeshShopApp : public AppBase
    {
    public:
        MeshShopApp();
        ~MeshShopApp();

        virtual bool Enter(void);
        virtual bool Update(float timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        bool ImportMesh(void);
        void ExportMesh(void);
        void ConsolidateTopology(void);
        void ReverseDirection(void);
        void ConsolidateGeometry(void);
        void SmoothMesh(void);
        void EnhanceMeshDetail(void);
        void LoopSubdivide(void);
        void CCSubdivide(void);
        void RefineMesh(int targetVertexCount);
        void SimplifyMesh(int targetVertexCount);
        void SampleMesh(void);
        void FindHole(bool isShowHole);
        void FillHole(bool isFillFlat);

        void SetMesh(GPP::TriMesh* triMesh);
        int GetMeshVertexCount(void);
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        void InitViewTool(void);
        void UpdateMeshRendering(void);
        void SetToShowHoleLoopVrtIds(const std::vector<std::vector<GPP::Int> >& toShowHoleLoopIds);
        void SetBoundarySeedIds(const std::vector<GPP::Int>& bounarySeedIds);
        void UpdateHoleRendering(void);

    private:
        MeshShopAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        MagicCore::ViewTool* mpViewTool;
        GPP::DumpBase* mpDumpInfo;

        std::vector<std::vector<GPP::Int> > mShowHoleLoopIds;
        std::vector<GPP::Int>               mBoundarySeedIds;
    };
}
