#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class RegistrationAppUI
    {
    public:
        RegistrationAppUI();
        ~RegistrationAppUI();

        void Setup();
        void Shutdown();

    private:
        void ImportPointCloudRef(MyGUI::Widget* pSender);
        void RefNormal(MyGUI::Widget* pSender);
        void CalculateRefNormal(MyGUI::Widget* pSender);
        void FlipRefNormal(MyGUI::Widget* pSender);
        void FuseRef(MyGUI::Widget* pSender);

        void ImportPointCloudFrom(MyGUI::Widget* pSender);
        void FromNormal(MyGUI::Widget* pSender);
        void CalculateFromNormal(MyGUI::Widget* pSender);
        void FlipFromNormal(MyGUI::Widget* pSender);
        void AlignFrom(MyGUI::Widget* pSender);
        void AlignFast(MyGUI::Widget* pSender);
        void AlignPrecise(MyGUI::Widget* pSender);
        void AlignICP(MyGUI::Widget* pSender);

        void EnterPointShop(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
