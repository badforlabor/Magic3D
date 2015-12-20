#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class PointShopAppUI
    {
    public:
        PointShopAppUI();
        ~PointShopAppUI();

        void Setup();
        void Shutdown();

    private:
        void ImportPointCloud(MyGUI::Widget* pSender);
        void ExportPointCloud(MyGUI::Widget* pSender);
        void RemovePointCloudOutlier(MyGUI::Widget* pSender);
        void SamplePointCloud(MyGUI::Widget* pSender);
        void DoSamplePointCloud(MyGUI::Widget* pSender);
        void PointCloudNormal(MyGUI::Widget* pSender);
        void CalculatePointCloudNormal(MyGUI::Widget* pSender);
        void FlipPointCloudNormal(MyGUI::Widget* pSender);
        void SmoothPointCloudNormal(MyGUI::Widget* pSender);
        void ReconstructMesh(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);
        void Contact(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
