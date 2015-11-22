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
        void SelectRef(MyGUI::Widget* pSender);
        void PushRef(MyGUI::Widget* pSender);
        void PopRef(MyGUI::Widget* pSender);
        void ViewRef(MyGUI::Widget* pSender);
        //void FuseRef(MyGUI::Widget* pSender);

        void ImportPointCloudFrom(MyGUI::Widget* pSender);
        void FromNormal(MyGUI::Widget* pSender);
        void CalculateFromNormal(MyGUI::Widget* pSender);
        void FlipFromNormal(MyGUI::Widget* pSender);
        void SelectFrom(MyGUI::Widget* pSender);
        void PushFrom(MyGUI::Widget* pSender);
        void PopFrom(MyGUI::Widget* pSender);
        void ViewFrom(MyGUI::Widget* pSender);
        void AlignFrom(MyGUI::Widget* pSender);

        void BackToHomepage(MyGUI::Widget* pSender);
        void Contact(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
