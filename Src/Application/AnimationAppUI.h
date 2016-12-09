#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class AnimationAppUI
    {
    public:
        AnimationAppUI();
        ~AnimationAppUI();

        void Setup();
        void Shutdown();

    private:
        void ImportModel(MyGUI::Widget* pSender);
        
        void InitControlPoint(MyGUI::Widget* pSender);
        void DoInitControlPoint(MyGUI::Widget* pSender);

        void SelectControlPoint(MyGUI::Widget* pSender);
        void SelectByRectangle(MyGUI::Widget* pSender);
        void ClearSelection(MyGUI::Widget* pSender);
        void MoveControlPoint(MyGUI::Widget* pSender);

        void DeformModel(MyGUI::Widget* pSender);
        void InitDeformation(MyGUI::Widget* pSender);
        void DoDeformation(MyGUI::Widget* pSender);
        void RealTimeDeform(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
    };
}
