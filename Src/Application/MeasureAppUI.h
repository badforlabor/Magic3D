#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class MeasureAppUI
    {
    public:
        MeasureAppUI();
        ~MeasureAppUI();

        void Setup();
        void Shutdown();

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

        void SetRefModelInfo(int vertexCount, int triangleCount);
        void SetFromModelInfo(int vertexCount, int triangleCount);
        void SetGeodesicsInfo(double distance);
        void SetDeviationInfo(double maxDistance);

    private:
        void SwitchDisplayMode(MyGUI::Widget* pSender);

        void ImportModelRef(MyGUI::Widget* pSender);
        void Geodesics(MyGUI::Widget* pSender);
        void DeleteMeshMarkRef(MyGUI::Widget* pSender);
        void ComputeApproximateGeodesics(MyGUI::Widget* pSender);
        void ComputeExactGeodesics(MyGUI::Widget* pSender);

        void ImportModelFrom(MyGUI::Widget* pSender);
        void Deviation(MyGUI::Widget* pSender);
        void ComputeDeviation(MyGUI::Widget* pSender);

        void BackToHomepage(MyGUI::Widget* pSender);

        void UpdateTextInfo(void);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
        MyGUI::TextBox* mTextInfo;
        // Application Information
        int mRefVertexCount;
        int mRefTriangleCount;
        int mFromVertexCount;
        int mFromTriangleCount;
        double mGeodesicsDistance;
        double mMaxDeviationDistance;
    };
}
