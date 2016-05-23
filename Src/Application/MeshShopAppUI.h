#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class MeshShopAppUI
    {
    public:
        MeshShopAppUI();
        ~MeshShopAppUI();

        void Setup();
        void Shutdown();

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

        void SetMeshInfo(int vertexCount, int triangleCount);

    private:
        void SwitchDisplayMode(MyGUI::Widget* pSender);
        void ImportMesh(MyGUI::Widget* pSender);
        void ExportMesh(MyGUI::Widget* pSender);
        void ConsolidateMesh(MyGUI::Widget* pSender);
        void ConsolidateTopology(MyGUI::Widget* pSender);
        void ConsolidateGeometry(MyGUI::Widget* pSender);
        void ReverseDirection(MyGUI::Widget* pSender);
        void RemoveMeshIsolatePart(MyGUI::Widget* pSender);
        void FilterMesh(MyGUI::Widget* pSender);
        void RemoveMeshNoise(MyGUI::Widget* pSender);
        void SmoothMesh(MyGUI::Widget* pSender);
        void EnhanceMeshDetail(MyGUI::Widget* pSender);
        void SubdivideMesh(MyGUI::Widget* pSender);
        void RefineMesh(MyGUI::Widget* pSender);
        void DoRefineMesh(MyGUI::Widget* pSender);
        void SimplifyMesh(MyGUI::Widget* pSender);
        void DoSimplifyMesh(MyGUI::Widget* pSender);
        void FillHole(MyGUI::Widget* pSender);
        void DoFillHoleFlat(MyGUI::Widget* pSender);
        void DoFillHoleTangent(MyGUI::Widget* pSender);
        void DoFillHoleSmooth(MyGUI::Widget* pSender);
        void AppJump(MyGUI::Widget* pSender);
        void SampleMesh(MyGUI::Widget* pSender);
        void EnterReliefApp(MyGUI::Widget* pSender);
        void EnterTextureApp(MyGUI::Widget* pSender);
        void EnterMeasureApp(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
        MyGUI::TextBox* mTextInfo;
    };
}
