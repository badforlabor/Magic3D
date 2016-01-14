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

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

    private:
        void SwitchDisplayMode(MyGUI::Widget* pSender);

        void ImportPointCloudRef(MyGUI::Widget* pSender);
        
        void RefNormal(MyGUI::Widget* pSender);
        void CalculateRefNormalFront(MyGUI::Widget* pSender);
        void CalculateRefNormal(MyGUI::Widget* pSender);
        void FlipRefNormal(MyGUI::Widget* pSender);
        //void SmoothRefNormal(MyGUI::Widget* pSender);
        
        void RefFeaturePoint(MyGUI::Widget* pSender);
        void RefDeleteMark(MyGUI::Widget* pSender);
        void RefImportMark(MyGUI::Widget* pSender);

        void ImportPointCloudFrom(MyGUI::Widget* pSender);
        
        void FromNormal(MyGUI::Widget* pSender);
        void CalculateFromNormalFront(MyGUI::Widget* pSender);
        void CalculateFromNormal(MyGUI::Widget* pSender);
        void FlipFromNormal(MyGUI::Widget* pSender);
        //void SmoothFromNormal(MyGUI::Widget* pSender);
        
        void FromFeaturePoint(MyGUI::Widget* pSender);
        void FromDeleteMark(MyGUI::Widget* pSender);
        void FromImportMark(MyGUI::Widget* pSender);

        void AlignFrom(MyGUI::Widget* pSender);
        void AlignMark(MyGUI::Widget* pSender);
        void AlignFree(MyGUI::Widget* pSender);
        void AlignICP(MyGUI::Widget* pSender);
        
        void FuseRef(MyGUI::Widget* pSender);
        
        void GlobalRegistrate(MyGUI::Widget* pSender);

        void EnterPointShop(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
    };
}
