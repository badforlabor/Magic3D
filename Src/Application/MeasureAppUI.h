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

    private:
        void ImportModelRef(MyGUI::Widget* pSender);
        void Geodesics(MyGUI::Widget* pSender);
        void SelectMeshMarkRef(MyGUI::Widget* pSender);
        void DeleteMeshMarkRef(MyGUI::Widget* pSender);
        void ComputeApproximateGeodesics(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);
        void Contact(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
