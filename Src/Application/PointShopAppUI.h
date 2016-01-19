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

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

        void SetPointCloudInfo(int pointCount);

    private:
        void ImportPointCloud(MyGUI::Widget* pSender);
        void ExportPointCloud(MyGUI::Widget* pSender);
        
        void SamplePointCloud(MyGUI::Widget* pSender);
        void DoSamplePointCloud(MyGUI::Widget* pSender);

        void PointCloudNormal(MyGUI::Widget* pSender);
        void CalculatePointCloudNormalFront(MyGUI::Widget* pSender);
        void CalculatePointCloudNormal(MyGUI::Widget* pSender);
        void FlipPointCloudNormal(MyGUI::Widget* pSender);
        void SmoothPointCloudNormal(MyGUI::Widget* pSender);
        
        void ConsolidatePointCloud(MyGUI::Widget* pSender);
        void RemovePointCloudOutlier(MyGUI::Widget* pSender);
        void RemoveIsolatePart(MyGUI::Widget* pSender);
        void SmoothPointCloudByNormal(MyGUI::Widget* pSender);

        void ReconstructMesh(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
        MyGUI::TextBox* mTextInfo;
    };
}
