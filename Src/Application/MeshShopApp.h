#pragma once
#include "AppBase.h"

namespace GPP
{
    class TriMesh;
    class PointCloud;
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
        void LoopSubdivide(void);
        void CCSubdivide(void);
        void RefineMesh(int targetVertexCount);
        void SimplifyMesh(int targetVertexCount);
        void ParameterizeMesh(void);
        void SampleMesh(void);

        void SetMesh(GPP::TriMesh* triMesh);
        int GetMeshVertexCount(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        void InitViewTool(void);
        void UpdateMeshRendering(void);

    private:
        MeshShopAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        MagicCore::ViewTool* mpViewTool;
    };
}
