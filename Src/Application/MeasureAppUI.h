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

    private:
        void ImportModelRef(MyGUI::Widget* pSender);
        void Geodesics(MyGUI::Widget* pSender);
        void SelectMeshMarkRef(MyGUI::Widget* pSender);
        void DeleteMeshMarkRef(MyGUI::Widget* pSender);
        void ComputeApproximateGeodesics(MyGUI::Widget* pSender);
        void ComputeExactGeodesics(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
    };
}
