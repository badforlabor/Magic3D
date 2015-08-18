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

    private:
        void ImportMesh(MyGUI::Widget* pSender);
        void ExportMesh(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);
        void Contact(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
