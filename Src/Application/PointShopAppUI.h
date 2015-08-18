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
        void SmoothPointCloud(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);
        void Contact(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
